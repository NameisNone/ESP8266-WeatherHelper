#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "EspUart.h"
#include "usart.h"
#include "wifi.h"
//ALIENTEK Mini STM32开发板范例代码2
//按键输入实验		   
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司


const u8* SSID="CMCC-Y3ff";
const u8* PSWD="9uas4tfq";
const u8* SERVER="api.seniverse.com";
const u8* PORT="80";

 int main(void)
 {	
	u8 key;	  
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);	 	//串口初始化为9600	
	USART2_Init(115200);  //初始化串口2波特率为11520
	LED_Init();		  	 	//初始化与LED连接的硬件接口
	KEY_Init();          	//初始化与按键连接的硬件接口
	while(!WiFi_module_check())//检查wifi模块是否存在
	{
		printf("未检测到wifi模块!!!\r\n");
		LED0 = 0;
	}
	while(1)
	{
		key = KEY_Scan(0);
		if(key == KEY0_PRES)
		{
			if(!wifi_send_ATcmd("AT","OK",15))
			{
				LED1 = !LED1;
			}
		}
		else if(key == KEY1_PRES)
		{
			//Connect_TCP_Server(ESP8266_STA_Mode,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT);
				if(!Connect_TCP_Server(ESP8266_STA_Mode,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT))//连接服务器
				{
					LED0 = !LED0;
				}
				else
				{
					LED0 = 0;
				} 
		}			
	}		 
}
