/*
 * camera.c
 *
 *  Created on: Feb 23, 2015
 *      Author: 연세대학교 기계공학과 정재학
 */

#include "camera.h"
#include "freerunadc.h"
#include "ngv_dmu.h"
#include "gpio.h"


//####### Pin Guide ##########
int16_t CAM_DATA_PIN= 5;//5=pd0//14=GPIO_PD10;//PD10;
int16_t CAM_TRIGGER_PIN= GPIO_PF0;//GPIO_PD12;
int16_t CAM_CLOCK_PIN= GPIO_PF1;//GPIO_PF1;

//####### timer variables ##########
int16_t TIME_CONST= 32;//1*(16<<NGV_DMU_SYSCLK);
int16_t DELAY_TRIG= 30;//20*TIME_CONST;//(16 << NGV_DMU_SYSCLK);//micosec
int16_t DELAY_CLK= 50;//25*TIME_CONST;//*(16 << NGV_DMU_SYSCLK);//micosec
int16_t DELAY_ADD= 500;//30*TIME_CONST;//*(16 << NGV_DMU_SYSCLK);//micosec
int16_t CAM_TIME_TOT= 0;// = delay_clk*128*2 + delay_additional;//11번	 //micro sec 

vuint32_t t0_CAM_TRIG;
vuint32_t t0_CAM_CLK;//variable for store timers
vuint32_t t0_CAM_CLK_DOWN;
vuint32_t t0_CAM_WAIT;

int8_t CAM_TIMER_CH= 4; // pit ch 3 or 4, but 3 is for FUSS
int16_t CAM_CLK_COUNTER= 0;

int8_t CAM_TRIGGER_FLAG= 1;
int8_t CAM_CLOCK_FLAG= 1;

vint16_t CAM_TEMP[NUM_OF_PIXEL+1] = {};
vint16_t CAM_READ[NUM_OF_PIXEL] = {};
vint16_t dummy_data[NUM_OF_PIXEL] = {};

void CAM_Init(void)
{	
	TIME_CONST= (16<<NGV_DMU_SYSCLK);
	DELAY_TRIG= DELAY_TRIG* TIME_CONST; //(16 << NGV_DMU_SYSCLK);//micosec
	DELAY_CLK= DELAY_CLK* TIME_CONST;//*(16 << NGV_DMU_SYSCLK);//micosec
	DELAY_ADD= DELAY_ADD* TIME_CONST;//*(16 << NGV_DMU_SYSCLK);//micosec
	CAM_TIME_TOT= DELAY_CLK*128*2*2 + DELAY_ADD; //128*2 cause up & down//micro sec 
	PIT.CH[CAM_TIMER_CH].TFLG.R = 0x00000001;
}

