/**
 * @file hostPC.c
 * @author framist (framist@163.com)
 * @brief 连接上位机
 * @version 0.1
 * @date 2021-10-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "FramewinDLG.h"
#include "hostPC.h"
#include "usart.h"
#include "string.h"
//#include <stdio.h>


// ----------------  选择性重传ARQ 接收方算法 ---------------- 

typedef uint8_t bool;
#define TRUE 1
#define FALSE 0
#define WINDOW_SIZE 16/2

struct WINDOW {
    char data[200];
    char * s;
    bool f;
};


static bool _WaitForEvent(char * frame) {
    if(USART_RX_STA&0x8000) {
        int len = USART_RX_STA & 0x3fff; //得到此次接收到的数据长度
        USART_RX_STA = 0;

        USART_RX_BUF[len] = '\0';
        mainLogPrintf("\nRAW receive: %s len=%d",USART_RX_BUF,len);

        memcpy(frame,USART_RX_BUF,len+1);
        
        return TRUE;
    } else {
        return FALSE;
    }
        
}


static bool _corrupeted(char * frame) {
    //先实现一个最简单的：字符串最后一个字节做长度校验（不包括校验和序号）
    int len = strlen(frame);
    if((int)(frame[len-1] - '0' ) != len-2) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void _SendNAK(int R_n){
    printf("NAK: %01X\r\n",R_n);
}
static void _SendAck(int R_n){
    printf("ACK: %01X\r\n",R_n);
}

//十六进制的序号 m=4 取第一位
static int _seqNo(char * frame) {
    int i;
    if('0' <= frame[0] && frame[0] <= '9'){
        i = (int)((frame[0] - '0')); //十六进制的序号
    } else {
        i = (int)((frame[0] - 'A')); //十六进制的序号
    }
    return i;
}

//打印目前状态
static void _printWindow(struct WINDOW * window) {
    int i;
    mainLogPrintf("\nW: ");
    for(i=0;i<WINDOW_SIZE;i++) {
        mainLogPrintf("%d:",i);
        if(window[i].f) {
            mainLogPrintf("%s",window[i].s);
        } else {
            mainLogPrintf("");
        }
        mainLogPrintf("| ");
    }
    return;
}


int host_ARQ_R(void) {
    static int R_n = 0;
    static bool NakSent = FALSE;
    static bool AckNeeded = FALSE;

    char frame[USART_REC_LEN]; // [序列号][数据帧*n][校验字段][\r\n->\0] 可见字符空间
    static struct WINDOW window[WINDOW_SIZE] = {NULL,FALSE};

    // marked all solt = false
    // while(1) { 不用while因为外部有主循环
    
    if(_WaitForEvent(frame)) {         // 数据帧到达事件
        if(_corrupeted(frame) ) { // 这边书上的示例逻辑貌似也是错的
            if(! NakSent) _SendNAK(R_n);
            NakSent = TRUE;
            mainLogPrintf("\n _corrupeted frame!");
            return 0;//sleep
        }
        if(_seqNo(frame) != R_n && (! NakSent) ){
            _SendNAK(R_n);
            NakSent = TRUE;
        } // 书上的伪代码中没有这个反括号
        if(TRUE &&(! window[_seqNo(frame)].f)){ //肯定in Window 因为取最大的窗口
            window[_seqNo(frame)].f = TRUE;
            // window[_seqNo(frame)].s = frame + 1;
            window[_seqNo(frame)].s = window[_seqNo(frame)].data;
            memcpy(window[_seqNo(frame)].s,frame+1,strlen(frame)-1);
            
            mainLogPrintf("\nget:%d: %s", _seqNo(frame),window[_seqNo(frame)].s);
        
            while (window[R_n].f) {
                mainLogPrintf("\n[+] DeliverData:%.*s",strlen(window[R_n].s)-1, window[R_n].s);
                //purge Rn
                window[R_n].s = NULL;    
                window[R_n].f = FALSE;
                R_n ++; 
                R_n %= WINDOW_SIZE;
                AckNeeded = TRUE;
            }
            if(AckNeeded){
                _SendAck(R_n);
                AckNeeded = FALSE;
                NakSent = FALSE;
            }
            _printWindow(window);
        }
        
    }
    // }
    
    return 0;
}




/*******************************END OF FILE************************************/
