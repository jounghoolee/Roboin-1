/*
 * youruart.c
 *
 *  Created on: Jan 6, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�. 
 */

#include <stdio.h>
#include <stdarg.h>
#include "youruart.h"
#include "ngv_dmu.h"
#include "rappid_utils.h"


void (*yuart_pSCmdCallback[YUART_SCMD_NUM])(char *);
void (*yuart_pCCmdCallback[YUART_CCMD_NUM])(void);


#define YUART_RX_SCMD 0x1
#define YUART_RX_HCMD 0x2


vint8_t yuart_RxFlag;
volatile char yuart_RxCCmd;
volatile char  yuart_RxSCmd[YUART_SCMD_BUF_SIZE];
vint16_t yuart_RxSCmdPtr;
vint16_t yuart_TxQueHeadPtr, yuart_TxQueTailPtr;
volatile char  yuart_TxQue[YUART_TX_QUE_SIZE];


//- UART RX2 ISR
void YUART_RX2_ISR (void)
{
	char rxd;
	
	rxd = (char)LINFLEX_2.BDRM.R;

	//- Capital Command: �� ���� ��ɾ�
	if ((rxd>=YUART_CCMD_START) && (rxd<=YUART_CCMD_END)) {
		if (!(yuart_RxFlag & YUART_RX_HCMD)) {
			yuart_RxCCmd = rxd;
			yuart_RxFlag |= YUART_RX_HCMD;
		}
	}
	// Small Command: �� �� ��ɾ�
	else {
		if (!(yuart_RxFlag & YUART_RX_SCMD)){
			if (rxd==YUART_ASCII_CR) {
				yuart_RxSCmd[yuart_RxSCmdPtr] = '\0';
				yuart_RxFlag |= YUART_RX_SCMD;
				yuart_RxSCmdPtr++;
			} else if (rxd==YUART_ASCII_BS) {
				if (yuart_RxSCmdPtr>0)
					yuart_RxSCmdPtr--;
			} else {
				yuart_RxSCmd[yuart_RxSCmdPtr] = rxd;
				if (yuart_RxSCmdPtr < (YUART_SCMD_BUF_SIZE-1)) {
					yuart_RxSCmdPtr++;
				}
			}
		}
	}
	
    LINFLEX_2.LINSR.R = 0xF23F;
    LINFLEX_2.UARTSR.R = 0x4;	// DRF
}


//- UART TX2 ISR
void YUART_TX2_ISR (void)
{
    LINFLEX_2.LINSR.R = 0xF23F;
    LINFLEX_2.UARTSR.R = 0x2;	// DTF

    if (yuart_TxQueHeadPtr!=yuart_TxQueTailPtr) {
		LINFLEX_2.BDRL.R = yuart_TxQue[yuart_TxQueTailPtr];
		yuart_TxQueTailPtr = (yuart_TxQueTailPtr+1)%YUART_TX_QUE_SIZE;
	}
	else {
		LINFLEX_2.LINIER.B.DTIE = 0;
	}	
}


//- YUART �ʱ�ȭ
void YUART_Init(void)
{
	int16_t i;

	yuart_RxFlag = 0;
	yuart_RxSCmdPtr = 0;
	yuart_TxQueHeadPtr = 0;
	yuart_TxQueTailPtr = 0;

	for (i=0; i<YUART_SCMD_NUM; i++) {
		yuart_pSCmdCallback[i] = NULL;
	}
	
	for (i=0; i<YUART_CCMD_NUM; i++) {
		yuart_pCCmdCallback[i] = NULL;
	}
	
	// RX Interrupt 
	LINFLEX_2.LINIER.B.DRIE = 1;
}


//- small command callback �Լ� ���
void YUART_RegisterSmallCmd(char scmd, void (*pCallback)(char*))
{
	if ((scmd<YUART_SCMD_START) || (scmd>YUART_SCMD_END)) {
		return;
	}
	yuart_pSCmdCallback[scmd-YUART_SCMD_START] = pCallback;
}


//- capital command callback �Լ� ���
void YUART_RegisterCapitalCmd(char ccmd, void (*pCallback)(void))
{
	if ((ccmd<YUART_CCMD_START) || (ccmd>YUART_CCMD_END)) {
		return;
	}
	yuart_pCCmdCallback[ccmd-YUART_CCMD_START] = pCallback;
}


//- Tx Queue�� �� ���� NGV_DMU_TRUE ����, �׷��� ������ NGV_DMU_FALSE ����
int16_t YUART_IsTxQueFull(void)
{
	int16_t size;
	
	DisableExternalInterrupts();
	size = (yuart_TxQueHeadPtr-yuart_TxQueTailPtr+YUART_TX_QUE_SIZE)%YUART_TX_QUE_SIZE;
	EnableExternalInterrupts();
	
	if (size>=(YUART_TX_QUE_SIZE-2)) {
		return NGV_DMU_TRUE;
	}
	return NGV_DMU_FALSE;
}


