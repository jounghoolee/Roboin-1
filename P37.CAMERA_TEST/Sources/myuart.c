/*
 * myuart.c
 *
 *  Created on: Jan 2, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�. 
 */

#include <stdio.h>
#include <stdarg.h>
#include "myuart.h"
#include "ngv_dmu.h"
#include "rappid_utils.h"


void (*muart_pSCmdCallback[MUART_SCMD_NUM])(char *);
void (*muart_pCCmdCallback[MUART_CCMD_NUM])(void);


#define MUART_RX_SCMD 0x1
#define MUART_RX_HCMD 0x2


vint8_t muart_RxFlag;
volatile char muart_RxCCmd;
volatile char  muart_RxSCmd[MUART_SCMD_BUF_SIZE];
vint16_t muart_RxSCmdPtr;
vint16_t muart_TxQueHeadPtr, muart_TxQueTailPtr;
volatile char  muart_TxQue[MUART_TX_QUE_SIZE];


//- UART RX1 ISR
void MUART_RX1_ISR (void)
{
	char rxd;
	
	rxd = (char)LINFLEX_1.BDRM.R;

	//- Capital Command: �� ���� ��ɾ�
	if ((rxd>=MUART_CCMD_START) && (rxd<=MUART_CCMD_END)) {
		if (!(muart_RxFlag & MUART_RX_HCMD)) {
			muart_RxCCmd = rxd;
			muart_RxFlag |= MUART_RX_HCMD;
		}
	}
	// Small Command: �� �� ��ɾ�
	else {
		if (!(muart_RxFlag & MUART_RX_SCMD)){
			if (rxd==MUART_ASCII_CR) {
				muart_RxSCmd[muart_RxSCmdPtr] = '\0';
				muart_RxFlag |= MUART_RX_SCMD;
				muart_RxSCmdPtr++;
			} else if (rxd==MUART_ASCII_BS) {
				if (muart_RxSCmdPtr>0)
					muart_RxSCmdPtr--;
			} else {
				muart_RxSCmd[muart_RxSCmdPtr] = rxd;
				if (muart_RxSCmdPtr < (MUART_SCMD_BUF_SIZE-1)) {
					muart_RxSCmdPtr++;
				}
			}
		}
	}
	
    LINFLEX_1.LINSR.R = 0xF23F;
    LINFLEX_1.UARTSR.R = 0x4;	// DRF
}


//- UART TX1 ISR
void MUART_TX1_ISR (void)
{
    LINFLEX_1.LINSR.R = 0xF23F;
    LINFLEX_1.UARTSR.R = 0x2;	// DTF

    if (muart_TxQueHeadPtr!=muart_TxQueTailPtr) {
		LINFLEX_1.BDRL.R = muart_TxQue[muart_TxQueTailPtr];
		muart_TxQueTailPtr = (muart_TxQueTailPtr+1)%MUART_TX_QUE_SIZE;
	}
	else {
		LINFLEX_1.LINIER.B.DTIE = 0;
	}	
}


//- MUART �ʱ�ȭ
void MUART_Init(void)
{
	int16_t i;

	muart_RxFlag = 0;
	muart_RxSCmdPtr = 0;
	muart_TxQueHeadPtr = 0;
	muart_TxQueTailPtr = 0;

	for (i=0; i<MUART_SCMD_NUM; i++) {
		muart_pSCmdCallback[i] = NULL;
	}
	
	for (i=0; i<MUART_CCMD_NUM; i++) {
		muart_pCCmdCallback[i] = NULL;
	}
	
	// RX Interrupt 
	LINFLEX_1.LINIER.B.DRIE = 1;
}


//- small command callback �Լ� ���
void MUART_RegisterSmallCmd(char scmd, void (*pCallback)(char*))
{
	if ((scmd<MUART_SCMD_START) || (scmd>MUART_SCMD_END)) {
		return;
	}
	muart_pSCmdCallback[scmd-MUART_SCMD_START] = pCallback;
}


//- capital command callback �Լ� ���
void MUART_RegisterCapitalCmd(char ccmd, void (*pCallback)(void))
{
	if ((ccmd<MUART_CCMD_START) || (ccmd>MUART_CCMD_END)) {
		return;
	}
	muart_pCCmdCallback[ccmd-MUART_CCMD_START] = pCallback;
}


//- Tx Queue�� �� ���� NGV_DMU_TRUE ����, �׷��� ������ NGV_DMU_FALSE ����
int16_t MUART_IsTxQueFull(void)
{
	int16_t size;
	
	DisableExternalInterrupts();
	size = (muart_TxQueHeadPtr-muart_TxQueTailPtr+MUART_TX_QUE_SIZE)%MUART_TX_QUE_SIZE;
	EnableExternalInterrupts();
	
	if (size>=(MUART_TX_QUE_SIZE-2)) {
		return NGV_DMU_TRUE;
	}
	return NGV_DMU_FALSE;
}


