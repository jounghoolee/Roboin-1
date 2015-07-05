/*
 * rotaryencodr.h
 *
 *  Created on: Jan 11, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�. 
 */

#ifndef ROTARYENCODER_H_
#define ROTARYENCODER_H_

#include "jdp.h"

#define RENC_PRIORITY PORTH_PRIORITY

#define RENC_PPR_1X 1000 // Pulse Per Round

//- FUNCTION ----------------------------------------------
void RENC_IRQ16_23_ISR(void);
void RENC_Init(void);
void RENC_Set(int32_t val);
int32_t RENC_Get(void);

#endif /* ROTARYENCODER_H_ */
