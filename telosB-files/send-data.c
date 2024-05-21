#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "utils.h"
#include "arch/dev/sensor/sht11/sht11-sensor.h"

#include <string.h>
#include <stdio.h> /* For printf() */

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (20 * CLOCK_SECOND)
static linkaddr_t dest_addr =         {{ 0xae, 0x52, 0x25, 0x0d, 0x00, 0x74, 0x12, 0x00 }};  

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    static struct data_form received_data;
    memcpy(&received_data, data, sizeof(received_data));
    //LOG_INFO("Received temperature:%u and humididty:%u from mote:%d with MAC adderess", received_data.temp,received_data.hum,received_data.ID);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;
  static struct data_form data_to_send;
  data_to_send.ID = 2;
  //data_to_send.count = 0;	

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(sht11_sensor);

#if MAC_CONF_WITH_TSCHdest
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&data_to_send;
  nullnet_len = sizeof(data_to_send);
  nullnet_set_input_callback(input_callback);

  if(!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) {
    etimer_set(&periodic_timer, SEND_INTERVAL);
    //unsigned temp;
    //unsigned hum;
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      data_to_send.temp = temperature_int2double(sht11_sensor.value(SHT11_SENSOR_TEMP));
      data_to_send.hum = humidity_int2double(sht11_sensor.value(SHT11_SENSOR_HUMIDITY));
      data_to_send.count = count;
      //LOG_INFO("temp:%f hum:%f\n", temp,hum);
      //LOG_INFO("Sending temperature:%s and humidity:%s from mote:%d to ", data_to_send.temp, data_to_send.hum, data_to_send.ID);
      LOG_INFO_LLADDR(&dest_addr);
      LOG_INFO_("\n");

      NETSTACK_NETWORK.output(&dest_addr);
      count++;
      etimer_reset(&periodic_timer);
    }
  }
  SENSORS_DEACTIVATE(sht11_sensor);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
