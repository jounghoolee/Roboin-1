/*
 * demoswitch.c
 *
 *  Created on: Jan 1, 2015
 *      Author: ����̷����б� ����
 *        Note: 2015�� NGV ���л� �� ���� ����, ����, ������ ���մϴ�.
 */

#include "ngv_dmu.h"
#include "gpio.h"
#include "demoswitch.h"


//- Switch �ʱ�ȭ
void DSW_Init(void)
{
}

//- Tactile Switch 4�� ��ü�� On/Off ���¸� ����
//	  B0 �ڸ��� S1�� �ش�, 1�̸� ON, 0�̸� OFF
//	  B1 �ڸ��� S2
//	  B2 �ڸ��� S3
//	  B3 �ڸ��� S4
uint16_t DSW_GetTactAll(void)
{
	uint16_t val = 0;
	
	if (DSW_Get(DSW_S1)==DSW_ON)
		val |= 0x1;
	if (DSW_Get(DSW_S2)==DSW_ON)
		val |= 0x2;
	if (DSW_Get(DSW_S3)==DSW_ON)
		val |= 0x4;
	if (DSW_Get(DSW_S4)==DSW_ON)
		val |= 0x8;
	
	return val;
}

//- 4P Dip Switch ��ü�� On/Off ���¸� ����
//	  B0 �ڸ��� DIP1�� �ش�, 1�̸� ON, 0�̸� OFF
//	  B1 �ڸ��� DIP2
//	  B2 �ڸ��� DIP3
//	  B3 �ڸ��� DIP4
uint16_t DSW_GetDipAll(void)
{
	uint16_t val = 0;
	
	if (DSW_Get(DSW_DIP1)==DSW_ON)
		val |= 0x1;
	if (DSW_Get(DSW_DIP2)==DSW_ON)
		val |= 0x2;
	if (DSW_Get(DSW_DIP3)==DSW_ON)
		val |= 0x4;
	if (DSW_Get(DSW_DIP4)==DSW_ON)
		val |= 0x8;
	
	return val;
}
