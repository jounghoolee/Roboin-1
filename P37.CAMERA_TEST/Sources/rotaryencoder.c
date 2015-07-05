/*
 * rotaryencodr.c
 *
 *  Created on: Jan 11, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�. 
 */

#include "rotaryencoder.h"
#include "gpio.h"

vint32_t renc_val;


void RENC_IRQ16_23_ISR(void)
{
	SIU.ISR.R = 0x00FF0000;
	
	if (GPIO_Get(GPIO_PA12) == 1) {
		if (GPIO_Get(GPIO_PA13) == 1) {
			renc_val++;
		}
		else {
			renc_val--;
		}
	}
	else {
		if (GPIO_Get(GPIO_PA13) == 1) {
			renc_val--;
		}
		else {
			renc_val++;
		}
	}
}


void RENC_Init(void)
{
	renc_val = 0;
}


// val ������ encoder ȸ����(����: pulse)�� ����
void RENC_Set(int32_t val)
{
	renc_val = val;
}


// Pulse ������ Encoder ȸ������ ����
int32_t RENC_Get(void)
{
	return renc_val;
}


