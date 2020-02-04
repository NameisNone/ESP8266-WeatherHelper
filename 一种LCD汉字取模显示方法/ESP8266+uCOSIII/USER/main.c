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
//ALIENTEK Mini STM32开发板范例代码2
//按键输入实验		   
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司


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
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);	 	//串口初始化为9600	
	USART2_Init(115200);  //初始化串口2波特率为11520
	LED_Init();		  	 	//初始化与LED连接的硬件接口
	LCD_Init();						//初始化液晶
	KEY_Init();          	//初始化与按键连接的硬件接口
	mem_init();					//初始化内存池
/*	while(!WiFi_module_check())//检查wifi模块是否存在
	{
		printf("未检测到wifi模块!!!\r\n");
		LED0 = 0;
	}
*/
	LCD_Clear(WHITE);
	while(1)//循环
	{
		//show_Hz32(60,100,"武汉加油",BLACK,WHITE);
		Show_Hz16(60,200,"武汉加油",BLACK,WHITE);
		LCD_ShowString(20,90,150,40,16,"China Come on!");
	}
		
	#if 0 
		key = KEY_Scan(0);
		if(key == KEY0_PRES)//key0
		{
			printf("按键key0!\r\n");
			if(!wifi_send_ATcmd("AT","OK",15))
			{
				LED1 = !LED1;
			}
		}
		else if(key == KEY1_PRES)//key1
		{
			printf("按键key1!\r\n");
			ipAddr = mymalloc(50);
				if(!Connect_TCP_Server(ipAddr,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT,1))//连接服务器开启透传模式
				{
					LED1 = !LED1;
					printf("IP地址：%s\r\n",ipAddr);
					myfree(ipAddr);
				}
				else
				{
					LED0 = 0;
				} 
		}
		else if(key == WKUP_PRES)//wkup
		{
			printf("按键wkup!\r\n");
			response = mymalloc(500);//分配内存给response指针
			userData = mymalloc(500);
			get_http((u8*)Request,response);//发送get请求
			cJSON_Parse_Weather((char*)response,userData);
			printf("****************************************数据如下:\r\n");
			printf("UserData->city:%s,\r\n",userData->city);
			printf("UserData->text:%s,\r\n",userData->text);
			printf("UserData->code:%d,\r\n",userData->code);
			printf("UserData->temp:%d,\r\n",userData->temp);
			myfree(response);//释放内存
			myfree(userData);//释放内存
			LED0 = !LED0;
			LED1 = !LED1;
		}
	}	
	#endif
		
}
