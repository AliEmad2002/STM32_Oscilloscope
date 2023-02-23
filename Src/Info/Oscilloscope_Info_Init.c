/*
 * Oscilloscope_Info_Init.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "LinkedList.h"
#include "MathParser.h"

/*	MCAL	*/
#include "RCC_interface.h"
#include "DMA_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Info.h"
#include "Oscilloscope_Info_Init.h"


extern volatile OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

void OSC_voidInitCh1Info(OSC_Info_t* infoArr, char* strHz, char* strV, char* strS)
{
	/*	freq	*/
	static const char str0[] = "F1";
	infoArr[0].name = (char*)str0;
	infoArr[0].unit = strHz;
	infoArr[0].getValInNanoCallback = OSC_s64GetFreq1Info;

	/*	vpp	*/
	static const char str1[] = "vpp1";
	infoArr[1].name = (char*)str1;
	infoArr[1].unit = strV;
	infoArr[1].getValInNanoCallback = OSC_s64GetVpp1Info;

	/*	vMin	*/
	static const char str2[] = "vMin1";
	infoArr[2].name = (char*)str2;
	infoArr[2].unit = strV;
	infoArr[2].getValInNanoCallback = OSC_s64GetVmin1Info;

	/*	vMax	*/
	static const char str3[] = "vMax1";
	infoArr[3].name = (char*)str3;
	infoArr[3].unit = strV;
	infoArr[3].getValInNanoCallback = OSC_s64GetVmax1Info;

	/*	vAvg	*/
	static const char str4[] = "vAvg1";
	infoArr[4].name = (char*)str4;
	infoArr[4].unit = strV;
	infoArr[4].getValInNanoCallback = OSC_s64GetVavg1Info;

	/*	volts per Div.	*/
	static const char str5[] = "vDiv1";
	infoArr[5].name = (char*)str5;
	infoArr[5].unit = strV;
	infoArr[5].getValInNanoCallback = OSC_s64GetVdiv1Info;

	/*	active time	*/
	static const char str6[] = "tOn1";
	infoArr[6].name = (char*)str6;
	infoArr[6].unit = strS;
	infoArr[6].getValInNanoCallback = OSC_s64GetTon1Info;
}

void OSC_voidInitCh2Info(OSC_Info_t* infoArr, char* strHz, char* strV, char* strS)
{
	/*	freq	*/
	static const char str0[] = "F2";
	infoArr[START_INDEX_OF_CH2_INFO].name = (char*)str0;
	infoArr[START_INDEX_OF_CH2_INFO].unit = strHz;
	infoArr[START_INDEX_OF_CH2_INFO].getValInNanoCallback = OSC_s64GetFreq2Info;

	/*	vpp	*/
	static const char str1[] = "vpp2";
	infoArr[START_INDEX_OF_CH2_INFO + 1].name = (char*)str1;
	infoArr[START_INDEX_OF_CH2_INFO + 1].unit = strV;
	infoArr[START_INDEX_OF_CH2_INFO + 1].getValInNanoCallback =
		OSC_s64GetVpp2Info;

	/*	vMin	*/
	static const char str2[] = "vMin2";
	infoArr[START_INDEX_OF_CH2_INFO + 2].name = (char*)str2;
	infoArr[START_INDEX_OF_CH2_INFO + 2].unit = strV;
	infoArr[START_INDEX_OF_CH2_INFO + 2].getValInNanoCallback =
		OSC_s64GetVmin2Info;

	/*	vMax	*/
	static const char str3[] = "vMax2";
	infoArr[START_INDEX_OF_CH2_INFO + 3].name = (char*)str3;
	infoArr[START_INDEX_OF_CH2_INFO + 3].unit = strV;
	infoArr[START_INDEX_OF_CH2_INFO + 3].getValInNanoCallback =
		OSC_s64GetVmax2Info;

	/*	vAvg	*/
	static const char str4[] = "vAvg2";
	infoArr[START_INDEX_OF_CH2_INFO + 4].name = (char*)str4;
	infoArr[START_INDEX_OF_CH2_INFO + 4].unit = strV;
	infoArr[START_INDEX_OF_CH2_INFO + 4].getValInNanoCallback =
		OSC_s64GetVavg2Info;

	/*	volts per Div.	*/
	static const char str5[] = "vDiv2";
	infoArr[START_INDEX_OF_CH2_INFO + 5].name = (char*)str5;
	infoArr[START_INDEX_OF_CH2_INFO + 5].unit = strV;
	infoArr[START_INDEX_OF_CH2_INFO + 5].getValInNanoCallback =
		OSC_s64GetVdiv2Info;

	/*	active time	*/
	static const char str6[] = "tOn2";
	infoArr[START_INDEX_OF_CH2_INFO + 6].name = (char*)str6;
	infoArr[START_INDEX_OF_CH2_INFO + 6].unit = strS;
	infoArr[START_INDEX_OF_CH2_INFO + 6].getValInNanoCallback =
		OSC_s64GetTon2Info;
}

void OSC_voidInitOtherInfo(OSC_Info_t* infoArr, char* strV, char* strS)
{
	/*	seconds per Div.	*/
	static const char str0[] = "tDiv";
	infoArr[START_INDEX_OF_OTHER_INFO].name = (char*)str0;
	infoArr[START_INDEX_OF_OTHER_INFO].unit = strS;
	infoArr[START_INDEX_OF_OTHER_INFO].getValInNanoCallback =
		OSC_s64GetTdivInfo;

	/*	Voltage cursors	*/
	static const char str1[] = "v1";
	infoArr[START_INDEX_OF_OTHER_INFO + 1].name = (char*)str1;
	infoArr[START_INDEX_OF_OTHER_INFO + 1].unit = strV;

	static const char str2[] = "v2";
	infoArr[START_INDEX_OF_OTHER_INFO + 2].name = (char*)str2;
	infoArr[START_INDEX_OF_OTHER_INFO + 2].unit = strV;

	/*	Time cursors	*/
	static const char str3[] = "t1";
	infoArr[START_INDEX_OF_OTHER_INFO + 3].name = (char*)str3;
	infoArr[START_INDEX_OF_OTHER_INFO + 3].unit = strS;
	infoArr[START_INDEX_OF_OTHER_INFO + 3].getValInNanoCallback =
		OSC_s64GetT1Info;

	static const char str4[] = "t2";
	infoArr[START_INDEX_OF_OTHER_INFO + 4].name = (char*)str4;
	infoArr[START_INDEX_OF_OTHER_INFO + 4].unit = strS;
	infoArr[START_INDEX_OF_OTHER_INFO + 4].getValInNanoCallback =
		OSC_s64GetT2Info;
}

void OSC_voidInitInfoArr(void)
{
	static const char strHz[] = "Hz";
	static const char strV[] = "V";
	static const char strS[] = "S";

	/**	Channel 1 info	**/
	OSC_voidInitCh1Info(
		(OSC_Info_t*)Global_InfoArr, (char*)strHz, (char*)strV, (char*)strS);

	/**	Channel 2 info	**/
	OSC_voidInitCh2Info(
		(OSC_Info_t*)Global_InfoArr, (char*)strHz, (char*)strV, (char*)strS);

	/**	Other info	**/
	OSC_voidInitOtherInfo((OSC_Info_t*)Global_InfoArr, (char*)strV, (char*)strS);

	/**
	 * Disable all. (Default enable is loaded at the first config flash to
	 * RAM update).
	 **/
	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
		Global_InfoArr[i].enabled = false;
}

