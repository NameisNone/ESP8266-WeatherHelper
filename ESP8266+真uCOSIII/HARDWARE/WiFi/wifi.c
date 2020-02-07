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

/*����ATָ���*/
//0�����ͳɹ���1������ʧ��
u8 wifi_send_ATcmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//��������
	#ifdef DEBUG_EN
	printf("send:%s\r\n",cmd);
	#endif
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(wifi_check_cmd(ack))
				{
					#ifdef DEBUG_EN
					printf("ack:%s\r\n",(u8*)ack);
					#endif
					break;//�õ���Ч���� 
				}
					USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}


//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* wifi_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);//����USART2_RX_BUF�к���str��λ�ò�����
	} 
	return (u8*)strx;
}


/*���ģ���Ƿ���ڲ��������ڴ���2��
 *����ֵ:0,�����ڣ�1������
 */
u8 WiFi_module_check(void)
{
	u8 status=0;
	if(!wifi_send_ATcmd("AT","OK",50))//����AT���ģ���Ƿ�Ӧ��
		status = 1;
	else
		status = 0;
	return status;
}

/*����wifi*/
/*���ӳɹ��᷵�ص�ǰ��IP��ַ*/
//0,���ӳɹ�����ȡ��ip��ַ��1������ʧ��ֱ�ӷ���
u8 Connect_AP(u8 *IP_Addr,u8 *WiFi_SSID, u8 *WiFi_PSWD)
{
	u8 time=0;
	u8 *CWJAP=mymalloc(100);//CWJAPָ��
	sprintf((char*)CWJAP,"AT+CWJAP=\"%s\",\"%s\"",(u8*)WiFi_SSID,(u8*)WiFi_PSWD);//��ӡwifi���ƺ����뵽ָ����
	printf("%s\r\n",CWJAP);
	if(!wifi_send_ATcmd((u8*)"AT",(u8*)"OK",30))//�鿴��ǰ�Ƿ�ΪATָ������״̬
	{
		if(!wifi_send_ATcmd((u8*)"AT+CWMODE=1",OK,20))//����STAģʽ
		{
			if(!wifi_send_ATcmd((u8*)"AT+CIFSR",(u8*)"0.0.0.0",30))//��ȡIPʧ��
			{
				while((wifi_send_ATcmd((u8*)CWJAP,(u8*)"OK",300)) && (time<=6))//��������ָ�ʧ����������������6��
				{
					time++;
					#ifdef DEBUG_EN
						printf("����wifiʧ�ܣ��������ڳ�����������...\r\n");
						printf("�������ӵ�%d�Ρ���\r\n",time);
					#endif
				}
				myfree(CWJAP);//�ͷ��ڴ�	
				if(time>6)//����ʧ��
				{
					#ifdef DEBUG_EN
					printf("***************************************wifi����ʧ�ܣ�����");
					#endif
					return 1;
				}
				else //���ӳɹ�
				{
					if(IP_Addr != NULL)//�����Ҫ��ȡIP��ַ
						get_IP_Addr(IP_Addr);//��ȡIP��ַ
					return 0;
				}			
			}
			else
			{ //��ȡIP�ɹ�
				if(IP_Addr != NULL)//�����Ҫ��ȡIP��ַ
						get_IP_Addr(IP_Addr);//��ȡIP��ַ
				return 0;
			}
		}
	}
	return 1;
}

