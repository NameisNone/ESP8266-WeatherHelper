#ifndef  __WIFI_H
#define  __WIFI_H
#include "sys.h"

#define DEBUG_EN 
#define OK	(u8*)"OK"

#define TCP (u8*)"TCP"
#define UDP (u8*)"UDP"

#define ESP8266_STA_Mode  (u8*)"AT+CWMODE=1"//STAģʽ
#define ESP8266_AP_Mode   (u8*)"AT+CWMODE=2"//APģʽ
#define ESP8266_STAP_Mode (u8*)"AT+CWMODE=3"//STA+APģʽ


u8 wifi_send_ATcmd(u8 *cmd,u8 *ack,u16 waittime);
u8* wifi_check_cmd(u8 *str);
u8 WiFi_module_check(void);
u8 Connect_TCP_Server(u8 *ESP8266_Mode, u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port);

#endif





