#include "wifi.h"
#include "EspUart.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"
#include "cJSON.h"

/*发送AT指令函数*/
//0，发送成功，1，发送失败
u8 wifi_send_ATcmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//发送命令
	#ifdef DEBUG_EN
	printf("send:%s\r\n",cmd);
	#endif
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(wifi_check_cmd(ack))
				{
					#ifdef DEBUG_EN
					printf("ack:%s\r\n",(u8*)ack);
					#endif
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

/*连接服务器*/
//0，成功连接并进入透传模式；1，失败
u8 Connect_TCP_Server(u8 *ESP8266_Mode, u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port)
{
	__align(8) u8 CWJAP[100]={0};
	__align(8) u8 CIPSTART[100]={0};
	sprintf((char*)CWJAP,"AT+CWJAP=\"%s\",\"%s\"",(u8*)WiFi_SSID,(u8*)WiFi_PSWD);//打印wifi名称和密码到指令中
	sprintf((char*)CIPSTART,"AT+CIPSTART=\"%s\",\"%s\",%s",ConnectMode,ServerIP,Port);//打印服务器信息到CIPSTART中
	#ifdef DEBUG_EN
	printf("CWJAP:%s\r\n",CWJAP);
	printf("CIPSTART:%s\r\n",CIPSTART);
	#endif
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
					}//else return 1;
				}
				//else return 1;
			}
			//else return 1;
		}
		//else return 1;
	}
	//else return 1;
	
	return 1;
}


/*退出透传模式*/
//0成功，1失败
u8 Exit_TransMode(void)
{
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==0);
  USART_SendData(USART2,'+');
	//delay_ms(15);	
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==0);
  USART_SendData(USART2,'+');
	//delay_ms(15);	
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==0);
  USART_SendData(USART2,'+');
	delay_ms(500);
	if(!wifi_send_ATcmd("AT","OK",20))
	{
		#ifdef DEBUG_EN
			printf("退出透传模式成功！\r\n");
		#endif
		return 0;
	}
	return 1;
}

/*获取IP地址*/
void get_IP_Addr(u8* ipbuf)
{
	u8 *p,*p1;
		if(wifi_send_ATcmd("AT+CIFSR","OK",50))//获取WAN IP地址失败
		{
			ipbuf[0]=0;
			return;
		}		
		p=wifi_check_cmd("\"");//在返回的数据中查找第一个"号的位置，第一对""中就是IP地址
		p1=(u8*)strstr((const char*)(p+1),"\"");//查找第二个 " 符号，找到IP地址结尾
		*p1=0;//加入字符串结束符/0
		sprintf((char*)ipbuf,"%s",p+1);	//打印IP地址字符串到ipbuf中
}

//向ATK-ESP8266发送指定数据
//data:发送的数据(不需要添加回车了)
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s",data);	//发送数据
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(wifi_check_cmd(ack))break;//得到有效数据 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}


/*JSON解析函数*/
void cJSON_Parse_Uart(char* jsonBuf)
{
	cJSON* cjson_main = NULL;
	cJSON* cjson_results = NULL;
	cJSON* cjson_resultsArray = NULL;
	cJSON* cjson_keyvalue = NULL;
	
/*	cJSON* cjson_location = NULL;
	cJSON* cjson_name = NULL;,char *realData
	cJSON* cjson_now = NULL;
	cJSON* cjson_text = NULL;
	cJSON* cjson_code = NULL;
	cJSON* cjson_temp = NULL; */
	
	cjson_main = cJSON_Parse(jsonBuf);//解析json整体
	if(cjson_main==NULL)
	{
		#ifdef DEBUG_EN
		printf("cjson解析失败！\r\n");
		#endif
		return ;
	}
	
	if((cjson_results = cJSON_GetObjectItem(cjson_main,"results")) != NULL)//解析results对象成功
	{
		int Arraysize = cJSON_GetArraySize(cjson_results);//获取results数组成员数
		
		/*解析location*/
		if((cjson_resultsArray = cJSON_GetArrayItem(cjson_results,0)) != NULL)//解析第1个成员location
		{
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"name"))!=NULL)//解析数组成员location中name的键值
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"name\":%s,\r\n",cjson_keyvalue->valuestring);//打印name的值
				#endif
				cJSON_Delete(cjson_resultsArray);//释放内存
			}
		}
		
		/*解析now*/
		if((cjson_resultsArray = cJSON_GetArrayItem(cjson_results,1)) != NULL)//解析第2个成员now
		{
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"text"))!=NULL)//解析text键值
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"text\":%s,\r\n",cjson_keyvalue->valuestring);//打印name的值
				#endif
				cJSON_Delete(cjson_keyvalue);//释放内存
			}
			
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"code"))!=NULL)//解析code键值
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"code\":%s,\r\n",cjson_keyvalue->valuestring);//打印name的值
				#endif
				cJSON_Delete(cjson_keyvalue);//释放内存
			}
			
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"temperature"))!=NULL)//解析text键值
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"temperature\":%s\r\n",cjson_keyvalue->valuestring);//打印name的值
				#endif
				cJSON_Delete(cjson_keyvalue);//释放内存
			}
			cJSON_Delete(cjson_resultsArray);//释放内存
		}
			
		/*解析lastupdate*/
	}
}


/*
 *发送http请求，解析接收到的数据
 *使用此函数前必须先连接服务器并且进入透传模式 
 */
void get_http(u8* request)
{
	__align(8) u8 response[300]={0};//mark： response的问题
	u8 *p = NULL;
	u8 *p1 = NULL;
	u8 *p2 = NULL;
	if(atk_8266_send_data(request,0,600))//如果请求失败
	{
		response[0]=0;
		printf("请求失败1!\r\n");
		return;
	}
	//请求成功
	delay_ms(1000);
	if(USART2_RX_STA&0X8000)//接收到期待的应答结果
	{
		if((p = wifi_check_cmd("{")) != NULL)//检查接收的数据是否正确，即是否为 { 开头的
		{
			printf("请求失败2!\r\n");
			p1=(u8*)strstr((const char*)(p),"]");//先查找 ] 符号
			printf("p1=(u8*)strstr((const char*)(p),\"]\");\r\n");
			p2=(u8*)strstr((const char*)(p1+1),"}");//查找 } 符号，标记json数据的结尾
			printf("p1=(u8*)strstr((const char*)(p1+1),\"}\");\r\n");
			*(p2+1)=0;//加入结束符
			
			printf("die here!\r\n");
			sprintf((char*)response,"%s",p);	//打印json串到response中
			printf("I am not die!\r\n");
			#ifdef DEBUG_EN
			printf("接收到的数据:%s\r\n",response);//打印数据
			#endif
		}
	}
	else 
	{
		printf("请求失败3!\r\n");
		return ;
	}
}
