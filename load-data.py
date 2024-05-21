import pymongo
import re
import os
import time
import select
import threading
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Connection with db and check if db exists
myclient = pymongo.MongoClient("mongodb://localhost:27017/")


dblist = myclient.list_database_names()
if "wsnDB" in dblist:
    print("The Database exists. Start loading data from file out_pipe...")
    db = myclient["wsnDB"]
    collist = db.list_collection_names()
    if "sensor_data" in collist:
        col = db["sensor_data"]
    else:
        col = db.create_collection("sensor_data")
else:
    print("Creating Database...")
    db = myclient["wsnDB"]
    col = db.create_collection("sensor_data")
    print("Database created. Start loading data from file out_pipe...")

# Lists to store data for plotting
timestamps = []
temperatures = []
humidities = []


data_lock = threading.Lock()

# proccess each line in out_pipe file to get the data and store them in db
def process_line(line):
    # Extract timestamp out of the line
    timestamp_match = re.search(r"\[(.*?)\]", line)
    if timestamp_match:
        timestamp = timestamp_match.group(1)
        # Find the next set of brackets which is like [INFO: App       ] in order to find the third pair of brackets that has our data
        info_match = re.search(r"\[(.*?)\]", line[timestamp_match.end():])
        if info_match:
            # Find the third set of brackets after the second
            data_match = re.search(r"\[(.*?)\]", line[timestamp_match.end() + info_match.end():])
            if data_match:
                data_inside_third_bracket = data_match.group(1).split(",")
                if len(data_inside_third_bracket) >= 4:
                    id_value = data_inside_third_bracket[0]
                    count_value = data_inside_third_bracket[1]
                    temperature_value = float(data_inside_third_bracket[2])
                    humidity_value = float(data_inside_third_bracket[3])
                    data = {
                        "Timestamp": timestamp,
                        "ID": id_value,
                        "Count": count_value,
                        "Temperature": temperature_value,
                        "Humidity": humidity_value,
                    }
                    col.insert_one(data)
                    print(f"New line inserted in database with values Timestamp: {timestamp} ID: {id_value} Count: {count_value} Temperature: {temperature_value} Humidity: {humidity_value}")

                    # Append data to the corresponding lists
                    with data_lock:
                        timestamps.append(timestamp)
                        temperatures.append(temperature_value)
                        humidities.append(humidity_value)

# check for new lines in the file in real-time and proccess the data
def follow_file(file_path):
    with open(file_path, 'r') as f:
        f.seek(0, os.SEEK_END)

        while True:
            ready, _, _ = select.select([f], [], [], 1.0)

            if ready:
                line = f.readline()
                while line:
                    process_line(line.strip())
                    line = f.readline()
            else:
                time.sleep(0.1)

# update the plot
def update_plot(frame):
    with data_lock:
        plt.cla()  # Clear the plot
        plt.plot(timestamps, temperatures, label='Temperature (C)')
        plt.plot(timestamps, humidities, label='Humidity (%)')
        plt.xlabel('Timestamp')
        plt.ylabel('Value')
        plt.legend()
        plt.tight_layout()


if __name__ == "__main__":
    # start the file-following thread
    threading.Thread(target=follow_file, args=('/home/stelios/contiki-ng/examples/my_nullnet/out_pipe',), daemon=True).start()

    # set up the plot
    fig = plt.figure()
    ani = FuncAnimation(fig, update_plot, interval=1000)

    # show the plot
    plt.show()
