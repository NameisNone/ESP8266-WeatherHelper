#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "EspUart.h"
#include "usart.h"
#include "wifi.h"
#include "lcd.h"
#include "malloc.h"
#include "Hz.h"
//ALIENTEK Mini STM32�����巶������2
//��������ʵ��		   
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾


const u8* SSID="CMCC-Y3ff";
const u8* PSWD="9uas4tfq";
const u8* SERVER="api.seniverse.com";
const u8* PORT="80";
const u8* Request=
"GET https://api.seniverse.com/v3/weather/now.json?key=Si4evEh1Az2pvdyB5&location=guilin&language=en&unit=c\r\n";


 int main(void)
 {	
	u8 *response = NULL;
	u8 *ipAddr = NULL;
	WeatherData *userData;
	u8 key;	  
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ9600	
	USART2_Init(115200);  //��ʼ������2������Ϊ11520
	LED_Init();		  	 	//��ʼ����LED���ӵ�Ӳ���ӿ�
	LCD_Init();						//��ʼ��Һ��
	KEY_Init();          	//��ʼ���밴�����ӵ�Ӳ���ӿ�
	mem_init();					//��ʼ���ڴ��
/*	while(!WiFi_module_check())//���wifiģ���Ƿ����
	{
		printf("δ��⵽wifiģ��!!!\r\n");
		LED0 = 0;
	}
*/
	LCD_Clear(WHITE);
	while(1)//ѭ��
	{
		//show_Hz32(60,100,"�人����",BLACK,WHITE);
		Show_Hz16(60,200,"�人����",BLACK,WHITE);
		LCD_ShowString(20,90,150,40,16,"China Come on!");
	}
		
	#if 0 
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
			ipAddr = mymalloc(50);
				if(!Connect_TCP_Server(ipAddr,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT,1))//���ӷ���������͸��ģʽ
				{
					LED1 = !LED1;
					printf("IP��ַ��%s\r\n",ipAddr);
					myfree(ipAddr);
				}
				else
				{
					LED0 = 0;
				} 
		}
		else if(key == WKUP_PRES)//wkup
		{
			printf("����wkup!\r\n");
			response = mymalloc(500);//�����ڴ��responseָ��
			userData = mymalloc(500);
			get_http((u8*)Request,response);//����get����
			cJSON_Parse_Weather((char*)response,userData);
			printf("****************************************��������:\r\n");
			printf("UserData->city:%s,\r\n",userData->city);
			printf("UserData->text:%s,\r\n",userData->text);
			printf("UserData->code:%d,\r\n",userData->code);
			printf("UserData->temp:%d,\r\n",userData->temp);
			myfree(response);//�ͷ��ڴ�
			myfree(userData);//�ͷ��ڴ�
			LED0 = !LED0;
			LED1 = !LED1;
		}
	}	
	#endif
		
}
