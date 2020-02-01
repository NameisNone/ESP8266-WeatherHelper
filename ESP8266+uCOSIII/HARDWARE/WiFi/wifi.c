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

WeatherData *userData;
/*JSON��������*/
void cJSON_Parse_Uart(char *jsonBuf)
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
					sprintf(userData->text,"%s",cjson_Value->valuestring);//��������װ��ṹ���Ա
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
					userData->temp = (char)atoi(cjson_Value->valuestring);
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








