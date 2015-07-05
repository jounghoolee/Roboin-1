/*
 * serialdebug.c
 *
 *  Created on: Jan 2, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�. 
 */

#include <stdio.h>
#include <stdarg.h>
#include "serialdebug.h"
#include "ngv_dmu.h"
#include "rappid_utils.h"


void (*sdbg_pSCmdCallback[SDBG_SCMD_NUM])(char *);
void (*sdbg_pCCmdCallback[SDBG_CCMD_NUM])(void);


#define SDBG_RX_SCMD 0x1
#define SDBG_RX_HCMD 0x2


vint8_t sdbg_RxFlag;
volatile char sdbg_RxCCmd;
volatile char  sdbg_RxSCmd[SDBG_SCMD_BUF_SIZE];
vint16_t sdbg_RxSCmdPtr;
vint16_t sdbg_TxQueHeadPtr, sdbg_TxQueTailPtr;
volatile char  sdbg_TxQue[SDBG_TX_QUE_SIZE];


//- UART RX0 ISR
void SDBG_RX0_ISR (void)
{
	char rxd;
	
	rxd = (char)LINFLEX_0.BDRM.R;

	//- Capital Command: �� ���� ��ɾ�
	if ((rxd>=SDBG_CCMD_START) && (rxd<=SDBG_CCMD_END)) {
		if (!(sdbg_RxFlag & SDBG_RX_HCMD)) {
			sdbg_RxCCmd = rxd;
			sdbg_RxFlag |= SDBG_RX_HCMD;
		}
	}
	// Small Command: �� �� ��ɾ�
	else {
		if (!(sdbg_RxFlag & SDBG_RX_SCMD)){
			if (rxd==SDBG_ASCII_CR) {
				sdbg_RxSCmd[sdbg_RxSCmdPtr] = '\0';
				sdbg_RxFlag |= SDBG_RX_SCMD;
				sdbg_RxSCmdPtr++;
			} else if (rxd==SDBG_ASCII_BS) {
				if (sdbg_RxSCmdPtr>0)
					sdbg_RxSCmdPtr--;
			} else {
				sdbg_RxSCmd[sdbg_RxSCmdPtr] = rxd;
				if (sdbg_RxSCmdPtr < (SDBG_SCMD_BUF_SIZE-1)) {
					sdbg_RxSCmdPtr++;
				}
			}
		}
	}
	
    LINFLEX_0.LINSR.R = 0xF23F;
    LINFLEX_0.UARTSR.R = 0x4;	// DRF
}


//- UART TX0 ISR
void SDBG_TX0_ISR (void)
{
    LINFLEX_0.LINSR.R = 0xF23F;
    LINFLEX_0.UARTSR.R = 0x2;	// DTF

    if (sdbg_TxQueHeadPtr!=sdbg_TxQueTailPtr) {
		LINFLEX_0.BDRL.R = sdbg_TxQue[sdbg_TxQueTailPtr];
		sdbg_TxQueTailPtr = (sdbg_TxQueTailPtr+1)%SDBG_TX_QUE_SIZE;
	}
	else {
		LINFLEX_0.LINIER.B.DTIE = 0;
	}	
}


//- SDBG �ʱ�ȭ
void SDBG_Init(void)
{
	int16_t i;

	sdbg_RxFlag = 0;
	sdbg_RxSCmdPtr = 0;
	sdbg_TxQueHeadPtr = 0;
	sdbg_TxQueTailPtr = 0;

	for (i=0; i<SDBG_SCMD_NUM; i++) {
		sdbg_pSCmdCallback[i] = NULL;
	}
	
	for (i=0; i<SDBG_CCMD_NUM; i++) {
		sdbg_pCCmdCallback[i] = NULL;
	}
	
	// RX Interrupt 
	LINFLEX_0.LINIER.B.DRIE = 1;
}


//- small command callback �Լ� ���
void SDBG_RegisterSmallCmd(char scmd, void (*pCallback)(char*))
{
	if ((scmd<SDBG_SCMD_START) || (scmd>SDBG_SCMD_END)) {
		return;
	}
	sdbg_pSCmdCallback[scmd-SDBG_SCMD_START] = pCallback;
}


//- capital command callback �Լ� ���
void SDBG_RegisterCapitalCmd(char ccmd, void (*pCallback)(void))
{
	if ((ccmd<SDBG_CCMD_START) || (ccmd>SDBG_CCMD_END)) {
		return;
	}
	sdbg_pCCmdCallback[ccmd-SDBG_CCMD_START] = pCallback;
}


//- Tx Queue�� �� ���� NGV_DMU_TRUE ����, �׷��� ������ NGV_DMU_FALSE ����
int16_t SDBG_IsTxQueFull(void)
{
	int16_t size;
	
	DisableExternalInterrupts();
	size = (sdbg_TxQueHeadPtr-sdbg_TxQueTailPtr+SDBG_TX_QUE_SIZE)%SDBG_TX_QUE_SIZE;
	EnableExternalInterrupts();
	
	if (size>=(SDBG_TX_QUE_SIZE-2)) {
		return NGV_DMU_TRUE;
	}
	return NGV_DMU_FALSE;
}


