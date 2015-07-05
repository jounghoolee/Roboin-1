/*
 * motorcontrol.c
 *
 *  Created on: Jan 11, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�.        
 */


#include "motorcontrol.h"
#include "gpio.h"


void MCTL_Init(void)
{
	GPIO_Set(GPIO_PA8, 1);
}

//- RC Servo ���Ϳ� angle�� �ش��ϴ� ���� ����� ����
//    angle�� ������ MCTL_SWHEEL_MIN ~ MCTL_SWHEEL_MAX
void MCTL_Servo(int16_t angle)
{
	angle += MCTL_SWHEEL_OFFSET;
	if (angle < MCTL_SWHEEL_MIN)
		angle = MCTL_SWHEEL_MIN;
	
	if (angle > MCTL_SWHEEL_MAX)
		angle = MCTL_SWHEEL_MAX;

	EMIOS_0.CH[1].CADR.R = (uint32_t)((angle<<5)+MCTL_SWHEEL_RAW_MID);
//	EMIOS_0.CH[1].CADR.R = (angle*(MCTL_SWHEEL_RAW_MAX-MCTL_SWHEEL_RAW_MID)/MCTL_SWHEEL_MAX)+MCTL_SWHEEL_RAW_MID;
}

//- DC Motor�� speed�� �ش��ϴ� PWM ���
//	  speed�� MCTL_ENGINE_MIN ~ MCTL_ENGINE_MAX ����
void MCTL_DC_R(int16_t speed)
{
	if (speed < MCTL_ENGINE_MIN)
		speed = MCTL_ENGINE_MIN;//-2047
	
	GPIO_Set(GPIO_PA4, 0);//HA IN2

	GPIO_Set(GPIO_PA4, 0);
	EMIOS_0.CH[2].CADR.R = speed;//PA2, AS HA IN1
}

void MCTL_DC_L(int16_t speed)
{
	if (speed < MCTL_ENGINE_MIN)
		speed = MCTL_ENGINE_MIN;//-2047
	
	GPIO_Set(GPIO_PA5, 0);//HB IN2
	
	GPIO_Set(GPIO_PA5, 0);
	EMIOS_0.CH[3].CADR.R = speed;//PA3, AS HB IN1
}

//- DC Motor Brake
void MCTL_BrakeEngle(void)
{
	GPIO_Set(GPIO_PA4, 0);
	GPIO_Set(GPIO_PA5, 0);
	EMIOS_0.CH[2].CADR.R = 0;
	EMIOS_0.CH[3].CADR.R = 0;
}
