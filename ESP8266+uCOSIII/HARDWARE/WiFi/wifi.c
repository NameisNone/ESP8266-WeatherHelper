#include "wifi.h"
#include "EspUart.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"
#include "cJSON.h"

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
	if(!wifi_send_ATcmd("AT","OK",30))//����AT���ģ���Ƿ�Ӧ��
		status = 1;
	else
		status = 0;
	return status;
}

/*���ӷ�����*/
//0���ɹ����Ӳ�����͸��ģʽ��1��ʧ��
u8 Connect_TCP_Server(u8 *ESP8266_Mode, u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port)
{
	__align(8) u8 CWJAP[100]={0};
	__align(8) u8 CIPSTART[100]={0};
	sprintf((char*)CWJAP,"AT+CWJAP=\"%s\",\"%s\"",(u8*)WiFi_SSID,(u8*)WiFi_PSWD);//��ӡwifi���ƺ����뵽ָ����
	sprintf((char*)CIPSTART,"AT+CIPSTART=\"%s\",\"%s\",%s",ConnectMode,ServerIP,Port);//��ӡ��������Ϣ��CIPSTART��
	#ifdef DEBUG_EN
	printf("CWJAP:%s\r\n",CWJAP);
	printf("CIPSTART:%s\r\n",CIPSTART);
	#endif
	if(!wifi_send_ATcmd(ESP8266_Mode,OK,20))//1.��������STAģʽ
	{
	  #ifdef DEBUG_EN
				printf("STAģʽ���óɹ���\r\n");
		#endif
		while(wifi_send_ATcmd((u8*)CWJAP,"OK",300))//2.��������wifi�ȵ�
		{
			#ifdef DEBUG_EN
				printf("**********************\r\n");
				printf("����wifi�ɹ���\r\n");
				printf("��ǰwifi�ȵ�Ϊ��");
				printf("SSID:%s\r\n",WiFi_SSID);
				printf("PSWD:%s\r\n",WiFi_PSWD);
				printf("**********************\r\n");
			#endif
			if(!wifi_send_ATcmd((u8*)"AT+CIPMUX=0",OK,300))//3.���õ�����
			{
				#ifdef DEBUG_EN
				printf("���õ����ӳɹ�!\r\n");				
				#endif
				if(!wifi_send_ATcmd((u8*)CIPSTART,OK,300))//4.���ӷ�����
				{
					#ifdef DEBUG_EN
					printf("����%s�ɹ�!\r\n",ServerIP);				
					#endif
					if(!wifi_send_ATcmd((u8*)"AT+CIPMODE=1",OK,20))//5.����͸��ģʽ
					{
						#ifdef DEBUG_EN
						printf("����͸��ģʽ!\r\n");				
						#endif
						if(!wifi_send_ATcmd((u8*)"AT+CIPSEND",OK,20))
						{
							#ifdef DEBUG_EN
							printf("����͸��ģʽ!\r\n");				
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


/*JSON��������*/
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
	
	cjson_main = cJSON_Parse(jsonBuf);//����json����
	if(cjson_main==NULL)
	{
		#ifdef DEBUG_EN
		printf("cjson����ʧ�ܣ�\r\n");
		#endif
		return ;
	}
	
	if((cjson_results = cJSON_GetObjectItem(cjson_main,"results")) != NULL)//����results����ɹ�
	{
		int Arraysize = cJSON_GetArraySize(cjson_results);//��ȡresults�����Ա��
		
		/*����location*/
		if((cjson_resultsArray = cJSON_GetArrayItem(cjson_results,0)) != NULL)//������1����Աlocation
		{
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"name"))!=NULL)//���������Աlocation��name�ļ�ֵ
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"name\":%s,\r\n",cjson_keyvalue->valuestring);//��ӡname��ֵ
				#endif
				cJSON_Delete(cjson_resultsArray);//�ͷ��ڴ�
			}
		}
		
		/*����now*/
		if((cjson_resultsArray = cJSON_GetArrayItem(cjson_results,1)) != NULL)//������2����Աnow
		{
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"text"))!=NULL)//����text��ֵ
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"text\":%s,\r\n",cjson_keyvalue->valuestring);//��ӡname��ֵ
				#endif
				cJSON_Delete(cjson_keyvalue);//�ͷ��ڴ�
			}
			
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"code"))!=NULL)//����code��ֵ
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"code\":%s,\r\n",cjson_keyvalue->valuestring);//��ӡname��ֵ
				#endif
				cJSON_Delete(cjson_keyvalue);//�ͷ��ڴ�
			}
			
			if((cjson_keyvalue = cJSON_GetObjectItem(cjson_resultsArray,"temperature"))!=NULL)//����text��ֵ
			{
				//sprintf("");
				#ifdef DEBUG_EN
				printf("\"temperature\":%s\r\n",cjson_keyvalue->valuestring);//��ӡname��ֵ
				#endif
				cJSON_Delete(cjson_keyvalue);//�ͷ��ڴ�
			}
			cJSON_Delete(cjson_resultsArray);//�ͷ��ڴ�
		}
			
		/*����lastupdate*/
	}
}


/*
 *����http���󣬽������յ�������
 *ʹ�ô˺���ǰ���������ӷ��������ҽ���͸��ģʽ 
 */
void get_http(u8* request)
{
	__align(8) u8 response[300]={0};//mark�� response������
	u8 *p = NULL;
	u8 *p1 = NULL;
	u8 *p2 = NULL;
	if(atk_8266_send_data(request,0,600))//�������ʧ��
	{
		response[0]=0;
		printf("����ʧ��1!\r\n");
		return;
	}
	//����ɹ�
	delay_ms(1000);
	if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
	{
		if((p = wifi_check_cmd("{")) != NULL)//�����յ������Ƿ���ȷ�����Ƿ�Ϊ { ��ͷ��
		{
			printf("����ʧ��2!\r\n");
			p1=(u8*)strstr((const char*)(p),"]");//�Ȳ��� ] ����
			printf("p1=(u8*)strstr((const char*)(p),\"]\");\r\n");
			p2=(u8*)strstr((const char*)(p1+1),"}");//���� } ���ţ����json���ݵĽ�β
			printf("p1=(u8*)strstr((const char*)(p1+1),\"}\");\r\n");
			*(p2+1)=0;//���������
			
			printf("die here!\r\n");
			sprintf((char*)response,"%s",p);	//��ӡjson����response��
			printf("I am not die!\r\n");
			#ifdef DEBUG_EN
			printf("���յ�������:%s\r\n",response);//��ӡ����
			#endif
		}
	}
	else 
	{
		printf("����ʧ��3!\r\n");
		return ;
	}
}