void CAM_RUN(void)
{
//CAM_Trigger_ISR();//For trigger Pulse
	if(CAM_TRIGGER_FLAG==1){//Trigger On
		GPIO_Set(CAM_TRIGGER_PIN, 1);//Trigger Pulse
		t0_CAM_TRIG = PIT.CH[CAM_TIMER_CH].CVAL.R;//trigger Timer Start
		//t0_CAM_CLK = PIT.CH[CAM_TIMER_CH].CVAL.R;
		CAM_TRIGGER_FLAG = 2;//3;
	}
	if((t0_CAM_TRIG - PIT.CH[CAM_TIMER_CH].CVAL.R) > DELAY_TRIG){//trigger down after 20micro
		GPIO_Set(CAM_TRIGGER_PIN, 0);	
	}
	
//CAM_Clock_ISR(); // For 128 + 1 Clock Pulse
	if(CAM_CLK_COUNTER < (NUM_OF_PIXEL+1) ){//128+1 cause there is an 1st pulse at Trigger
		//if(CAM_TRIGGER_FLAG==3){
			/*if((t0_CAM_TRIG - PIT.CH[CAM_TIMER_CH].CVAL.R) > DELAY_CLK/2){
				GPIO_Set(CAM_CLOCK_PIN, 1);//First clock Pulse	
				if((t0_CAM_CLK - PIT.CH[CAM_TIMER_CH].CVAL.R) > DELAY_CLK*3/2 ){
					t0_CAM_CLK_DOWN = PIT.CH[CAM_TIMER_CH].CVAL.R;
					CAM_CLOCK_FLAG= 0;
					CAM_TRIGGER_FLAG = 2;
				//}
			}
		}
		
		else */
		if(CAM_TRIGGER_FLAG == 2){
			if(CAM_CLOCK_FLAG== 1){//Clock On
				GPIO_Set(CAM_CLOCK_PIN, 1);
				//CAM_DATA1[CAM_CLK_COUNTER] = FADC_Get(CAM_DATA_PIN);//Read Pixel
				t0_CAM_CLK = PIT.CH[CAM_TIMER_CH].CVAL.R;//clk Timer restart
				CAM_CLOCK_FLAG= 2;
			}
			else if (CAM_CLOCK_FLAG==2){//clock high
				if((t0_CAM_CLK - PIT.CH[CAM_TIMER_CH].CVAL.R) > DELAY_CLK){
					CAM_TEMP[CAM_CLK_COUNTER] = FADC_Get(CAM_DATA_PIN);//Read Pixel
					GPIO_Set(CAM_CLOCK_PIN, 0);
					t0_CAM_CLK_DOWN = PIT.CH[CAM_TIMER_CH].CVAL.R;
					CAM_CLOCK_FLAG = 0;
				}
			}
			else if(CAM_CLOCK_FLAG == 0){//Clock down
				if((t0_CAM_CLK_DOWN - PIT.CH[CAM_TIMER_CH].CVAL.R) > DELAY_CLK){//Clock ReUp high25,Low25 Micro 
					CAM_CLK_COUNTER++; //= CAM_CLK_COUNTER+ 1;
					CAM_CLOCK_FLAG= 1;
				}
			}
		}
	}
	/*else{//CLOCK>=128
		GPIO_Set(CAM_CLOCK_PIN, 0);
		CAM_CLOCK_FLAG= 0;
	}*/
	
	
	////Check CAMERA END
	//if((t0_CAM_TRIG - PIT.CH[CAM_TIMER_CH].CVAL.R) > CAM_TIME_TOT){
	//if(CAM_CLK_COUNTER >= (NUM_OF_PIXEL+1) ){
	else{
		if(CAM_TRIGGER_FLAG == 2){
			t0_CAM_WAIT = PIT.CH[CAM_TIMER_CH].CVAL.R;//wait start
			CAM_TRIGGER_FLAG = 0;
			CAM_UPDATE_DATA();
			CAM_CLOCK_FLAG= 0;
		}
		GPIO_Set(CAM_TRIGGER_PIN, 0);	
		GPIO_Set(CAM_CLOCK_PIN, 0);
		//if((t0_CAM_TRIG - PIT.CH[CAM_TIMER_CH].CVAL.R) > CAM_TIME_TOT){
		if((t0_CAM_WAIT - PIT.CH[CAM_TIMER_CH].CVAL.R) > DELAY_ADD){//CAM_TIME_TOT){
			CAM_CLK_COUNTER= 0;//Clock counter reset
			CAM_TRIGGER_FLAG= 1;//trigger reset
			CAM_CLOCK_FLAG= 1;
			PIT.CH[CAM_TIMER_CH].TFLG.R = 0x00000001; // timer reset
		}
	}
}

void CAM_RUN2(void)
{
	CAM_CLK_COUNTER = 0;
	GPIO_Set(CAM_TRIGGER_PIN, 1);//Trigger Pulse
	GPIO_Set(CAM_CLOCK_PIN, 1);//First clock pulse
	CAM_DELAY(DELAY_TRIG);
	GPIO_Set(CAM_TRIGGER_PIN, 0);//Trigger Down
	CAM_DELAY(DELAY_CLK-DELAY_TRIG);
	//dummy_data[CAM_CLK_COUNTER] = FADC_Get(CAM_DATA_PIN);//Read Pixel
	CAM_TEMP[0] = FADC_Get(CAM_DATA_PIN);//Read Pixel
	GPIO_Set(CAM_CLOCK_PIN, 0);//Dummy Clock Down
	
	for(CAM_CLK_COUNTER=1; CAM_CLK_COUNTER < NUM_OF_PIXEL+1; CAM_CLK_COUNTER++){
		GPIO_Set(CAM_CLOCK_PIN, 1);//CLock On
		CAM_DELAY(DELAY_CLK);
		CAM_TEMP[CAM_CLK_COUNTER] = FADC_Get(CAM_DATA_PIN);//Read Pixel
		GPIO_Set(CAM_CLOCK_PIN, 0);//Clock Down
		CAM_DELAY(DELAY_CLK);
	}
	CAM_UPDATE_DATA();
	CAM_DELAY(DELAY_ADD);
}

