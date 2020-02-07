#ifndef  __WIFI_H
#define  __WIFI_H
#include "sys.h"
#include "Weather.h"

typedef struct
{
	unsigned int confirm;//确诊人数
	unsigned int suspect;//疑似人数
	unsigned int dead;//死亡人数
	unsigned int heal;//治愈人数
	unsigned char date[15];
}FY_StructDef;

//注释掉DEBUG_EN可以关闭wifi.c中输出的串口调试信息
#define DEBUG_EN 

//宏定义
#define OK	(u8*)"OK"

#define TCP (u8*)"TCP"
#define UDP (u8*)"UDP"

#define ESP8266_STA_Mode  (u8*)"AT+CWMODE=1"//STA
#define ESP8266_AP_Mode   (u8*)"AT+CWMODE=2"//AP
#define ESP8266_STAP_Mode (u8*)"AT+CWMODE=3"//STA+AP


u8 wifi_send_ATcmd(u8 *cmd,u8 *ack,u16 waittime);
u8* wifi_check_cmd(u8 *str);
u8 WiFi_module_check(void);
u8 Connect_TCP_Server(u8 *ipAddr,u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port,u8 CIPMODE1_EN);
u8 Exit_TransMode(void);
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime);
void get_IP_Addr(u8* ipbuf);
u8 Connect_AP(u8 *IP_Addr,u8 *WiFi_SSID, u8 *WiFi_PSWD);
void cJSON_Parse_Weather(char *jsonBuf,WeatherData *userData);
void get_http(u8* request, u8 *response);
void show_weather(u8 x,u8 y,u16 Wcolor,u16 backColor,u8 Code);
void Exit_ServerConnect(void);
void Get_myApi_Http(u8* request, FY_StructDef *Data);
u8 Connect_NewServer(u8* Server);
#endif





