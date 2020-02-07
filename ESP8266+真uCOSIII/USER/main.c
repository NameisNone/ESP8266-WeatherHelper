#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "includes.h"
#include "EspUart.h"
#include "wifi.h"
#include "key.h"
#include "malloc.h"

/*
const u8* GL="Guilin";
const u8* GG="Guigang";
const u8* GZ="Guangzhou";
*/

const u8* SSID="CMCC-Y3ff";
const u8* PSWD="9uas4tfq";
const u8* Seniverse="api.seniverse.com";//����������
const u8* Ihogu="myapi.ihogu.com";//���������
const u8* PORT="80";

//��֪����API
const u8 *Request=
"GET https://api.seniverse.com/v3/weather/now.json?key=Si4evEh1Az2pvdyB5&location=guigang&language=en&unit=c\r\n";

//��������API
const u8 *myapiRequest = 
"GET https://myapi.ihogu.com/public/?s=Whfy.count\r\n";

/************************************************/

//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		512
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define Parse_TASK_PRIO		4
//�����ջ��С	
#define Parse_STK_SIZE 		256
//������ƿ�
OS_TCB ParseTaskTCB;
//�����ջ	
CPU_STK Parse_TASK_STK[Parse_STK_SIZE];
//������
void parse_task(void *p_arg);

//�������ȼ�
#define FY_TASK_PRIO		5
//�����ջ��С	
#define FY_STK_SIZE 		256
//������ƿ�
OS_TCB FyTaskTCB;
//�����ջ	
CPU_STK Fy_TASK_STK[FY_STK_SIZE];
//������
void fy_task(void *p_arg);

//�������ȼ�
#define KEY_TASK_PRIO		6
//�����ջ��С	
#define KEY_STK_SIZE 		128
//������ƿ�
OS_TCB KEYTaskTCB;
//�����ջ	
CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
void key_task(void *p_arg);


//�ź���
OS_SEM TqSem;
OS_SEM FySem;

