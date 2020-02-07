#include "wifi.h"
#include "EspUart.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"
#include "cJSON.h"
#include "malloc.h"
#include "Weather.h"
#include "stdlib.h"
//#include "Hz.h"

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
	if(!wifi_send_ATcmd("AT","OK",50))//发送AT检测模块是否应答
		status = 1;
	else
		status = 0;
	return status;
}

/*连接wifi*/
/*连接成功会返回当前的IP地址*/
//0,连接成功并获取到ip地址，1：连接失败直接返回
u8 Connect_AP(u8 *IP_Addr,u8 *WiFi_SSID, u8 *WiFi_PSWD)
{
	u8 time=0;
	u8 *CWJAP=mymalloc(100);//CWJAP指令
	sprintf((char*)CWJAP,"AT+CWJAP=\"%s\",\"%s\"",(u8*)WiFi_SSID,(u8*)WiFi_PSWD);//打印wifi名称和密码到指令中
	printf("%s\r\n",CWJAP);
	if(!wifi_send_ATcmd((u8*)"AT",(u8*)"OK",30))//查看当前是否为AT指令配置状态
	{
		if(!wifi_send_ATcmd((u8*)"AT+CWMODE=1",OK,20))//设置STA模式
		{
			if(!wifi_send_ATcmd((u8*)"AT+CIFSR",(u8*)"0.0.0.0",30))//获取IP失败
			{
				while((wifi_send_ATcmd((u8*)CWJAP,(u8*)"OK",300)) && (time<=6))//发送连接指令，失败则重试重新连接6次
				{
					time++;
					#ifdef DEBUG_EN
						printf("连接wifi失败！！！正在尝试重新连接...\r\n");
						printf("重新连接第%d次。。\r\n",time);
					#endif
				}
				myfree(CWJAP);//释放内存	
				if(time>6)//连接失败
				{
					#ifdef DEBUG_EN
					printf("***************************************wifi连接失败！！！");
					#endif
					return 1;
				}
				else //连接成功
				{
					if(IP_Addr != NULL)//如果需要获取IP地址
						get_IP_Addr(IP_Addr);//获取IP地址
					return 0;
				}			
			}
			else
			{ //获取IP成功
				if(IP_Addr != NULL)//如果需要获取IP地址
						get_IP_Addr(IP_Addr);//获取IP地址
				return 0;
			}
		}
	}
	return 1;
}