/*���ӷ�����,����ʧ�ܻ��˳�*/
//0���ɹ����Ӳ�����͸��ģʽ��1��ʧ��
u8 Connect_TCP_Server(u8 *ipAddr,u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port,u8 CIPMODE1_EN)
{
	u8 *CIPSTART=mymalloc(100);
	sprintf((char*)CIPSTART,"AT+CIPSTART=\"%s\",\"%s\",%s",ConnectMode,ServerIP,Port);//��ӡ��������Ϣ��CIPSTART��
	#ifdef DEBUG_EN
	printf("CIPSTART����:%s\r\n",CIPSTART);
	#endif
	
	//����ȷ������������
	if(!Connect_AP(ipAddr,WiFi_SSID,WiFi_PSWD))
	{
		if(!wifi_send_ATcmd((u8*)"AT+CWMODE=1",OK,20))//����STAģʽ
		{
			#ifdef DEBUG_EN
				printf("����AT+CWMODE=1,STAģʽ�ɹ�!\r\n");				
			#endif
			if(!wifi_send_ATcmd((u8*)"AT+CIPMUX=0",OK,300))//���õ�����
			{
				#ifdef DEBUG_EN
				printf("����AT+CIPMUX=0�����ӳɹ�!\r\n");				
				#endif
				if(!wifi_send_ATcmd((u8*)CIPSTART,OK,300))//���ӷ�����
				{
					#ifdef DEBUG_EN
					printf("!!!�ɹ����ӷ�����:%s\r\n",CIPSTART);				
					#endif
					myfree(CIPSTART);
					if(CIPMODE1_EN)
					{
						if(!wifi_send_ATcmd((u8*)"AT+CIPMODE=1",OK,20))//����͸��ģʽ
						{
							#ifdef DEBUG_EN
							printf("!!!����͸��ģʽ\r\n");				
							#endif
							if(!wifi_send_ATcmd((u8*)"AT+CIPSEND",OK,20))
							{
								#ifdef DEBUG_EN
								printf("!!!��ʼ͸��!!!\r\n");				
								#endif
								return 0;
							}					
						}
					}else return 0;
				}
			}
		}
	}
	myfree(CIPSTART);//�ͷ��ڴ�
	return 1;//���ӷ�����ʧ��
}


/*�˳�͸��ģʽ*/
//0�ɹ���1ʧ��
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
			printf("�˳�͸��ģʽ�ɹ���\r\n");
		#endif
		return 0;
	}
	return 1;
}

/*��ȡIP��ַ*/
void get_IP_Addr(u8* ipbuf)
{
		u8 *p,*p1;
		if(wifi_send_ATcmd("AT+CIFSR","OK",50))//��ȡWAN IP��ַʧ��
		{
			ipbuf[0]=0;
			return;
		}		
		p=wifi_check_cmd("\"");//�ڷ��ص������в��ҵ�һ��"�ŵ�λ�ã���һ��""�о���IP��ַ
		p1=(u8*)strstr((const char*)(p+1),"\"");//���ҵڶ��� " ���ţ��ҵ�IP��ַ��β
		*p1=0;//�����ַ���������/0
		sprintf((char*)ipbuf,"%s",p+1);	//��ӡIP��ַ�ַ�����ipbuf��
}

//��ATK-ESP8266����ָ������
//data:���͵�����(����Ҫ��ӻس���)
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s",data);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(wifi_check_cmd(ack))break;//�õ���Ч���� 
				USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
}

/*�˳���ǰ����������*/
void Exit_ServerConnect(void)
{
	if(wifi_send_ATcmd("AT","OK",20))//����͸��ģʽ
	{
		if(!Exit_TransMode())//�˳�͸��ģʽ
		{
			if(!wifi_send_ATcmd("AT+CIPCLOSE","OK",50))//�رյ�ǰTCP����������
			{
				#ifdef DEBUG_EN
				printf("�ѶϿ�����������\r\n");
				#endif
			}
		}else return;
	}
	else //ATָ��ģʽ
	{
		if(!wifi_send_ATcmd("AT+CIPCLOSE","OK",50))//�رյ�ǰTCP����������
			{
				#ifdef DEBUG_EN
				printf("�ѶϿ�����������\r\n");
				#endif
			}else return;
	}
}

