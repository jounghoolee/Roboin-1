/*
 * myuart.h
 *
 *  Created on: Jan 2, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�. 
 */

#ifndef MYUART_H_
#define MYUART_H_

#include "jdp.h"

#define MUART_ASCII_CR 	'\r'  // 13
#define MUART_ASCII_LF	'\n'  // 10
#define MUART_ASCII_BS	'\b'  // 8

// �� ���� ��ɾ�
#define MUART_CCMD_START	'A'
#define MUART_CCMD_END		'Z'
#define MUART_CCMD_NUM (MUART_CCMD_END-MUART_CCMD_START+1)

// \n���� ������ ���� ��ɾ�
#define MUART_SCMD_START	'a'
#define MUART_SCMD_END		'z'
#define MUART_SCMD_NUM		(MUART_SCMD_END-MUART_SCMD_START+1)

#define MUART_SCMD_BUF_SIZE		120
#define MUART_PRINTF_BUF_SIZE	160

#define MUART_TX_QUE_SIZE	300


//- FUNCTION ----------------------------------------------
void MUART_RX1_ISR(void);
void MUART_TX1_ISR(void);
void MUART_Init(void);
void MUART_RegisterSmallCmd(char scmd, void (*pCallback)(char*));
void MUART_RegisterCapitalCmd(char ccmd, void (*pCallback)(void));
int16_t MUART_IsTxQueFull(void);
void MUART_PutCh(char ch);
void MUART_PutStr(char* pstr);
void MUART_Printf(const char* format, ...);
uint16_t MUART_IsEvents(void);
void MUART_PrintNoRegCmd(void);
void MUART_ExecuteCallback(void);

#endif /* MYUART_H_ */
