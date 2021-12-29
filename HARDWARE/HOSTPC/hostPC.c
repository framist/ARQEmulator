/**
 * @file hostPC.c
 * @author framist (framist@163.com)
 * @brief ������λ��
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


// ----------------  ѡ�����ش�ARQ ���շ��㷨 ---------------- 

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
        int len = USART_RX_STA & 0x3fff; //�õ��˴ν��յ������ݳ���
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
    //��ʵ��һ����򵥵ģ��ַ������һ���ֽ�������У�飨������У�����ţ�
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

//ʮ�����Ƶ���� m=4 ȡ��һλ
static int _seqNo(char * frame) {
    int i;
    if('0' <= frame[0] && frame[0] <= '9'){
        i = (int)((frame[0] - '0')); //ʮ�����Ƶ����
    } else {
        i = (int)((frame[0] - 'A')); //ʮ�����Ƶ����
    }
    return i;
}

//��ӡĿǰ״̬
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

    char frame[USART_REC_LEN]; // [���к�][����֡*n][У���ֶ�][\r\n->\0] �ɼ��ַ��ռ�
    static struct WINDOW window[WINDOW_SIZE] = {NULL,FALSE};

    // marked all solt = false
    // while(1) { ����while��Ϊ�ⲿ����ѭ��
    
    if(_WaitForEvent(frame)) {         // ����֡�����¼�
        if(_corrupeted(frame) ) { // ������ϵ�ʾ���߼�ò��Ҳ�Ǵ��
            if(! NakSent) _SendNAK(R_n);
            NakSent = TRUE;
            mainLogPrintf("\n _corrupeted frame!");
            return 0;//sleep
        }
        if(_seqNo(frame) != R_n && (! NakSent) ){
            _SendNAK(R_n);
            NakSent = TRUE;
        } // ���ϵ�α������û�����������
        if(TRUE &&(! window[_seqNo(frame)].f)){ //�϶�in Window ��Ϊȡ���Ĵ���
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