//�˳���ǰ������������������������������
/*����ֵΪ0���ɹ�������ʧ��*/
u8 Connect_NewServer(u8* Server)
{
	char *CIPSTART = mymalloc(50);
	sprintf(CIPSTART,"AT+CIPSTART=\"TCP\",\"%s\",80",Server);
	Exit_ServerConnect();//�˳���ǰ����
	if(!wifi_send_ATcmd((u8*)CIPSTART,(u8*)"OK",50))//���ӷ������ɹ�
	{
		#ifdef DEBUG_EN
			printf("�ɹ����ӵ�---->%s\r\n",Server);
		#endif
		if(!wifi_send_ATcmd((u8*)"AT+CIPMODE=1",(u8*)"OK",50))//����͸��ģʽ
		{
			#ifdef DEBUG_EN
				printf("����͸��ģʽ!!\r\n");
			#endif
			if(!wifi_send_ATcmd("AT+CIPSEND","OK",20))
			{
				#ifdef DEBUG_EN
					printf("��ʼ͸��!!\r\n");
				#endif
				return 0;
			}
			return 1;
		}
		return 1;
	}else{
		#ifdef DEBUG_EN
			printf("����%sʧ�ܣ���\r\n",Server);
		#endif
		return 1;
	}
}

//WeatherData *userData;
/*JSON��������*/
void cJSON_Parse_Weather(char *jsonBuf,WeatherData *userData)
{
	cJSON* cjson_main;
	cJSON* cjson_results;
	cJSON* cjson_resultsArray_Obj;
	cJSON *cjson_Key, *cjson_Value;
	//u8 *json_str;
	
	cjson_main = cJSON_Parse(jsonBuf);//����json����
	if(cjson_main==NULL)
	{
		printf("Error before: [%s]\r\n",cJSON_GetErrorPtr()); //��ӡ���ݰ��﷨�����λ��
		#ifdef DEBUG_EN
			printf("cjson����ʧ�ܣ�\r\n");
		#endif
		return ;
	}
	
/*	#ifdef DEBUG_EN
		printf("\r\n\r\n\r\n*******************�������*******************\r\n");
		json_str = mymalloc(500);
		json_str = (u8*)cJSON_Print(cjson_main);//��ӡjson��������
		printf("%s\r\n",json_str);
		myfree(json_str);
	#endif*/

	//����results
	if((cjson_results = cJSON_GetObjectItem(cjson_main,"results")) != NULL)
	{
		int Arraysize = cJSON_GetArraySize(cjson_results);//��ȡresults�����Ա��
		#ifdef DEBUG_EN
		printf("cjson������ʼ��\r\n");
		printf("results Arraysize:%d\r\n",Arraysize);//��ӡ�����С����СΪ1
		#endif

		if((cjson_resultsArray_Obj = cJSON_GetArrayItem(cjson_results,0)) != NULL)//����results
		{
#if 1			/*����location*/
			printf("����location��\r\n");
			if((cjson_Key = cJSON_GetObjectItem(cjson_resultsArray_Obj,"location"))!=NULL)//����location
			{
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"name"))!=NULL)//����name
				{
					sprintf((char*)userData->city,"%s",cjson_Value->valuestring);//��name������װ��ṹ���Ա
					#ifdef DEBUG_EN
					printf("\"city\":%s\r\n",userData->city);
					printf("\"name\":%s\r\n",cjson_Value->valuestring);//��ӡname��ֵ
					#endif
					//cJSON_Delete(cjson_Key);//�ͷ��ڴ�
				}
			}
#endif				
			/*����now*/
			printf("����now��\r\n");
			if((cjson_Key = cJSON_GetObjectItem(cjson_resultsArray_Obj,"now"))!=NULL)//����now
			{	
#if 1				
				printf("now\r\n");
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"text"))!=NULL)//����text
				{
					sprintf((char*)userData->text,"%s",cjson_Value->valuestring);//��������װ��ṹ���Ա
					#ifdef DEBUG_EN
					printf("\"text\":%s,\r\n",cjson_Value->valuestring);//��ӡtext��ֵ
					#endif
					//cJSON_Delete(cjson_Value);//ɾ��text�ͷ��ڴ�
				}