/*连接服务器,连接失败会退出*/
//0，成功连接并进入透传模式；1，失败
u8 Connect_TCP_Server(u8 *ipAddr,u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port,u8 CIPMODE1_EN)
{
	u8 *CIPSTART=mymalloc(100);
	sprintf((char*)CIPSTART,"AT+CIPSTART=\"%s\",\"%s\",%s",ConnectMode,ServerIP,Port);//打印服务器信息到CIPSTART中
	#ifdef DEBUG_EN
	printf("CIPSTART内容:%s\r\n",CIPSTART);
	#endif
	
	//首先确定网络已连接
	if(!Connect_AP(ipAddr,WiFi_SSID,WiFi_PSWD))
	{
		if(!wifi_send_ATcmd((u8*)"AT+CWMODE=1",OK,20))//设置STA模式
		{
			#ifdef DEBUG_EN
				printf("设置AT+CWMODE=1,STA模式成功!\r\n");				
			#endif
			if(!wifi_send_ATcmd((u8*)"AT+CIPMUX=0",OK,300))//设置单连接
			{
				#ifdef DEBUG_EN
				printf("设置AT+CIPMUX=0单连接成功!\r\n");				
				#endif
				if(!wifi_send_ATcmd((u8*)CIPSTART,OK,300))//连接服务器
				{
					#ifdef DEBUG_EN
					printf("!!!成功连接服务器:%s\r\n",CIPSTART);				
					#endif
					myfree(CIPSTART);
					if(CIPMODE1_EN)
					{
						if(!wifi_send_ATcmd((u8*)"AT+CIPMODE=1",OK,20))//设置透传模式
						{
							#ifdef DEBUG_EN
							printf("!!!开启透传模式\r\n");				
							#endif
							if(!wifi_send_ATcmd((u8*)"AT+CIPSEND",OK,20))
							{
								#ifdef DEBUG_EN
								printf("!!!开始透传!!!\r\n");				
								#endif
								return 0;
							}					
						}
					}else return 0;
				}
			}
		}
	}
	myfree(CIPSTART);//释放内存
	return 1;//连接服务器失败
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

/*退出当前服务器连接*/
void Exit_ServerConnect(void)
{
	if(wifi_send_ATcmd("AT","OK",20))//处于透传模式
	{
		if(!Exit_TransMode())//退出透传模式
		{
			if(!wifi_send_ATcmd("AT+CIPCLOSE","OK",50))//关闭当前TCP服务器连接
			{
				#ifdef DEBUG_EN
				printf("已断开服务器连接\r\n");
				#endif
			}
		}else return;
	}
	else //AT指令模式
	{
		if(!wifi_send_ATcmd("AT+CIPCLOSE","OK",50))//关闭当前TCP服务器连接
			{
				#ifdef DEBUG_EN
				printf("已断开服务器连接\r\n");
				#endif
			}else return;
	}
}

//退出当前服务器单连接重新连接其他服务器
/*返回值为0，成功；否则失败*/
u8 Connect_NewServer(u8* Server)
{
	char *CIPSTART = mymalloc(50);
	sprintf(CIPSTART,"AT+CIPSTART=\"TCP\",\"%s\",80",Server);
	Exit_ServerConnect();//退出当前连接
	if(!wifi_send_ATcmd((u8*)CIPSTART,(u8*)"OK",50))//连接服务器成功
	{
		#ifdef DEBUG_EN
			printf("成功连接到---->%s\r\n",Server);
		#endif
		if(!wifi_send_ATcmd((u8*)"AT+CIPMODE=1",(u8*)"OK",50))//设置透传模式
		{
			#ifdef DEBUG_EN
				printf("开启透传模式!!\r\n");
			#endif
			if(!wifi_send_ATcmd("AT+CIPSEND","OK",20))
			{
				#ifdef DEBUG_EN
					printf("开始透传!!\r\n");
				#endif
				return 0;
			}
			return 1;
		}
		return 1;
	}else{
		#ifdef DEBUG_EN
			printf("连接%s失败！！\r\n",Server);
		#endif
		return 1;
	}
}

//WeatherData *userData;
/*JSON解析函数*/
void cJSON_Parse_Weather(char *jsonBuf,WeatherData *userData)
{
	cJSON* cjson_main;
	cJSON* cjson_results;
	cJSON* cjson_resultsArray_Obj;
	cJSON *cjson_Key, *cjson_Value;
	//u8 *json_str;
	
	cjson_main = cJSON_Parse(jsonBuf);//解析json整体
	if(cjson_main==NULL)
	{
		printf("Error before: [%s]\r\n",cJSON_GetErrorPtr()); //打印数据包语法错误的位置
		#ifdef DEBUG_EN
			printf("cjson解析失败！\r\n");
		#endif
		return ;
	}
	
/*	#ifdef DEBUG_EN
		printf("\r\n\r\n\r\n*******************解析结果*******************\r\n");
		json_str = mymalloc(500);
		json_str = (u8*)cJSON_Print(cjson_main);//打印json数据整体
		printf("%s\r\n",json_str);
		myfree(json_str);
	#endif*/

	//解析results
	if((cjson_results = cJSON_GetObjectItem(cjson_main,"results")) != NULL)
	{
		int Arraysize = cJSON_GetArraySize(cjson_results);//获取results数组成员数
		#ifdef DEBUG_EN
		printf("cjson解析开始！\r\n");
		printf("results Arraysize:%d\r\n",Arraysize);//打印数组大小，大小为1
		#endif

		if((cjson_resultsArray_Obj = cJSON_GetArrayItem(cjson_results,0)) != NULL)//解析results
		{
#if 1			/*解析location*/
			printf("解析location：\r\n");
			if((cjson_Key = cJSON_GetObjectItem(cjson_resultsArray_Obj,"location"))!=NULL)//解析location
			{
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"name"))!=NULL)//解析name
				{
					sprintf((char*)userData->city,"%s",cjson_Value->valuestring);//把name的数据装入结构体成员
					#ifdef DEBUG_EN
					printf("\"city\":%s\r\n",userData->city);
					printf("\"name\":%s\r\n",cjson_Value->valuestring);//打印name的值
					#endif
					//cJSON_Delete(cjson_Key);//释放内存
				}
			}
#endif				
			/*解析now*/
			printf("解析now：\r\n");
			if((cjson_Key = cJSON_GetObjectItem(cjson_resultsArray_Obj,"now"))!=NULL)//解析now
			{	
#if 1				
				printf("now\r\n");
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"text"))!=NULL)//解析text
				{
					sprintf((char*)userData->text,"%s",cjson_Value->valuestring);//天气现象装入结构体成员
					#ifdef DEBUG_EN
					printf("\"text\":%s,\r\n",cjson_Value->valuestring);//打印text的值
					#endif
					//cJSON_Delete(cjson_Value);//删除text释放内存
				}
