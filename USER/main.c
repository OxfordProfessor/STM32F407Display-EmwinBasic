﻿#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "sram.h"
#include "malloc.h"
#include "ILI93xx.h"
#include "led.h"
#include "timer.h"
#include "touch.h"
#include "FreeRTOS.h"
#include "ds18b20.h"
#include "task.h"
#include "limits.h"
#include "gui.h"
#include "GUI.h"
#include "GUIDemo.h"
#include "ff.h"
#include "exfuns.h"

//任务优先级
#define START_TASK_PRIO			1
//任务堆栈大小	
#define START_STK_SIZE 			256  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//TOUCH任务
//设置任务优先级
#define TOUCH_TASK_PRIO			2
//任务堆栈大小
#define TOUCH_STK_SIZE			128
//任务句柄
TaskHandle_t TouchTask_Handler;
//touch任务
void touch_task(void *pvParameters);

//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO 			3
//任务堆栈大小
#define LED0_STK_SIZE			128
//任务句柄
TaskHandle_t Led0Task_Handler;
//led0任务
void led0_task(void *pvParameters);

//EMWINDEMO任务
//设置任务优先级
#define EMWINDEMO_TASK_PRIO		4
//任务堆栈大小
#define EMWINDEMO_STK_SIZE		512
//任务句柄
TaskHandle_t EmwindemoTask_Handler;
//emwindemo_task任务
void emwindemo_task(void *pvParameters);

//FATFS   fs;			/* FatFs文件系统对象 */
//FIL     file;		/* file objects */
//UINT    bw;     /* File R/W count */
FRESULT result; 
//FILINFO fno;
//DIR dir;

int main(void)
{

	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 	//中断分组配置
	uart_init(115200);    	//串口波特率设置
	TFTLCD_Init();			//初始化LCD
	TP_Init();				//初始化触摸屏
	LED_Init();   			//LED初始化
	DS18B20_Init();			//温度传感器初始化
	FSMC_SRAM_Init(); 		//SRAM初始化	
	mem_init(SRAMIN); 		//内部RAM初始化
	mem_init(SRAMEX); 		//外部RAM初始化
	mem_init(SRAMCCM);		//CCM初始化
	
	exfuns_init();			//为fatfs文件系统分配内存
	result = f_mount(fs[0],"0:",1);	//挂载SD卡

//	while(!result)
//	{
//		
//	}
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄                
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC,ENABLE);//开启CRC时钟
	WM_SetCreateFlags(WM_CF_MEMDEV);
	GUI_Init();  					//STemWin初始化
	WM_MULTIBUF_Enable(1);  		//开启STemWin多缓冲,RGB屏可能会用到
    taskENTER_CRITICAL();           //进入临界区
	//创建触摸任务
    xTaskCreate((TaskFunction_t )touch_task,             
                (const char*    )"touch_task",           
                (uint16_t       )TOUCH_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TOUCH_TASK_PRIO,        
                (TaskHandle_t*  )&TouchTask_Handler);   	
    //创建LED0任务
    xTaskCreate((TaskFunction_t )led0_task,             
                (const char*    )"led0_task",           
                (uint16_t       )LED0_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LED0_TASK_PRIO,        
                (TaskHandle_t*  )&Led0Task_Handler);  
    //创建EMWIN Demo任务
    xTaskCreate((TaskFunction_t )emwindemo_task,             
                (const char*    )"emwindemo_task",           
                (uint16_t       )EMWINDEMO_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EMWINDEMO_TASK_PRIO,        
                (TaskHandle_t*  )&EmwindemoTask_Handler);   				
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//EMWINDEMO任务
void emwindemo_task(void *pvParameters)
{
	WM_MULTIBUF_Enable(1);
	while(1)
	{
		MainTask();
	}
}

//触摸任务的任务函数
void touch_task(void *pvParameters)
{
	while(1)
	{
		GUI_TOUCH_Exec();	
		vTaskDelay(5);		//延时5ms
	}
}

//LED0任务
void led0_task(void *p_arg)
{
	while(1)
	{
		LED0 = !LED0;
		vTaskDelay(500);		//延时500ms
	}
}