#endif
			
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"code"))!=NULL)//����text
				{					
					userData->code = (char)atoi(cjson_Value->valuestring);//���ַ���ת��Ϊchar����
					#ifdef DEBUG_EN
					printf("\"code\":%s,\r\n",cjson_Value->valuestring);//��ӡtext��ֵ
					#endif
					//cJSON_Delete(cjson_Value);//�ͷ��ڴ�
				}
				
				if((cjson_Value = cJSON_GetObjectItem(cjson_Key,"temperature"))!=NULL)//����text
				{
					userData->temp = (char)atoi(cjson_Value->valuestring);//�ַ���תint
					#ifdef DEBUG_EN
					printf("\"temperature\":%s,\r\n",cjson_Value->valuestring);//��ӡtext��ֵ
					#endif
					//cJSON_Delete(cjson_Value);//�ͷ��ڴ�
				}
				cJSON_Delete(cjson_main);//ɾ�����нṹ��,�ͷŶѵ��ڴ�
			}
			else
			{
				printf("Error before: [%s]\r\n",cJSON_GetErrorPtr()); //��ӡ���ݰ��﷨�����λ��
				return ;
			}
		}
		/*����lastupdate*/
	}
}


/*
 *����http���󣬽������յ�������
 *ʹ�ô˺���ǰ���������ӷ��������ҽ���͸��ģʽ 
 */
