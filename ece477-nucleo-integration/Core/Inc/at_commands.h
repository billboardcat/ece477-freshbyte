#ifndef ECE477_ESP8266_AT_COMMANDS_H
#define ECE477_ESP8266_AT_COMMANDS_H

#define AT_SUCCESS 0
#define AT_FAIL 1

int setup_wifi(char * ssid, char * password);
int sent_freshbyte_data(int temp_F, int humid, int methane);
unsigned char * receive_prediction();
unsigned char *  extract_prediction();

#endif //ECE477_ESP8266_AT_COMMANDS_H
