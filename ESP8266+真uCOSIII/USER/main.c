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
const u8* Seniverse="api.seniverse.com";//天气服务器
const u8* Ihogu="myapi.ihogu.com";//疫情服务器
const u8* PORT="80";

//心知天气API
const u8 *Request=
"GET https://api.seniverse.com/v3/weather/now.json?key=Si4evEh1Az2pvdyB5&location=guigang&language=en&unit=c\r\n";

//肺炎疫情API
const u8 *myapiRequest = 
"GET https://myapi.ihogu.com/public/?s=Whfy.count\r\n";

/************************************************/

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define Parse_TASK_PRIO		4
//任务堆栈大小	
#define Parse_STK_SIZE 		256
//任务控制块
OS_TCB ParseTaskTCB;
//任务堆栈	
CPU_STK Parse_TASK_STK[Parse_STK_SIZE];
//任务函数
void parse_task(void *p_arg);

//任务优先级
#define FY_TASK_PRIO		5
//任务堆栈大小	
#define FY_STK_SIZE 		256
//任务控制块
OS_TCB FyTaskTCB;
//任务堆栈	
CPU_STK Fy_TASK_STK[FY_STK_SIZE];
//任务函数
void fy_task(void *p_arg);

//任务优先级
#define KEY_TASK_PRIO		6
//任务堆栈大小	
#define KEY_STK_SIZE 		128
//任务控制块
OS_TCB KEYTaskTCB;
//任务堆栈	
CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
void key_task(void *p_arg);


//信号量
OS_SEM TqSem;
OS_SEM FySem;

/*http请求定时器*/
OS_TMR HTTP_timer;//用于发送请求的软件定时器
void HTTP_timer_callback(void *p_tmr,void *p_arg);//回调函数声明

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	uart_init(115200);    //串口波特率设置
	USART2_Init(115200);	//串口2波特率115200
	LED_Init();         //LED初始化
	KEY_Init();					//初始化按键
	mem_init();				
	LCD_Init();       //初始化LCD液晶
	LCD_Display_Dir(1);//设置横屏
	LCD_Clear(WHITE);
	POINT_COLOR=BLACK; 
	LCD_ShowString(75,5,216,20,24,"WiFi Helper");
	POINT_COLOR=DARKBLUE; 
	LCD_DrawLine(0,30,320,30);
	LCD_DrawLine(145,30,145,240);
	while(!WiFi_module_check())//检查wifi模块是否存在
	{
		POINT_COLOR=RED; 
		LCD_ShowString(68,100,200,30,24,"No WiFI Module!!");
		printf("未检测到wifi模块!!!\r\n");
		LED0 = 0;
	}
	POINT_COLOR=BLUE;
	LCD_ShowString(20,140,250,8,16,"KEY-WKUP to Connect Internet!");
	LCD_ShowString(20,160,200,8,16,"KEY-0 to Start Service!");
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区，进行代码保护
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY)0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	 *)0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();//初始化CPU
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	//创建HTTP_timer软件定时器
  OSTmrCreate ((OS_TMR*)   &HTTP_timer,
							 (CPU_CHAR*) "httpTimer",//名称
							 (OS_TICK)   100,//初始延时5ms*100=500ms
							 (OS_TICK)   1000,//周期延时5ms*1000=5s
							 (OS_OPT )   OS_OPT_TMR_PERIODIC,//选项设置周期模式
							 (OS_TMR_CALLBACK_PTR) HTTP_timer_callback,//定时器回调函数
							 (void *)	0,//传递给回调函数的参数
							 (OS_ERR *) &err);
	
	OS_CRITICAL_ENTER();	//进入临界区
	//创建按键信号量信号量
	OSSemCreate((OS_SEM*)   &TqSem,//按键信号量
							 (CPU_CHAR*) "Tq_sem",//信号量名称
							 (OS_SEM_CTR)0,//信号量初始值
							 (OS_ERR*)    &err);
							 
	OSSemCreate((OS_SEM*)   &FySem,//显示信号量
						 (CPU_CHAR*) "Fy_Sem",//信号量名称
						 (OS_SEM_CTR)0,//信号量初始值
						 (OS_ERR*)    &err);		
	//创建解析任务
	OSTaskCreate((OS_TCB 	* )&ParseTaskTCB,		
							(CPU_CHAR	* )"parse task", 		
                 (OS_TASK_PTR )parse_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )Parse_TASK_PRIO,    //任务优先级
                 (CPU_STK   * )&Parse_TASK_STK[0],	//任务堆栈首地址
                 (CPU_STK_SIZE)Parse_STK_SIZE/10,	//栈深
                 (CPU_STK_SIZE)Parse_STK_SIZE,	//堆栈大小	
                 (OS_MSG_QTY  )0,	//关闭消息队列				
                 (OS_TICK	  )0,		//不使用时间片轮转调度
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
		//创建疫情相关任务
		OSTaskCreate((OS_TCB 	* )&FyTaskTCB,		
							(CPU_CHAR	* )"Fy task", 		
                 (OS_TASK_PTR )fy_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )FY_TASK_PRIO,    //任务优先级
                 (CPU_STK   * )&Fy_TASK_STK[0],	//任务堆栈首地址
                 (CPU_STK_SIZE)FY_STK_SIZE/10,	//栈深
                 (CPU_STK_SIZE)FY_STK_SIZE,	//堆栈大小	
                 (OS_MSG_QTY  )0,	//关闭消息队列				
                 (OS_TICK	  )0,		//不使用时间片轮转调度
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);			
	
	//创建按键任务
	OSTaskCreate((OS_TCB 	* )&KEYTaskTCB,		
				 (CPU_CHAR	* )"key task", 		
                 (OS_TASK_PTR )key_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )KEY_TASK_PRIO,    //任务优先级
                 (CPU_STK   * )&KEY_TASK_STK[0],	//任务堆栈首地址
                 (CPU_STK_SIZE)KEY_STK_SIZE/10,	//栈深
                 (CPU_STK_SIZE)KEY_STK_SIZE,	//堆栈大小	
                 (OS_MSG_QTY  )0,	//关闭消息队列				
                 (OS_TICK	  )0,		//不使用时间片轮转调度
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 		 
	OS_CRITICAL_EXIT();	//进入临界区
	OSTaskDel((OS_TCB*)0,&err);//删除本任务
}