void get_http(u8* request, u8 *response)
{
	u8 *p = NULL;
	u8 *p1 = NULL;
	u8 *p2 = NULL;
	if(atk_8266_send_data(request,0,600))//�������ʧ��
	{
		response[0]=0;
		//printf("����ʧ��1!\r\n");
		return;
	}
	//����ɹ�
	delay_ms(1000);
	if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
	{
		if((p = wifi_check_cmd("{")) != NULL)//�����յ������Ƿ���ȷ�����Ƿ�Ϊ { ��ͷ��
		{
			p1=(u8*)strstr((const char*)(p),"]");//�Ȳ��� ] ����
			p2=(u8*)strstr((const char*)(p1+1),"}");//���� } ���ţ����json���ݵĽ�β
			*(p2+1)=0;//���������
			//printf("die here!\r\n");
			sprintf((char*)response,"%s",p);	//��ӡjson����response��
			//printf("I am not die!\r\n");
			#ifdef DEBUG_EN
			printf("���յ�������:\r\n%s\r\n",response);//��ӡ����
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
	if(atk_8266_send_data(request,0,600))//��������ʧ��
	{
		Data = NULL;
		//printf("����ʧ��1!\r\n");
		return;
	}
	delay_us(500000);printf("delay_us!\r\n");
	if(USART2_RX_STA&0X8000)//���յ�����
	{	printf("recv Data!\r\n");
		USART2_RX_BUF[150]=0;//ֻ����ǰ���150���ֽ�
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		//BUF = USART2_RX_BUF;//��ֵ��buf
		if((p=strstr((const char*)BUF,"confirm")) != NULL)//����confirmȷ������
		{
			if((p1=strstr((const char*)p,":")) != NULL)//����:
			{		
				if((p2=strstr((const char*)p1,","))!=NULL)//����,
				{
					*p2='\0';//,λ���滻Ϊ������
					Data->confirm = atoi(p1+1);//����ȷ����������
				}
			}			
		}
		//printf("USART2_RX_BUF:%s\r\n",USART2_RX_BUF);
		//printf("BUF:%s\r\n",BUF);
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"suspect")) != NULL)
		{printf("find suspect!\r\n");
			if((p1=strstr((const char*)p,":")) != NULL)//����:
				{	printf("find :!\r\n");
				if((p2=strstr((const char*)p1,","))!=NULL)//����,
				{
					*p2='\0';//,λ���滻Ϊ������
					Data->suspect = atoi(p1+1);//�������Ʋ�������		
					printf("Data->suspect:%d\r\n",Data->suspect);
				}
			}	
		}
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"dead")) != NULL)
		{printf("find dead!\r\n");
			if((p1=strstr((const char*)p,":")) != NULL)//����:
				{	printf("find :!\r\n");
				if((p2=strstr((const char*)p1,","))!=NULL)//����,
				{
					*p2='\0';//,λ���滻Ϊ������
					Data->dead = atoi(p1+1);//�������Ʋ�������		
					printf("Data->dead:%d\r\n",Data->dead);
				}
			}	
		}
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"heal")) != NULL)
		{printf("find heal!\r\n");
			if((p1=strstr((const char*)p,":")) != NULL)//����:
				{	printf("find :!\r\n");
				if((p2=strstr((const char*)p1,","))!=NULL)//����,
				{
					*p2='\0';//,λ���滻Ϊ������
					Data->heal = atoi(p1+1);//�������Ʋ�������		
					printf("Data->heal:%d\r\n",Data->heal);
				}
			}	
		}
		strcpy((char*)BUF,(char*)USART2_RX_BUF);
		if((p=strstr((const char*)BUF,"create_time")) != NULL)//���ҵ�ǰ����
		{printf("find create_time!\r\n");
			if((p1=strstr((const char*)(p+12),"\"")) != NULL)//����"
				{	printf("find \"!\r\n");
					if((p2=strstr((const char*)p1+1,"\""))!=NULL)//������һ��"
					{
						*p2='\0';//,λ���滻Ϊ������
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
	}else //δ�յ�����
	{
		#ifdef DEBUG_EN
			printf("recv failed!\r\n");
		#endif
			return;
	}
}
	
/*��LCD����ʾ��������*//*
void show_weather(u8 x,u8 y,u16 Wcolor,u16 backColor,u8 Code)
{
	switch(Code)
	{
		case 0:show_Hz32(x, y,"��", Wcolor, backColor);
			break;
		case 1:show_Hz32(x, y,"��", Wcolor, backColor);
			break;
		case 2:show_Hz32(x, y,"��", Wcolor, backColor);
			break;
		case 3:show_Hz32(x, y,"��", Wcolor, backColor);
			break;
		case 4:show_Hz32(x, y,"����", Wcolor, backColor);
			break;
		case 5:show_Hz32(x, y,"��ת����", Wcolor, backColor);
			break;		
		case 6:show_Hz32(x, y,"��ת����", Wcolor, backColor);
			break;	
		case 7:show_Hz32(x, y,"�󲿶���", Wcolor, backColor);
			break;	
		case 8:show_Hz32(x, y,"�󲿶���", Wcolor, backColor);
			break;	
		case 9:show_Hz32(x, y,"��", Wcolor, backColor);
			break;			
		case 10:show_Hz32(x, y,"����", Wcolor, backColor);
			break;	
		case 11:show_Hz32(x, y,"������", Wcolor, backColor);
			break;	
			
		case 13:show_Hz32(x, y,"С��", Wcolor, backColor);
			break;	
		case 14:show_Hz32(x, y,"����", Wcolor, backColor);
			break;	
		case 15:show_Hz32(x, y,"����", Wcolor, backColor);
			break;	
		case 16:show_Hz32(x, y,"����", Wcolor, backColor);
			break;	
		case 17:show_Hz32(x, y,"����", Wcolor, backColor);
			break;	
		case 18:show_Hz32(x, y,"�ش���", Wcolor, backColor);
				break;
		case 20:show_Hz32(x, y,"���ѩ", Wcolor, backColor);
			break;	
		case 21:show_Hz32(x, y,"��ѩ", Wcolor, backColor);
			break;	
		case 22:show_Hz32(x, y,"Сѩ", Wcolor, backColor);
				break;
		default: 
			break;
	}
}*/






