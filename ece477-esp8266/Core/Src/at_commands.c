#include "serial_print.h"
#include "at_commands.h"
#include <stdarg.h>

int session_id = 1;

// This function sets the wifi mode to station, sets the AP to connect to,
int setup_wifi(char * ssid, char * password){

  //TODO - define error

//  serial_select(WIFI);
  serial_println("AT+CWMODE=1");
  HAL_Delay(50); //dealy of 50 ms
  serial_printf("AT+CWJAP=\"%s\",\"%s\"\n", ssid, password);
  HAL_Delay(2000);
  //TODO wait until OK

  return AT_SUCCESS;
}

int sent_freshbyte_data(int temp_F, int humid, int methane){

  //TODO - define error

  serial_printf("AT+HTTPCLIENT=3,0,\"http://maker.ifttt.com/trigger/ece477/"
                "with/key/cRY9n1jJnl-fCLuPYsZZ-8\",\"maker.ifttt.com\",\""
                "/trigger/ece477/with/key/cRY9n1jJnl-fCLuPYsZZ-8\",1,\""
                "value1=%d%%7C%%7C%%7C%dF&value2=%d%%25&value3=%d\"\n", session_id, temp_F, humid, methane);
  HAL_Delay(50);
  session_id++;

  return AT_SUCCESS;
}