#endif
			
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"code"))!=NULL)//解析text
				{					
					userData->code = (char)atoi(cjson_Value->valuestring);//将字符串转换为char类型
					#ifdef DEBUG_EN
					printf("\"code\":%s,\r\n",cjson_Value->valuestring);//打印text的值
					#endif
					//cJSON_Delete(cjson_Value);//释放内存
				}
				
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"temperature"))!=NULL)//解析text
				{
					userData->temp = (char)atoi(cjson_Value->valuestring);//字符串转int
					#ifdef DEBUG_EN
					printf("\"temperature\":%s,\r\n",cjson_Value->valuestring);//打印text的值
					#endif
					//cJSON_Delete(cjson_Value);//释放内存
				}
				cJSON_Delete(cjson_main);//删除所有结构体,释放堆的内存
			}
			else
			{
				printf("Error before: [%s]\r\n",cJSON_GetErrorPtr()); //打印数据包语法错误的位置
				return ;
			}
		}
		/*解析lastupdate*/
	}
}


/*
 *发送http请求，解析接收到的数据
 *使用此函数前必须先连接服务器并且进入透传模式 
 */
void get_http(u8* request, u8 *response)
{
	u8 *p = NULL;
	u8 *p1 = NULL;
	u8 *p2 = NULL;
	if(atk_8266_send_data(request,0,600))//如果请求失败
	{
		response[0]=0;
		//printf("请求失败1!\r\n");
		return;
	}
	//请求成功
	delay_ms(1000);
	if(USART2_RX_STA&0X8000)//接收到期待的应答结果
	{
		if((p = wifi_check_cmd("{")) != NULL)//检查接收的数据是否正确，即是否为 { 开头的
		{
			p1=(u8*)strstr((const char*)(p),"]");//先查找 ] 符号
			p2=(u8*)strstr((const char*)(p1+1),"}");//查找 } 符号，标记json数据的结尾
			*(p2+1)=0;//加入结束符
			//printf("die here!\r\n");
			sprintf((char*)response,"%s",p);	//打印json串到response中
			//printf("I am not die!\r\n");
			#ifdef DEBUG_EN
			printf("接收到的数据:\r\n%s\r\n",response);//打印数据
			#endif
		}
	}
	else return ;	
}


