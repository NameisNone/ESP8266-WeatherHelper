#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "EspUart.h"
#include "usart.h"
#include "wifi.h"
//ALIENTEK Mini STM32�����巶������2
//��������ʵ��		   
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾


const u8* SSID="CMCC-Y3ff";
const u8* PSWD="9uas4tfq";
const u8* SERVER="api.seniverse.com";
const u8* PORT="80";

 int main(void)
 {	
	u8 key;	  
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ9600	
	USART2_Init(115200);  //��ʼ������2������Ϊ11520
	LED_Init();		  	 	//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();          	//��ʼ���밴�����ӵ�Ӳ���ӿ�
	while(!WiFi_module_check())//���wifiģ���Ƿ����
	{
		printf("δ��⵽wifiģ��!!!\r\n");
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
				if(!Connect_TCP_Server(ESP8266_STA_Mode,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT))//���ӷ�����
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
