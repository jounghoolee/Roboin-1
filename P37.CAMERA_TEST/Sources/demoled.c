/*
 * demoled.c
 *
 *  Created on: Dec 29, 2014
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�.            
 */

#include "ngv_dmu.h"
#include "gpio.h"
#include "demoled.h"

//- LED �ʱ�ȭ (���� OFF)
void DLED_Init(void)
{
	DLED_SetAll(0);
}


//- LED 4�� ��ü�� On/Off
//	  B0 �ڸ��� LED1�� �ش�, 1�̸� ON, 0�̸� OFF
//	  B1 �ڸ��� LED2
//	  B2 �ڸ��� LED3
//	  B3 �ڸ��� LED4
void DLED_SetAll(uint16_t val)
{
	DLED_Set(DLED_LED1, val&0x1 ? DLED_ON : DLED_OFF);
	DLED_Set(DLED_LED2, val&0x2 ? DLED_ON : DLED_OFF);
	DLED_Set(DLED_LED3, val&0x4 ? DLED_ON : DLED_OFF);
	DLED_Set(DLED_LED4, val&0x8 ? DLED_ON : DLED_OFF);
}

