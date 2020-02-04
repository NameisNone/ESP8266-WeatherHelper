#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "EspUart.h"
#include "usart.h"
#include "wifi.h"
#include "lcd.h"
#include "malloc.h"

#include "timer.h"
//ALIENTEK Mini STM32�����巶������2
//��������ʵ��		   
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾

const u8* GL="Guilin";
const u8* GG="Guigang";
const u8* GZ="Guangzhou";

const u8* SSID="CMCC-Y3ff";
const u8* PSWD="9uas4tfq";
const u8* SERVER="api.seniverse.com";
const u8* PORT="80";

const u8* Request=
"GET https://api.seniverse.com/v3/weather/now.json?key=Si4evEh1Az2pvdyB5&location=guigang&language=en&unit=c\r\n";

extern u8 Parse_Flag;
int main(void)
{	
	u8 *response = NULL;
	u8 *ipAddr = NULL;
	WeatherData userData;
	u8 key=0;	  
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ9600	
	USART2_Init(115200);  //��ʼ������2������Ϊ11520
	LED_Init();		  	 	//��ʼ����LED���ӵ�Ӳ���ӿ�
	LCD_Init();						//��ʼ��Һ��
	LCD_Display_Dir(1);//���ú���
	KEY_Init();          	//��ʼ���밴�����ӵ�Ӳ���ӿ�
	TIM3_Int_Init(49999,7199);//��ʼ����ʱ��3��ÿ5s�ж�һ��
	TIM_Cmd(TIM3, DISABLE);  //ʹ��TIMx����	
	mem_init();					//��ʼ���ڴ��
	while(!WiFi_module_check())//���wifiģ���Ƿ����
	{
		POINT_COLOR=RED; 
		LCD_ShowString(0,120,200,30,16,"No WiFI Module!!");
		printf("δ��⵽wifiģ��!!!\r\n");
		LED0 = 0;
	}
	LCD_Clear(WHITE);
	POINT_COLOR=BLACK; 
	LCD_ShowString(50,0,216,20,24,"WiFi WeatherHelper");
	POINT_COLOR=DARKBLUE; 
	LCD_DrawLine(0,30,320,30);
	LCD_DrawLine(145,30,145,240);
	while(1)//ѭ��
	{	
		key = KEY_Scan(0);//����ɨ��
		if(key == KEY0_PRES)//key0
		{
			key=0;
			printf("����key0!\r\n");
			if(!wifi_send_ATcmd("AT","OK",15))
			{
				LED1 = !LED1;
			}
		}
		else if(key == KEY1_PRES)//key1
		{
			key=0;
			printf("����key1!\r\n");
			ipAddr = mymalloc(50);
				if(!Connect_TCP_Server(ipAddr,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT,1))//���ӷ���������͸��ģʽ
				{
					LED1 = !LED1;
					printf("IP��ַ��%s\r\n",ipAddr);
					LCD_ShowString(160,40,24,8,16,"IP:");//��ʾ��ǰIP��ַ
					LCD_ShowString(184,40,100,8,16,ipAddr);
					myfree(ipAddr);
					LCD_ShowString(10,40,60,12,16,"CITY:");
					LCD_ShowString(10,60,60,12,16,"Temp:");
					LCD_ShowString(90,60,60,12,16,"'C");//��ʾ��ǰ����
					LCD_ShowString(10,80,60,12,16,"Text:");
				}
				else printf("ʧ��\r\n");
		}
		else if(key == WKUP_PRES)//wkup
		{
			key=0;
			printf("����wkup!\r\n");
			TIM_Cmd(TIM3, ENABLE);		//������ʱ��
		}
		
		if(Parse_Flag != 0)
		{
			response = mymalloc(1000);
			userData.city = mymalloc(50);
			userData.text = mymalloc(50);
			get_http((u8*)Request,response);//����get����
			cJSON_Parse_Weather((char*)response,&userData);
			printf("****************************************��������:\r\n");
			printf("userData->city:%s,\r\n",userData.city);
			printf("userData->text:%s,\r\n",userData.text);
			printf("userData->code:%d,\r\n",userData.code);
			printf("userData->temp:%d,\r\n",userData.temp);
			POINT_COLOR = DARKBLUE;
			if(strstr(userData.city,"Guilin"))
			{
				LCD_ShowString(55,40,60,12,16,userData.city);
			}
			else if(strstr(userData.city,"Guangzhou"))
			{
				LCD_ShowString(55,40,60,12,16,"Guangzhou");
			}	
			else if(strstr(userData.city,"Guigang"))
			{
				LCD_ShowString(55,40,60,12,16,"Guigang");
			}			
			LCD_ShowNum(55,60,userData.temp,3,16);//��ʾ��ǰ����		
			LCD_ShowString(55,80,80,12,16,userData.text);
			
			myfree(response);//�ͷ��ڴ�
			myfree(userData.city);
			myfree(userData.text);
			Parse_Flag = 0;
		}	
	}	
}	