//- Tx Queue�� Ch ����. ��, Tx Queue�� �� ������ �� ������ ��ٸ�
void SDBG_PutCh(char ch)
{
	vint32_t cnt=0;
	
	while (SDBG_IsTxQueFull()) {
		if (++cnt>30000)
			break;
	}
	if (ch == SDBG_ASCII_LF)
		SDBG_PutCh(SDBG_ASCII_CR);

	DisableExternalInterrupts();

	if (LINFLEX_0.LINIER.B.DTIE == 0) {
	    LINFLEX_0.UARTSR.R = 0x2;	// DTF
		LINFLEX_0.LINIER.B.DTIE = 1;
		LINFLEX_0.BDRL.R = ch;
		LINFLEX_0.LINSR.R = 0xF23F;
	}
	else {
		sdbg_TxQue[sdbg_TxQueHeadPtr] = ch;
		sdbg_TxQueHeadPtr = (sdbg_TxQueHeadPtr+1)%SDBG_TX_QUE_SIZE;
	}
	EnableExternalInterrupts();
}


//- Tx Queue�� pstr�� ����Ű�� ���ڿ� ����
void SDBG_PutStr(char* pstr)
{
	while (*pstr!='\0') {
		SDBG_PutCh(*pstr++);
	}
}


//- Tx queue�� printf ��ɾ�� ���ڿ� ����, ����� ���ڰ� SDBG_PRINTF_BUF_SIZE�� ������ å�� ����!
void SDBG_Printf(const char* format, ...)
{
	char buf[SDBG_PRINTF_BUF_SIZE];
	
	va_list args;
	va_start(args, format);
	vsprintf((char*)buf, format, args);
	va_end(args);
	
	SDBG_PutStr(buf);
}


//- Event(�� ���� ��ɾ� �Ǵ� �� �� ��ɾ�)�� ������ NGV_DMU_TRUE ����, �׷��� ������ FASLE ����
uint16_t SDBG_IsEvents(void)
{
	if ((sdbg_RxFlag & SDBG_RX_SCMD) || (sdbg_RxFlag & SDBG_RX_HCMD)) {
		return NGV_DMU_TRUE;
	}
	return NGV_DMU_FALSE;
}


//- ��ϵǾ� ���� ���� command�� ���� �� ���
void SDBG_PrintNoRegCmd(void)
{
	SDBG_Printf("NGV_DMU> \n");
}


//- SDBG_IsEvents()�� NGV_DMU_TRUE �� �� Event�� �ش��ϴ� �Լ� ����
void SDBG_ExecuteCallback(void)
{
	int16_t i;
	char cmd;

	// �� ���� ��ɾ ���� callback �Լ� ȣ��
	if (sdbg_RxFlag & SDBG_RX_HCMD) {
		if ((sdbg_RxCCmd>=SDBG_CCMD_START) && (sdbg_RxCCmd<=SDBG_CCMD_END)) {
			if (sdbg_pCCmdCallback[sdbg_RxCCmd-SDBG_CCMD_START]!=NULL) {
				sdbg_pCCmdCallback[sdbg_RxCCmd-SDBG_CCMD_START]();
			}
			else {
				SDBG_PrintNoRegCmd();
			}
		}
		else {
			SDBG_PrintNoRegCmd();
		}
		sdbg_RxFlag &= ~SDBG_RX_HCMD;
	}

	// �� �� ��ɾ ���� callback �Լ� ȣ��
	if (sdbg_RxFlag & SDBG_RX_SCMD) {
		for (i=0; i<sdbg_RxSCmdPtr; i++) {
			if ((sdbg_RxSCmd[i] == SDBG_ASCII_LF) || sdbg_RxSCmd[i] == ' ') {
				continue;
			}
			break; 
		}
		if (i!=sdbg_RxSCmdPtr) {
			cmd = sdbg_RxSCmd[i];
			if ((cmd>=SDBG_SCMD_START) && (cmd<=SDBG_SCMD_END)) {
				if (sdbg_pSCmdCallback[cmd-SDBG_SCMD_START]!=NULL) {
					sdbg_pSCmdCallback[cmd-SDBG_SCMD_START]((char*)&sdbg_RxSCmd[i+1]);
				}
				else {
					SDBG_PrintNoRegCmd();
				}
			} else {
				SDBG_PrintNoRegCmd();
			}
		}
		else {
			SDBG_PrintNoRegCmd();
		}
		sdbg_RxSCmdPtr = 0;
		sdbg_RxFlag &= ~SDBG_RX_SCMD;
	}
}


