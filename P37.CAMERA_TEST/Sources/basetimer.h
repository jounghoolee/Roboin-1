/*
 * basetimer.h
 *
 *  Created on: Jan 1, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�.        
 */

#ifndef BASETIMER_H_
#define BASETIMER_H_

#include "jdp.h"

void BTMR_PIT0_ISR (void);
void BTMR_Init(void);
void BTMR_DelayMs(uint32_t msec);
void BTMR_SetTimer0(uint32_t msec);
uint32_t BTMR_GetTimer0(void);
void  BTMR_SetTimer1(uint32_t msec);
uint32_t BTMR_GetTimer1(void);
uint32_t BTMR_GetRuntime(void);
void BMTR_SetRuntime(uint32_t msec);

#endif /* BASETIMER_H_ */
