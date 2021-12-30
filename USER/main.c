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
//GUI֧��
#include "GUI.h"
#include "FramewinDLG.h"
//
#include "hostPC.h"

/************************************************
fork from F4_HAL_emwin https://github.com/framist/STemWinForHAL
ʵ��ƽ̨ ALIENTEK STM32F407ZGT6��Сϵͳ��
��ĿӦ�� ����ʵ��1

�漶�ӿڣ�

printf ������Ϊ�������

���һʱˬ���ع����᳡��
************************************************/



int main(void)
{
    HAL_Init();                   	//��ʼ��HAL��  
    
    Stm32_Clock_Init(336,8,2,7);  	//����ʱ��,168Mhz
	delay_init(168);               	//��ʼ����ʱ����
	uart_init(9600);             	//��ʼ��USART
    
    TIM3_Init(999,83); 	            //1KHZ ��ʱ��3����Ϊ1ms
    TIM4_Init(999,839);             //������ɨ���ٶ�,100HZ.

	LED_Init();						//��ʼ��LED	
	KEY_Init();						//��ʼ��KEY
	TFTLCD_Init();           	    //��ʼ��LCD FSMC�ӿ�
    TP_Init();				        //��������ʼ��
    
	
	my_mem_init(SRAMIN);			//��ʼ���ڲ��ڴ��
	//my_mem_init(SRAMEX);			//��ʹ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);			//��ʼ��CCM�ڴ��
    
    __HAL_RCC_CRC_CLK_ENABLE();//ʹ��CRCʱ�ӣ�����STemWin����ʹ��

	WM_SetCreateFlags(WM_CF_MEMDEV);//Ϊ�ػ�����Զ�ʹ�ô洢�豸
	GUI_Init();
    // GUI_CURSOR_Show();


// ����Ƥ��

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