//- Tx Queue�� Ch ����. ��, Tx Queue�� �� ������ �� ������ ��ٸ�
void MUART_PutCh(char ch)
{
	vint32_t cnt=0;
	
	while (MUART_IsTxQueFull()) {
		if (++cnt>30000)
			break;
	}
	if (ch == MUART_ASCII_LF)
		MUART_PutCh(MUART_ASCII_CR);

	DisableExternalInterrupts();

	if (LINFLEX_1.LINIER.B.DTIE == 0) {
	    LINFLEX_1.UARTSR.R = 0x2;	// DTF
		LINFLEX_1.LINIER.B.DTIE = 1;
		LINFLEX_1.BDRL.R = ch;
		LINFLEX_1.LINSR.R = 0xF23F;
	}
	else {
		muart_TxQue[muart_TxQueHeadPtr] = ch;
		muart_TxQueHeadPtr = (muart_TxQueHeadPtr+1)%MUART_TX_QUE_SIZE;
	}
	EnableExternalInterrupts();
}


//- Tx Queue�� pstr�� ����Ű�� ���ڿ� ����
void MUART_PutStr(char* pstr)
{
	while (*pstr!='\0') {
		MUART_PutCh(*pstr++);
	}
}


//- Tx queue�� printf ��ɾ�� ���ڿ� ����, ����� ���ڰ� MUART_PRINTF_BUF_SIZE�� ������ å�� ����!
void MUART_Printf(const char* format, ...)
{
	char buf[MUART_PRINTF_BUF_SIZE];
	
	va_list args;
	va_start(args, format);
	vsprintf((char*)buf, format, args);
	va_end(args);
	
	MUART_PutStr(buf);
}


//- Event(�� ���� ��ɾ� �Ǵ� �� �� ��ɾ�)�� ������ NGV_DMU_TRUE ����, �׷��� ������ FASLE ����
uint16_t MUART_IsEvents(void)
{
	if ((muart_RxFlag & MUART_RX_SCMD) || (muart_RxFlag & MUART_RX_HCMD)) {
		return NGV_DMU_TRUE;
	}
	return NGV_DMU_FALSE;
}


//- ��ϵǾ� ���� ���� command�� ���� �� ���
void MUART_PrintNoRegCmd(void)
{
	MUART_Printf("NGV_DMU> \n");
}


//- MUART_IsEvents()�� NGV_DMU_TRUE �� �� Event�� �ش��ϴ� �Լ� ����
void MUART_ExecuteCallback(void)
{
	int16_t i;
	char cmd;

	// �� ���� ��ɾ ���� callback �Լ� ȣ��
	if (muart_RxFlag & MUART_RX_HCMD) {
		if ((muart_RxCCmd>=MUART_CCMD_START) && (muart_RxCCmd<=MUART_CCMD_END)) {
			if (muart_pCCmdCallback[muart_RxCCmd-MUART_CCMD_START]!=NULL) {
				muart_pCCmdCallback[muart_RxCCmd-MUART_CCMD_START]();
			}
			else {
				MUART_PrintNoRegCmd();
			}
		}
		else {
			MUART_PrintNoRegCmd();
		}
		muart_RxFlag &= ~MUART_RX_HCMD;
	}

	// �� �� ��ɾ ���� callback �Լ� ȣ��
	if (muart_RxFlag & MUART_RX_SCMD) {
		for (i=0; i<muart_RxSCmdPtr; i++) {
			if ((muart_RxSCmd[i] == MUART_ASCII_LF) || muart_RxSCmd[i] == ' ') {
				continue;
			}
			break; 
		}
		if (i!=muart_RxSCmdPtr) {
			cmd = muart_RxSCmd[i];
			if ((cmd>=MUART_SCMD_START) && (cmd<=MUART_SCMD_END)) {
				if (muart_pSCmdCallback[cmd-MUART_SCMD_START]!=NULL) {
					muart_pSCmdCallback[cmd-MUART_SCMD_START]((char*)&muart_RxSCmd[i+1]);
				}
				else {
					MUART_PrintNoRegCmd();
				}
			} else {
				MUART_PrintNoRegCmd();
			}
		}
		else {
			MUART_PrintNoRegCmd();
		}
		muart_RxSCmdPtr = 0;
		muart_RxFlag &= ~MUART_RX_SCMD;
	}
}


