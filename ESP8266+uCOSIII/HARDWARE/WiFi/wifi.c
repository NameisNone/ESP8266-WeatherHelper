#include "wifi.h"
#include "EspUart.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"

/*发送AT指令函数*/
//0，发送成功，1，发送失败
u8 wifi_send_ATcmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//发送命令
	printf("send:%s\r\n",cmd);
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(wifi_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
					break;//得到有效数据 
				}
					USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}


//ATK-ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* wifi_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);//查找USART2_RX_BUF中含有str的位置并返回
	} 
	return (u8*)strx;
}


/*检测模块是否存在并且连接在串口2上
 *返回值:0,不存在；1，存在
 */
u8 WiFi_module_check(void)
{
	u8 status=0;
	if(!wifi_send_ATcmd("AT","OK",30))//发送AT检测模块是否应答
		status = 1;
	else
		status = 0;
	return status;
}

/*连接心知天气服务器*/
//0，成功连接进入透传模式；1，失败
u8 Connect_TCP_Server(u8 *ESP8266_Mode, u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port)
{
	__align(8) u8 CWJAP[200]={0};
	__align(8) u8 CIPSTART[200]={0};
	sprintf((char*)CWJAP,"AT+CWJAP=\"%s\",\"%s\"",(u8*)WiFi_SSID,(u8*)WiFi_PSWD);//打印wifi名称和密码到指令中
	sprintf((char*)CIPSTART,"AT+CIPSTART=\"%s\",\"%s\",%s",ConnectMode,ServerIP,Port);//打印服务器信息到CIPSTART中
	printf("CWJAP:%s\r\n",CWJAP);
	printf("CIPSTART:%s\r\n",CIPSTART);
	if(!wifi_send_ATcmd(ESP8266_Mode,OK,20))//1.首先设置STA模式
	{
	  #ifdef DEBUG_EN
				printf("STA模式设置成功！\r\n");
		#endif
		while(wifi_send_ATcmd((u8*)CWJAP,"OK",300))//2.接着连接wifi热点
		{
			#ifdef DEBUG_EN
				printf("**********************\r\n");
				printf("连接wifi成功！\r\n");
				printf("当前wifi热点为：");
				printf("SSID:%s\r\n",WiFi_SSID);
				printf("PSWD:%s\r\n",WiFi_PSWD);
				printf("**********************\r\n");
			#endif
			if(!wifi_send_ATcmd((u8*)"AT+CIPMUX=0",OK,300))//3.设置单连接
			{
				#ifdef DEBUG_EN
				printf("设置单连接成功!\r\n");				
				#endif
				if(!wifi_send_ATcmd((u8*)CIPSTART,OK,300))//4.连接服务器
				{
					#ifdef DEBUG_EN
					printf("连接%s成功!\r\n",ServerIP);				
					#endif
					if(!wifi_send_ATcmd((u8*)"AT+CIPMODE=1",OK,20))//5.设置透传模式
					{
						#ifdef DEBUG_EN
						printf("设置透传模式!\r\n");				
						#endif
						if(!wifi_send_ATcmd((u8*)"AT+CIPSEND",OK,20))
						{
							#ifdef DEBUG_EN
							printf("开启透传模式!\r\n");				
							#endif
							return 0;
						}else return 1;
					}else return 1;
				}
				else return 1;
			}
			else return 1;
		}
		//else return 1;
	}
	else return 1;
}

