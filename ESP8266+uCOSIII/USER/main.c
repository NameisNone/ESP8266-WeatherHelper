#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "EspUart.h"
#include "usart.h"
#include "wifi.h"
#include "lcd.h"
//ALIENTEK Mini STM32�����巶������2
//��������ʵ��		   
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾


const u8* SSID="CMCC-Y3ff";
const u8* PSWD="9uas4tfq";
const u8* SERVER="api.seniverse.com";
const u8* PORT="80";
const u8* Request="GET https://api.seniverse.com/v3/weather/now.json?key=Si4evEh1Az2pvdyB5&location=guilin&language=en&unit=c\r\n";
 
 
 int main(void)
 {	
	u8 *response = NULL;
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
	
	while(1)//ѭ��
	{
		key = KEY_Scan(0);
		if(key == KEY0_PRES)//key0
		{
			printf("����key0!\r\n");
			if(!wifi_send_ATcmd("AT","OK",15))
			{
				LED1 = !LED1;
			}
		}
		else if(key == KEY1_PRES)//key1
		{
			printf("����key1!\r\n");
				if(!Connect_TCP_Server(ESP8266_STA_Mode,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT))//���ӷ�����
				{
					LED1 = !LED1;
				}
				else
				{
					LED0 = 0;
				} 
		}
		else if(key == WKUP_PRES)//wkup
		{
			printf("����wkup!\r\n");
			get_http((u8*)Request);//����get����
			LED0 = !LED0;
			LED1 = !LED1;
		}
	}	
		
}
