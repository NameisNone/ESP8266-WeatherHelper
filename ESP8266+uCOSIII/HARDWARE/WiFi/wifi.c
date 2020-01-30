#include "wifi.h"
#include "EspUart.h"
#include "delay.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"

/*����ATָ���*/
//0�����ͳɹ���1������ʧ��
u8 wifi_send_ATcmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//��������
	printf("send:%s\r\n",cmd);
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(wifi_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
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

/*������֪����������*/
//0���ɹ����ӽ���͸��ģʽ��1��ʧ��
u8 Connect_TCP_Server(u8 *ESP8266_Mode, u8 *ConnectMode, u8 *WiFi_SSID, u8 *WiFi_PSWD, u8 *ServerIP, u8 *Port)
{
	__align(8) u8 CWJAP[200]={0};
	__align(8) u8 CIPSTART[200]={0};
	sprintf((char*)CWJAP,"AT+CWJAP=\"%s\",\"%s\"",(u8*)WiFi_SSID,(u8*)WiFi_PSWD);//��ӡwifi���ƺ����뵽ָ����
	sprintf((char*)CIPSTART,"AT+CIPSTART=\"%s\",\"%s\",%s",ConnectMode,ServerIP,Port);//��ӡ��������Ϣ��CIPSTART��
	printf("CWJAP:%s\r\n",CWJAP);
	printf("CIPSTART:%s\r\n",CIPSTART);
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