int16_t i_cam=0;
int16_t pt1 = 13;//10*100/50; //#For 100ms#
int16_t pt2 = 45;//43*100/50;
int16_t pt3 = 118;//NUM_OF_PIXEL-(NUM_OF_PIXEL-120)*100/50;
int16_t initial_value = 627;

int16_t CAM_DATA(int16_t pixelnum){//기울기 크면 데이터 처리.
//50micro - 0-19 하강 & 20-85 완만하게 상승
//100micro  0-13 하강, 13-45 상승 118부터 급하강
	/* int16_t cam_read_offset = CAM_MAX()*276/800;//800/4;//CAM_MAX()/4;
	if(pixelnum <= pt1)
		return CAM_READ[pixelnum]*800/(-9.45*pixelnum+628);
	else if( (pt1 < pixelnum) && (pixelnum <= pt2) )
		return CAM_READ[pixelnum]*800/(7.7*(pixelnum-pt1)+518);
	else if( pt3 < pixelnum )
		return CAM_READ[pixelnum]*800/(-9.7*(pixelnum-pt3)+798);
	else
		return CAM_READ[pixelnum];*/
	return CAM_READ[pixelnum];
}


int16_t CAM_MAX(void){
	int16_t max_value = CAM_READ[0];
	for(i_cam=0; i_cam < (NUM_OF_PIXEL); i_cam++){
		if(max_value < CAM_READ[i_cam])
			max_value = CAM_READ[i_cam];
	}
	return max_value;
}

int16_t CAM_MIN(void){
	int16_t min_value = CAM_READ[0];
	for(i_cam=0; i_cam < (NUM_OF_PIXEL); i_cam++){
		if(min_value > CAM_READ[i_cam])
			min_value = CAM_READ[i_cam];
	}
	return min_value;
}
int16_t CAM_TRESHLOLD(int16_t pixelnum)
{
	int16_t CAM_READ_AVG = (CAM_MIN()+CAM_MAX())/2;
	if(pixelnum < pt1)
		return CAM_MAX()*3/4;
	else if( (pt1 <= pixelnum) && (pixelnum < pt2) )
		return CAM_MAX()*3/4;
	else if( (3*pt3-2*NUM_OF_PIXEL) <= pixelnum )
		return CAM_MAX()*7/8;
	else
		return CAM_READ_AVG;// + 20;//+10 to too dark
}

void CAM_DELAY(int32_t delay_long)
{
	vuint32_t t0_CAM_DELAY = PIT.CH[CAM_TIMER_CH].CVAL.R;
	while ((t0_CAM_DELAY - PIT.CH[CAM_TIMER_CH].CVAL.R)<(delay_long)) {}//32 = 1 microsec
}

void CAM_PulseWidth(int16_t PULSEWIDTH)
{
	pt1 = 10*100/PULSEWIDTH;//for read to data 보정
	pt2 = 43*100/PULSEWIDTH;
	pt3 = NUM_OF_PIXEL-(NUM_OF_PIXEL-120)*100/PULSEWIDTH;
	DELAY_TRIG= PULSEWIDTH;
	DELAY_CLK= PULSEWIDTH;
	DELAY_TRIG= DELAY_TRIG* TIME_CONST;
	DELAY_CLK= DELAY_CLK* TIME_CONST;
}

void CAM_UPDATE_DATA(void)
{
	int16_t i=0;
	for(i=0;i<128;i++)
	{
		CAM_READ[i] = CAM_TEMP[i+1];
	}
}