/*http����ʱ��*/
OS_TMR HTTP_timer;//���ڷ�������������ʱ��
void HTTP_timer_callback(void *p_tmr,void *p_arg);//�ص���������

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	uart_init(115200);    //���ڲ���������
	USART2_Init(115200);	//����2������115200
	LED_Init();         //LED��ʼ��
	KEY_Init();					//��ʼ������
	mem_init();				
	LCD_Init();       //��ʼ��LCDҺ��
	LCD_Display_Dir(1);//���ú���
	LCD_Clear(WHITE);
	POINT_COLOR=BLACK; 
	LCD_ShowString(75,5,216,20,24,"WiFi Helper");
	POINT_COLOR=DARKBLUE; 
	LCD_DrawLine(0,30,320,30);
	LCD_DrawLine(145,30,145,240);
	while(!WiFi_module_check())//���wifiģ���Ƿ����
	{
		POINT_COLOR=RED; 
		LCD_ShowString(68,100,200,30,24,"No WiFI Module!!");
		printf("δ��⵽wifiģ��!!!\r\n");
		LED0 = 0;
	}
	POINT_COLOR=BLUE;
	LCD_ShowString(20,140,250,8,16,"KEY-WKUP to Connect Internet!");
	LCD_ShowString(20,160,200,8,16,"KEY-0 to Start Service!");
	OSInit(&err);		//��ʼ��UCOSIII
	OS_CRITICAL_ENTER();//�����ٽ��������д��뱣��
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY)0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	 *)0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);  //����UCOSIII
	while(1);
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();//��ʼ��CPU
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	//����HTTP_timer�����ʱ��
  OSTmrCreate ((OS_TMR*)   &HTTP_timer,
							 (CPU_CHAR*) "httpTimer",//����
							 (OS_TICK)   100,//��ʼ��ʱ5ms*100=500ms
							 (OS_TICK)   1000,//������ʱ5ms*1000=5s
							 (OS_OPT )   OS_OPT_TMR_PERIODIC,//ѡ����������ģʽ
							 (OS_TMR_CALLBACK_PTR) HTTP_timer_callback,//��ʱ���ص�����
							 (void *)	0,//���ݸ��ص������Ĳ���
							 (OS_ERR *) &err);
	
	OS_CRITICAL_ENTER();	//�����ٽ���
	//���������ź����ź���
	OSSemCreate((OS_SEM*)   &TqSem,//�����ź���
							 (CPU_CHAR*) "Tq_sem",//�ź�������
							 (OS_SEM_CTR)0,//�ź�����ʼֵ
							 (OS_ERR*)    &err);
							 
	OSSemCreate((OS_SEM*)   &FySem,//��ʾ�ź���
						 (CPU_CHAR*) "Fy_Sem",//�ź�������
						 (OS_SEM_CTR)0,//�ź�����ʼֵ
						 (OS_ERR*)    &err);		
	//������������
	OSTaskCreate((OS_TCB 	* )&ParseTaskTCB,		
							(CPU_CHAR	* )"parse task", 		
                 (OS_TASK_PTR )parse_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )Parse_TASK_PRIO,    //�������ȼ�
                 (CPU_STK   * )&Parse_TASK_STK[0],	//�����ջ�׵�ַ
                 (CPU_STK_SIZE)Parse_STK_SIZE/10,	//ջ��
                 (CPU_STK_SIZE)Parse_STK_SIZE,	//��ջ��С	
                 (OS_MSG_QTY  )0,	//�ر���Ϣ����				
                 (OS_TICK	  )0,		//��ʹ��ʱ��Ƭ��ת����
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
		//���������������
		OSTaskCreate((OS_TCB 	* )&FyTaskTCB,		
							(CPU_CHAR	* )"Fy task", 		
                 (OS_TASK_PTR )fy_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )FY_TASK_PRIO,    //�������ȼ�
                 (CPU_STK   * )&Fy_TASK_STK[0],	//�����ջ�׵�ַ
                 (CPU_STK_SIZE)FY_STK_SIZE/10,	//ջ��
                 (CPU_STK_SIZE)FY_STK_SIZE,	//��ջ��С	
                 (OS_MSG_QTY  )0,	//�ر���Ϣ����				
                 (OS_TICK	  )0,		//��ʹ��ʱ��Ƭ��ת����
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);			
	
	//������������
	OSTaskCreate((OS_TCB 	* )&KEYTaskTCB,		
				 (CPU_CHAR	* )"key task", 		
                 (OS_TASK_PTR )key_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )KEY_TASK_PRIO,    //�������ȼ�
                 (CPU_STK   * )&KEY_TASK_STK[0],	//�����ջ�׵�ַ
                 (CPU_STK_SIZE)KEY_STK_SIZE/10,	//ջ��
                 (CPU_STK_SIZE)KEY_STK_SIZE,	//��ջ��С	
                 (OS_MSG_QTY  )0,	//�ر���Ϣ����				
                 (OS_TICK	  )0,		//��ʹ��ʱ��Ƭ��ת����
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 		 
	OS_CRITICAL_EXIT();	//�����ٽ���
	OSTaskDel((OS_TCB*)0,&err);//ɾ��������
}

//���ݽ�������
void parse_task(void *p_arg)
{
	OS_ERR err;
	u8 *Response = mymalloc(500);
	WeatherData UserData;
	while(1)
	{
		printf("�����ź���TqSem\r\n");
		OSSemPend(&TqSem,0,OS_OPT_PEND_BLOCKING,0,&err);//�����ź���
		printf("�����ź���TqSem�ɹ�\r\n");	
		UserData.city=mymalloc(50);
		UserData.text=mymalloc(50);
		if(!Connect_NewServer((u8*)"api.seniverse.com"))//����api.seniverse.com�������ɹ�
		{
			get_http((u8*)Request,Response);
			cJSON_Parse_Weather((char*)Response,&UserData);
		}
		printf("****************************************��������:\r\n");
		printf("UserData.city:%s,\r\n",UserData.city);
		printf("UserData.text:%s,\r\n",UserData.text);
		printf("UserData.code:%d,\r\n",UserData.code);
		printf("UserData.temp:%d,\r\n",UserData.temp);	
		POINT_COLOR = BLACK;
		LCD_ShowString(55,40,60,12,16,UserData.city);//����
		LCD_ShowNum(55,60,UserData.temp,3,16);//��ʾ��ǰ����		
		LCD_ShowString(55,80,80,12,16,UserData.text);//��������
		myfree(Response);//�ͷ��ڴ�
		myfree(UserData.city);
		myfree(UserData.text);
		
		LED1 = ~LED1;
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
	}
}

