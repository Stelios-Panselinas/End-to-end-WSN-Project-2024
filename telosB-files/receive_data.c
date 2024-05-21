#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "utils.h"
#include <string.h>
#include <stdio.h> /* For printf() */
//#include <time.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

void double2str(char *buf, double num);

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);





/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
    static struct data_form received_data;
    //time_t rawtime;
    //struct tm *timeinfo;
    

    memcpy(&received_data, data, sizeof(struct data_form));
    if(len == sizeof(struct data_form)) {
    char temp[30];
    char hum[30];
    
    double2str(temp, received_data.temp);
    double2str(hum, received_data.hum);
    
    /*time(&rawtime);
    timeinfo = localtime(&rawtime);
    char datetime[80];
    strftime(datetime, sizeof(datetime), "%d-%m-%Y, %H:%M:%S", timeinfo);*/
    
    LOG_INFO("[%d,%d,%s,%s]", received_data.ID, received_data.count, temp, hum);
    LOG_INFO_("\n");
  }
}





/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  //static struct etimer periodic_timer;
  //static unsigned count = 0;

  PROCESS_BEGIN();
  
  /* Initialize NullNet */
  //nullnet_buf = (uint8_t *)&count;
  //nullnet_len = sizeof(count);
  LOG_INFO("ID,Count,Temperature,Humidity,Datetime");
  nullnet_set_input_callback(input_callback);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
