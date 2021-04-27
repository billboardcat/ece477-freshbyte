#include "serial_print.h"
#include "at_commands.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int session_id = 1;

extern unsigned char UART1_rxBuffer[600];
extern unsigned char prediction_str[2];

const char wifi_send_data_str [] = "AT+HTTPCLIENT=3,0,\"http://maker.ifttt.com/trigger/ece477/"
                                   "with/key/cRY9n1jJnl-fCLuPYsZZ-8\",\"maker.ifttt.com\",\""
                                   "/trigger/ece477/with/key/cRY9n1jJnl-fCLuPYsZZ-8\",1,\""
                                   "value1=%d&value2=%d&value3=%d%%7C%%7C%%7C%d\"\n";

const char wifi_receive_data_str [] = "AT+HTTPCLIENT=2,0,\"http://gsx2json.com/api?id=1wP-fJqQEgVEwBICx2EHh9-tfq526fHGmL53x4p5_quc&sheet=1&q=Prediction(Days)\",\"gsx2json.com\",\"/get\",1";


//Todo check if busy before sending

//TODO - implement OK check + Error Retry

// This function sets the wifi mode to station, sets the AP to connect to,
int setup_wifi(char * ssid, char * password){

  //TODO - define error

//  serial_select(WIFI);
  serial_select(WIFI);
  serial_println("AT+CWMODE=1");
  HAL_Delay(50); //dealy of 50 ms
  serial_printf("AT+CWJAP=\"%s\",\"%s\"\n", ssid, password);
  HAL_Delay(2000);
  //TODO wait until OK

  return AT_SUCCESS;
}

int sent_freshbyte_data(int temp_F, int humid, int methane){

  //TODO - define error

  //TODO - define all these as consts
  serial_select(WIFI);
  serial_printf(wifi_send_data_str, temp_F, humid, methane, session_id);
  HAL_Delay(2000);
  session_id++;

  return AT_SUCCESS;
}

unsigned char * receive_prediction(){
  //TODO - make this for the real dataset!

  //TODO - define all these as consts

  serial_select(WIFI);
  //TODO test this new one!
  serial_println(wifi_receive_data_str);
  HAL_Delay(5000);

  return extract_prediction();
}

unsigned char * extract_prediction(){

  unsigned char * prediction = prediction_str;
  unsigned char * str = UART1_rxBuffer;
  unsigned char sub[] = "cyevm\":[";

  unsigned char *p1, *p2, *p3;
  int i=0,j=0,flag=0;

  p1 = str;
  p2 = sub;

  for(i = 0; i<strlen(str); i++)
  {
    if(*p1 == *p2)
    {
      p3 = p1;
      for(j = 0;j<strlen(sub);j++)
      {
        if(*p3 == *p2)
        {
          p3++;p2++;
        }
        else
          break;
      }
      p2 = sub;
      if(j == strlen(sub))
      {
        flag = 1;
        // printf("\nSubstring found at index : %d\n",i);
        break;
      }
    }
    p1++;
  }
  if(flag==0)
  {
      //printf("Substring NOT found");
    return 0;
  }

  p1 += sizeof(sub) - 1; //add length of substring to now point to start of number
  i += sizeof(sub) - 1;
//  prediction = strtof(p1);

  //hack for now ...
  //TODO fix this
  p2 = p1;
  while (*p2 != ']'){
    p2++;
  }
  int prediction_str_len = p2 - p1;
  strncpy(prediction, p1, prediction_str_len);

  return prediction;
}
