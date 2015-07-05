/*
 * demoswitch.h
 *
 *  Created on: Jan 1, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�.  
 */

#ifndef DEMOSWITCH_H_
#define DEMOSWITCH_H_

#include "gpio.h"

#define DSW_S1		GPIO_PE0
#define DSW_S2		GPIO_PE1
#define DSW_S3		GPIO_PE2
#define DSW_S4		GPIO_PE3

#define DSW_DIP1	GPIO_PG6
#define DSW_DIP2	GPIO_PG7
#define DSW_DIP3	GPIO_PG8
#define DSW_DIP4	GPIO_PG9

#define DSW_ON 	1
#define DSW_OFF 	0


void DSW_Init(void);

//- Switch �� ���� ����(DSW_ON �Ǵ� DSW_OFF)�� ����
//	  sw: �� ��ũ�� ��� ���� (DSW_S1 ~ DSW_DIP4 �� �ϳ�)
#define DSW_Get(sw)	(GPIO_Get(sw) ? DSW_OFF : DSW_ON)

uint16_t DSW_GetTactAll(void);
uint16_t DSW_GetDipAll(void);

#endif /* DEMOSWITCH_H_ */