void Get_myApi_Http(u8* request, FY_StructDef *Data)
{
	u8 BUF[150]={0};
	//u8 *dataBuf=mymalloc(5);
	char *p=NULL;
	char *p1=NULL;
	char *p2=NULL;
	if(atk_8266_send_data(request,0,600))//发送请求失败
	{
		Data = NULL;
		//printf("请求失败1!\r\n");
		return;
	}
	delay_us(500000);printf("delay_us!\r\n");
	if(USART2_RX_STA&0X8000)//接收到数据
	{	printf("recv Data!\r\n");
		USART2_RX_BUF[150]=0;//只接收前面的150个字节
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		//BUF = USART2_RX_BUF;//赋值给buf
		if((p=strstr((const char*)BUF,"confirm")) != NULL)//查找confirm确诊人数
		{
			if((p1=strstr((const char*)p,":")) != NULL)//查找:
			{		
				if((p2=strstr((const char*)p1,","))!=NULL)//查找,
				{
					*p2='\0';//,位置替换为结束符
					Data->confirm = atoi(p1+1);//传入确诊人数数据
				}
			}			
		}
		//printf("USART2_RX_BUF:%s\r\n",USART2_RX_BUF);
		//printf("BUF:%s\r\n",BUF);
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"suspect")) != NULL)
		{printf("find suspect!\r\n");
			if((p1=strstr((const char*)p,":")) != NULL)//查找:
				{	printf("find :!\r\n");
				if((p2=strstr((const char*)p1,","))!=NULL)//查找,
				{
					*p2='\0';//,位置替换为结束符
					Data->suspect = atoi(p1+1);//传入疑似病例数据		
					printf("Data->suspect:%d\r\n",Data->suspect);
				}
			}	
		}
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"dead")) != NULL)
		{printf("find dead!\r\n");
			if((p1=strstr((const char*)p,":")) != NULL)//查找:
				{	printf("find :!\r\n");
				if((p2=strstr((const char*)p1,","))!=NULL)//查找,
				{
					*p2='\0';//,位置替换为结束符
					Data->dead = atoi(p1+1);//传入疑似病例数据		
					printf("Data->dead:%d\r\n",Data->dead);
				}
			}	
		}
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"heal")) != NULL)
		{printf("find heal!\r\n");
			if((p1=strstr((const char*)p,":")) != NULL)//查找:
				{	printf("find :!\r\n");
				if((p2=strstr((const char*)p1,","))!=NULL)//查找,
				{
					*p2='\0';//,位置替换为结束符
					Data->heal = atoi(p1+1);//传入疑似病例数据		
					printf("Data->heal:%d\r\n",Data->heal);
				}
			}	
		}
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"create_time")) != NULL)//查找当前日期
		{printf("find create_time!\r\n");
			if((p1=strstr((const char*)(p+12),"\"")) != NULL)//查找"
				{	printf("find \"!\r\n");
					if((p2=strstr((const char*)p1+1,"\""))!=NULL)//查找下一个"
					{
						*p2='\0';//,位置替换为结束符
						sprintf((char*)Data->date,"%s",p1+1);
						printf("                Data->date:%s\r\n",Data->date);
					}
			}	
		}else 
		{
			#ifdef DEBUG_EN
			printf("can not find!\r\n");
			#endif	
		}
//		myfree(BUF);
//		myfree(p);
//		myfree(p1);
//		myfree(p2);
	}else //未收到数据
	{
		#ifdef DEBUG_EN
			printf("recv failed!\r\n");
		#endif
			return;
	}
}
	
/*在LCD上显示天气现象*//*
void show_weather(u8 x,u8 y,u16 Wcolor,u16 backColor,u8 Code)
{
	switch(Code)
	{
		case 0:show_Hz32(x, y,"晴", Wcolor, backColor);
			break;
		case 1:show_Hz32(x, y,"晴", Wcolor, backColor);
			break;
		case 2:show_Hz32(x, y,"晴", Wcolor, backColor);
			break;
		case 3:show_Hz32(x, y,"晴", Wcolor, backColor);
			break;
		case 4:show_Hz32(x, y,"多云", Wcolor, backColor);
			break;
		case 5:show_Hz32(x, y,"晴转多云", Wcolor, backColor);
			break;		
		case 6:show_Hz32(x, y,"晴转多云", Wcolor, backColor);
			break;	
		case 7:show_Hz32(x, y,"大部多云", Wcolor, backColor);
			break;	
		case 8:show_Hz32(x, y,"大部多云", Wcolor, backColor);
			break;	
		case 9:show_Hz32(x, y,"阴", Wcolor, backColor);
			break;			
		case 10:show_Hz32(x, y,"阵雨", Wcolor, backColor);
			break;	
		case 11:show_Hz32(x, y,"雷阵雨", Wcolor, backColor);
			break;	
			
		case 13:show_Hz32(x, y,"小雨", Wcolor, backColor);
			break;	
		case 14:show_Hz32(x, y,"中雨", Wcolor, backColor);
			break;	
		case 15:show_Hz32(x, y,"大雨", Wcolor, backColor);
			break;	
		case 16:show_Hz32(x, y,"暴雨", Wcolor, backColor);
			break;	
		case 17:show_Hz32(x, y,"大暴雨", Wcolor, backColor);
			break;	
		case 18:show_Hz32(x, y,"特大暴雨", Wcolor, backColor);
				break;
		case 20:show_Hz32(x, y,"雨夹雪", Wcolor, backColor);
			break;	
		case 21:show_Hz32(x, y,"阵雪", Wcolor, backColor);
			break;	
		case 22:show_Hz32(x, y,"小雪", Wcolor, backColor);
				break;
		default: 
			break;
	}
}*/