//- Tx Queue�� Ch ����. ��, Tx Queue�� �� ������ �� ������ ��ٸ�
void YUART_PutCh(char ch)
{
	vint32_t cnt=0;
	
	while (YUART_IsTxQueFull()) {
		if (++cnt>30000)
			break;
	}
	if (ch == YUART_ASCII_LF)
		YUART_PutCh(YUART_ASCII_CR);

	DisableExternalInterrupts();

	if (LINFLEX_2.LINIER.B.DTIE == 0) {
	    LINFLEX_2.UARTSR.R = 0x2;	// DTF
		LINFLEX_2.LINIER.B.DTIE = 1;
		LINFLEX_2.BDRL.R = ch;
		LINFLEX_2.LINSR.R = 0xF23F;
	}
	else {
		yuart_TxQue[yuart_TxQueHeadPtr] = ch;
		yuart_TxQueHeadPtr = (yuart_TxQueHeadPtr+1)%YUART_TX_QUE_SIZE;
	}
	EnableExternalInterrupts();
}


//- Tx Queue�� pstr�� ����Ű�� ���ڿ� ����
void YUART_PutStr(char* pstr)
{
	while (*pstr!='\0') {
		YUART_PutCh(*pstr++);
	}
}


//- Tx queue�� printf ��ɾ�� ���ڿ� ����, ����� ���ڰ� YUART_PRINTF_BUF_SIZE�� ������ å�� ����!
void YUART_Printf(const char* format, ...)
{
	char buf[YUART_PRINTF_BUF_SIZE];
	
	va_list args;
	va_start(args, format);
	vsprintf((char*)buf, format, args);
	va_end(args);
	
	YUART_PutStr(buf);
}


//- Event(�� ���� ��ɾ� �Ǵ� �� �� ��ɾ�)�� ������ NGV_DMU_TRUE ����, �׷��� ������ FASLE ����
uint16_t YUART_IsEvents(void)
{
	if ((yuart_RxFlag & YUART_RX_SCMD) || (yuart_RxFlag & YUART_RX_HCMD)) {
		return NGV_DMU_TRUE;
	}
	return NGV_DMU_FALSE;
}


//- ��ϵǾ� ���� ���� command�� ���� �� ���
void YUART_PrintNoRegCmd(void)
{
	YUART_Printf("NGV_DMU> \n");
}


//- YUART_IsEvents()�� NGV_DMU_TRUE �� �� Event�� �ش��ϴ� �Լ� ����
void YUART_ExecuteCallback(void)
{
	int16_t i;
	char cmd;

	// �� ���� ��ɾ ���� callback �Լ� ȣ��
	if (yuart_RxFlag & YUART_RX_HCMD) {
		if ((yuart_RxCCmd>=YUART_CCMD_START) && (yuart_RxCCmd<=YUART_CCMD_END)) {
			if (yuart_pCCmdCallback[yuart_RxCCmd-YUART_CCMD_START]!=NULL) {
				yuart_pCCmdCallback[yuart_RxCCmd-YUART_CCMD_START]();
			}
			else {
				YUART_PrintNoRegCmd();
			}
		}
		else {
			YUART_PrintNoRegCmd();
		}
		yuart_RxFlag &= ~YUART_RX_HCMD;
	}

	// �� �� ��ɾ ���� callback �Լ� ȣ��
	if (yuart_RxFlag & YUART_RX_SCMD) {
		for (i=0; i<yuart_RxSCmdPtr; i++) {
			if ((yuart_RxSCmd[i] == YUART_ASCII_LF) || yuart_RxSCmd[i] == ' ') {
				continue;
			}
			break; 
		}
		if (i!=yuart_RxSCmdPtr) {
			cmd = yuart_RxSCmd[i];
			if ((cmd>=YUART_SCMD_START) && (cmd<=YUART_SCMD_END)) {
				if (yuart_pSCmdCallback[cmd-YUART_SCMD_START]!=NULL) {
					yuart_pSCmdCallback[cmd-YUART_SCMD_START]((char*)&yuart_RxSCmd[i+1]);
				}
				else {
					YUART_PrintNoRegCmd();
				}
			} else {
				YUART_PrintNoRegCmd();
			}
		}
		else {
			YUART_PrintNoRegCmd();
		}
		yuart_RxSCmdPtr = 0;
		yuart_RxFlag &= ~YUART_RX_SCMD;
	}
}


