#include "main.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "ILI93xx.h"
#include "key.h"
#include "sram.h"
#include "malloc.h"
#include "touch.h"
#include "timer.h"
#include "arm_math.h"  
#include "adc.h"
//GUI支持
#include "GUI.h"
#include "FramewinDLG.h"
//
#include "hostPC.h"

/************************************************
fork from F4_HAL_emwin https://github.com/framist/STemWinForHAL
实现平台 ALIENTEK STM32F407ZGT6最小系统版
项目应用 计网实验1

版级接口：

printf 已重载为串口输出

耦合一时爽，重构火葬场！
************************************************/



int main(void)
{
    HAL_Init();                   	//初始化HAL库  
    
    Stm32_Clock_Init(336,8,2,7);  	//设置时钟,168Mhz
	delay_init(168);               	//初始化延时函数
	uart_init(9600);             	//初始化USART
    
    TIM3_Init(999,83); 	            //1KHZ 定时器3设置为1ms
    TIM4_Init(999,839);             //触摸屏扫描速度,100HZ.

	LED_Init();						//初始化LED	
	KEY_Init();						//初始化KEY
	TFTLCD_Init();           	    //初始化LCD FSMC接口
    TP_Init();				        //触摸屏初始化
    
	
	my_mem_init(SRAMIN);			//初始化内部内存池
	//my_mem_init(SRAMEX);			//不使用外部内存池
	my_mem_init(SRAMCCM);			//初始化CCM内存池
    
    __HAL_RCC_CRC_CLK_ENABLE();//使能CRC时钟，否则STemWin不能使用

	WM_SetCreateFlags(WM_CF_MEMDEV);//为重绘操作自动使用存储设备
	GUI_Init();
    // GUI_CURSOR_Show();


// 更换皮肤

#if 1
#include "DIALOG.h"
    BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX); 
    CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
    DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
    FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
    HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
    MENU_SetDefaultSkin(MENU_SKIN_FLEX);
    MULTIPAGE_SetDefaultSkin(MULTIPAGE_SKIN_FLEX);
    PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
    RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
    SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
    SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
    SPINBOX_SetDefaultSkin(SPINBOX_SKIN_FLEX);
#endif


    WM_HWIN CreatemainFramewin(void);
    CreatemainFramewin();
    
    mainLogPrint("\ninit OK!");
    mainLogPrint("\n === Selective Repeat ARQ experiment ===");
    //printf("init over\r\n");
    
    host_ARQ_R();
    while(1)
	{
		// GUI_Delay(100); 
        GUI_Exec();
        host_ARQ_R();
        LED0 = !LED0;
	} 
}