void fy_task(void *p_arg)
{
	OS_ERR err;
	FY_StructDef FY_Data;
	p_arg = p_arg;
	while(1)
	{
		printf("�����ź���FySem\r\n");
		OSSemPend(&FySem,0,OS_OPT_PEND_BLOCKING,0,&err);//�����ź���
		printf("�����ź���FySem�ɹ�\r\n");	
		if(!Connect_NewServer((u8*)"myapi.ihogu.com"))//����myapi.ihogu.com�������ɹ�
		Get_myApi_Http((u8*)myapiRequest,&FY_Data);//��ȡ������������
		printf("****************************************��������:\r\n");
		printf("FY_Data.confirm:%d\r\n",FY_Data.confirm);
		printf("FY_Data.suspect:%d\r\n",FY_Data.suspect);
		printf("FY_Data.dead:%d\r\n",FY_Data.dead);
		printf("FY_Data.heal:%d\r\n",FY_Data.heal);
		printf("FY_Data.date:%s\r\n",FY_Data.date);
		
		POINT_COLOR = RED;
		LCD_ShowNum(224,60,FY_Data.confirm,5,16);
		LCD_ShowNum(224,80,FY_Data.suspect,5,16);
		LCD_ShowNum(200,100,FY_Data.dead,4,16);
		LCD_ShowNum(200,120,FY_Data.heal,5,16);
		POINT_COLOR = BLUE;
		LCD_ShowString(125,210,120,12,24,FY_Data.date);
		
		LED1 = ~LED1;
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
	}
}

u8 TimerCount=0;//��ʱ������ֵ
//key������
void key_task(void *p_arg)
{
	u8 key=0,num,keyFlag=0;
	u8 *ipAddr = NULL;
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		key = KEY_Scan(0);//����ɨ��
		switch(key)
		{
			case KEY0_PRES:keyFlag=!keyFlag;
				if(keyFlag)
				OSTmrStart(&HTTP_timer,&err);//������ʱ��
				else
				{
					OSTmrStop(&HTTP_timer,OS_OPT_TMR_NONE,0,&err);//�رն�ʱ��
					TimerCount = 0;
				}
				break;
			case KEY1_PRES:
					LED1 = !LED1;
				break;
			case WKUP_PRES:
				ipAddr = mymalloc(50);
				if(!Connect_TCP_Server(ipAddr,TCP,(u8*)SSID,(u8*)PSWD,(u8*)Ihogu,(u8*)PORT,1))//���ӷ���������͸��ģʽ
				{
					LED1 = 0;
					printf("IP��ַ��%s\r\n",ipAddr);
					POINT_COLOR = BLUE;
					LCD_ShowString(160,40,24,8,16,"IP:");//��ʾ��ǰIP��ַ
					LCD_ShowString(184,40,100,8,16,ipAddr);
					myfree(ipAddr);
					
					LCD_ShowString(10,40,60,12,16,"CITY:");
					LCD_ShowString(10,60,60,12,16,"Temp:");
					LCD_ShowString(90,60,60,12,16,"^C");//��ʾ��ǰ����
					LCD_ShowString(10,80,60,12,16,"Text:");
					
					LCD_ShowString(160,60,64,8,16,"confirm:");
					LCD_ShowString(160,80,64,8,16,"suspect:");
					LCD_ShowString(160,100,40,8,16,"dead:");
					LCD_ShowString(160,120,40,8,16,"heal:");
					LCD_ShowString(65,210,60,12,24,"Date:");
				}else printf("wifi����ʧ��\r\n");
				break;
			default:key=0;
				break;
		}
		num++;
		if(num==50)
		{
			num=0;
			LED0 = ~LED0;
		}
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ20ms
	}
}

u8 switchFlag=0;
//��ʱ���ص�����
void HTTP_timer_callback(void *p_tmr,void *p_arg)
{
	OS_ERR err;
	if(switchFlag)//���ݱ�־λ������Ӧ���ź���
	{
		OSSemPost((OS_SEM*)&TqSem,
						(OS_OPT )OS_OPT_POST_1,//ֻ����������ǰ�ȴ����ź��������ȼ���ߵ�����
						(OS_ERR*)&err);
	}else{
		OSSemPost((OS_SEM*)&FySem,
					(OS_OPT )OS_OPT_POST_1,//ֻ����������ǰ�ȴ����ź��������ȼ���ߵ�����
					(OS_ERR*)&err);
	}
	switchFlag = !switchFlag;
	LCD_ShowString(20,190,40,6,16,"Time:");//��ʾ��ʱ�����д���
	LCD_ShowxNum(60,190,TimerCount+1,3,16,0);
	TimerCount++;
}
