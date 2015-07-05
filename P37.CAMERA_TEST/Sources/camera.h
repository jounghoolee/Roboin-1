/*
 * camera.c
 *
 *  Created on: June 23, 2015
 *      Author: �������б� �����а� ������
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "jdp.h"

void CAM_Init(void);//calc delay variables
void CAM_RUN(void);// running camera
//void CAM_Clock_ISR(void);//clock pulse
//void CAM_Trigger_ISR(void);//Triger Pulse
void CAM_PulseWidth(int16_t PULSEWIDTH);//change camera clock, trig width

#define NUM_OF_PIXEL 128
int16_t CAM_MAX(void);//MAX Readed value
int16_t CAM_MIN(void);//Min Readed value 
int16_t CAM_DATA(int16_t pixelnum);//Processed CAM_READ
int16_t CAM_TRESHLOLD(int16_t pixelnum);

void CAM_DELAY(int32_t delay_long);
void CAM_RUN2(void);//Copy Last Year
void CAM_UPDATE_DATA(void);

#endif /* CAMERA_H_ */