//数据解析任务
void parse_task(void *p_arg)
{
	OS_ERR err;
	u8 *Response = mymalloc(500);
	WeatherData UserData;
	while(1)
	{
		printf("请求信号量TqSem\r\n");
		OSSemPend(&TqSem,0,OS_OPT_PEND_BLOCKING,0,&err);//请求信号量
		printf("请求信号量TqSem成功\r\n");	
		UserData.city=mymalloc(50);
		UserData.text=mymalloc(50);
		if(!Connect_NewServer((u8*)"api.seniverse.com"))//连接api.seniverse.com服务器成功
		{
			get_http((u8*)Request,Response);
			cJSON_Parse_Weather((char*)Response,&UserData);
		}
		printf("****************************************数据如下:\r\n");
		printf("UserData.city:%s,\r\n",UserData.city);
		printf("UserData.text:%s,\r\n",UserData.text);
		printf("UserData.code:%d,\r\n",UserData.code);
		printf("UserData.temp:%d,\r\n",UserData.temp);	
		POINT_COLOR = BLACK;
		LCD_ShowString(55,40,60,12,16,UserData.city);//城市
		LCD_ShowNum(55,60,UserData.temp,3,16);//显示当前气温		
		LCD_ShowString(55,80,80,12,16,UserData.text);//天气现象
		myfree(Response);//释放内存
		myfree(UserData.city);
		myfree(UserData.text);
		
		LED1 = ~LED1;
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
}

void fy_task(void *p_arg)
{
	OS_ERR err;
	FY_StructDef FY_Data;
	p_arg = p_arg;
	while(1)
	{
		printf("请求信号量FySem\r\n");
		OSSemPend(&FySem,0,OS_OPT_PEND_BLOCKING,0,&err);//请求信号量
		printf("请求信号量FySem成功\r\n");	
		if(!Connect_NewServer((u8*)"myapi.ihogu.com"))//连接myapi.ihogu.com服务器成功
		Get_myApi_Http((u8*)myapiRequest,&FY_Data);//获取肺炎疫情数据
		printf("****************************************数据如下:\r\n");
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
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
}

u8 TimerCount=0;//定时器计数值
//key任务函数
void key_task(void *p_arg)
{
	u8 key=0,num,keyFlag=0;
	u8 *ipAddr = NULL;
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		key = KEY_Scan(0);//按键扫描
		switch(key)
		{
			case KEY0_PRES:keyFlag=!keyFlag;
				if(keyFlag)
				OSTmrStart(&HTTP_timer,&err);//启动定时器
				else
				{
					OSTmrStop(&HTTP_timer,OS_OPT_TMR_NONE,0,&err);//关闭定时器
					TimerCount = 0;
				}
				break;
			case KEY1_PRES:
					LED1 = !LED1;
				break;
			case WKUP_PRES:
				ipAddr = mymalloc(50);
				if(!Connect_TCP_Server(ipAddr,TCP,(u8*)SSID,(u8*)PSWD,(u8*)Ihogu,(u8*)PORT,1))//连接服务器开启透传模式
				{
					LED1 = 0;
					printf("IP地址：%s\r\n",ipAddr);
					POINT_COLOR = BLUE;
					LCD_ShowString(160,40,24,8,16,"IP:");//显示当前IP地址
					LCD_ShowString(184,40,100,8,16,ipAddr);
					myfree(ipAddr);
					
					LCD_ShowString(10,40,60,12,16,"CITY:");
					LCD_ShowString(10,60,60,12,16,"Temp:");
					LCD_ShowString(90,60,60,12,16,"^C");//显示当前气温
					LCD_ShowString(10,80,60,12,16,"Text:");
					
					LCD_ShowString(160,60,64,8,16,"confirm:");
					LCD_ShowString(160,80,64,8,16,"suspect:");
					LCD_ShowString(160,100,40,8,16,"dead:");
					LCD_ShowString(160,120,40,8,16,"heal:");
					LCD_ShowString(65,210,60,12,24,"Date:");
				}else printf("wifi连接失败\r\n");
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
		OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时20ms
	}
}

u8 switchFlag=0;
//定时器回调函数
void HTTP_timer_callback(void *p_tmr,void *p_arg)
{
	OS_ERR err;
	if(switchFlag)//根据标志位发布对应的信号量
	{
		OSSemPost((OS_SEM*)&TqSem,
						(OS_OPT )OS_OPT_POST_1,//只发布给给当前等待此信号量中优先级最高的任务
						(OS_ERR*)&err);
	}else{
		OSSemPost((OS_SEM*)&FySem,
					(OS_OPT )OS_OPT_POST_1,//只发布给给当前等待此信号量中优先级最高的任务
					(OS_ERR*)&err);
	}
	switchFlag = !switchFlag;
	LCD_ShowString(20,190,40,6,16,"Time:");//显示定时器运行次数
	LCD_ShowxNum(60,190,TimerCount+1,3,16,0);
	TimerCount++;
}
