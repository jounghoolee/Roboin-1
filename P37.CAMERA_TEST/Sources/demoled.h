/*
 * demoled.h
 *
 *  Created on: Dec 29, 2014
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�.      
 */

#ifndef DEMOLED_H_
#define DEMOLED_H_

#include "gpio.h"

#define DLED_LED1  GPIO_PE4
#define DLED_LED2  GPIO_PE5
#define DLED_LED3  GPIO_PE6
#define DLED_LED4  GPIO_PE7

#define DLED_ON 	1
#define DLED_OFF 	0



void DLED_Init(void);

//- LED �� ���� ON/OFF
//	  led: DLED_LED1~DLED_LED4 �� �ϳ�
// 	  onoff: DLED_ON, DLED_OFF �� ����
#define DLED_Set(led, onoff)	GPIO_Set((led), ((onoff)==DLED_ON ? 0 : 1))

//- LED �� ���� Toggle (ON�̸� OFF, OFF�� ON)
//	  led: DLED_LED1~DLED_LED4 �� �ϳ�
#define DLED_Toggle(led)		GPIO_Set((led), GPIO_GetDO(led) ? 0 : 1)


void DLED_SetAll(uint16_t val);


#endif /* DEMOLED_H_ */
