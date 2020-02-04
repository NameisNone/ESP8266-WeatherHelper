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
//ALIENTEK Mini STM32开发板范例代码2
//按键输入实验		   
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司

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
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);	 	//串口初始化为9600	
	USART2_Init(115200);  //初始化串口2波特率为11520
	LED_Init();		  	 	//初始化与LED连接的硬件接口
	LCD_Init();						//初始化液晶
	LCD_Display_Dir(1);//设置横屏
	KEY_Init();          	//初始化与按键连接的硬件接口
	TIM3_Int_Init(49999,7199);//初始化定时器3，每5s中断一次
	TIM_Cmd(TIM3, DISABLE);  //使能TIMx外设	
	mem_init();					//初始化内存池
	while(!WiFi_module_check())//检查wifi模块是否存在
	{
		POINT_COLOR=RED; 
		LCD_ShowString(0,120,200,30,16,"No WiFI Module!!");
		printf("未检测到wifi模块!!!\r\n");
		LED0 = 0;
	}
	LCD_Clear(WHITE);
	POINT_COLOR=BLACK; 
	LCD_ShowString(50,0,216,20,24,"WiFi WeatherHelper");
	POINT_COLOR=DARKBLUE; 
	LCD_DrawLine(0,30,320,30);
	LCD_DrawLine(145,30,145,240);
	while(1)//循环
	{	
		key = KEY_Scan(0);//按键扫描
		if(key == KEY0_PRES)//key0
		{
			key=0;
			printf("按键key0!\r\n");
			if(!wifi_send_ATcmd("AT","OK",15))
			{
				LED1 = !LED1;
			}
		}
		else if(key == KEY1_PRES)//key1
		{
			key=0;
			printf("按键key1!\r\n");
			ipAddr = mymalloc(50);
				if(!Connect_TCP_Server(ipAddr,TCP,(u8*)SSID,(u8*)PSWD,(u8*)SERVER,(u8*)PORT,1))//连接服务器开启透传模式
				{
					LED1 = !LED1;
					printf("IP地址：%s\r\n",ipAddr);
					LCD_ShowString(160,40,24,8,16,"IP:");//显示当前IP地址
					LCD_ShowString(184,40,100,8,16,ipAddr);
					myfree(ipAddr);
					LCD_ShowString(10,40,60,12,16,"CITY:");
					LCD_ShowString(10,60,60,12,16,"Temp:");
					LCD_ShowString(90,60,60,12,16,"'C");//显示当前气温
					LCD_ShowString(10,80,60,12,16,"Text:");
				}
				else printf("失败\r\n");
		}
		else if(key == WKUP_PRES)//wkup
		{
			key=0;
			printf("按键wkup!\r\n");
			TIM_Cmd(TIM3, ENABLE);		//开启定时器
		}
		
		if(Parse_Flag != 0)
		{
			response = mymalloc(1000);
			userData.city = mymalloc(50);
			userData.text = mymalloc(50);
			get_http((u8*)Request,response);//发送get请求
			cJSON_Parse_Weather((char*)response,&userData);
			printf("****************************************数据如下:\r\n");
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
			LCD_ShowNum(55,60,userData.temp,3,16);//显示当前气温		
			LCD_ShowString(55,80,80,12,16,userData.text);
			
			myfree(response);//释放内存
			myfree(userData.city);
			myfree(userData.text);
			Parse_Flag = 0;
		}	
	}	
}	

