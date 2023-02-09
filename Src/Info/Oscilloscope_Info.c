/*
 * Oscilloscope_Info.c
 *
 *  Created on: Feb 8, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*	SELF	*/
#include "Oscilloscope_Info.h"


OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

#define START_INDEX_OF_CH2_INFO		6
#define START_INDEX_OF_OTHER_INFO	(START_INDEX_OF_CH2_INFO * 2)

void OSC_voidInitCh1Info(void)
{
	/*	freq	*/
	strcpy(Global_InfoArr[0].name, "F1");
	strcpy(Global_InfoArr[0].unit, "Hz");

	/*	vpp	*/
	strcpy(Global_InfoArr[1].name, "vpp1");
	strcpy(Global_InfoArr[1].unit, "V");

	/*	vMin	*/
	strcpy(Global_InfoArr[2].name, "vMin1");
	strcpy(Global_InfoArr[2].unit, "V");

	/*	vMax	*/
	strcpy(Global_InfoArr[3].name, "vMax1");
	strcpy(Global_InfoArr[3].unit, "V");

	/*	vAvg	*/
	strcpy(Global_InfoArr[4].name, "vAvg1");
	strcpy(Global_InfoArr[4].unit, "V");

	/*	volts per Div.	*/
	strcpy(Global_InfoArr[5].name, "vDiv1");
	strcpy(Global_InfoArr[5].unit, "V");
}

void OSC_voidInitCh2Info(void)
{
	/*	freq	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO].name, "F2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO].unit, "Hz");

	/*	vpp	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 1].name, "vpp2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 1].unit, "V");

	/*	vMin	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 2].name, "vMin2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 2].unit, "V");

	/*	vMax	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 3].name, "vMax2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 3].unit, "V");

	/*	vAvg	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 4].name, "vAvg2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 4].unit, "V");

	/*	volts per Div.	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 5].name, "vDiv2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 5].unit, "V");
}

void OSC_voidInitOtherInfo(void)
{
	/*	seconds per Div.	*/
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO].name, "tDiv");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO].unit, "S");

	/*	Voltage cursors	*/
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 1].name, "v1");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 1].unit, "V");

	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 2].name, "v2");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 2].unit, "V");

	/*	Time cursors	*/
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 3].name, "t1");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 3].unit, "S");

	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 4].name, "t2");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 4].unit, "S");

	/*	user defined expressions	*/
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 5].name, "ex1");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 5].unit, "");

	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 6].name, "ex2");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 6].unit, "");
}

void OSC_voidInitInfo(void)
{
	/**	Channel 1 info	**/
	OSC_voidInitCh1Info();

	/**	Channel 2 info	**/
	OSC_voidInitCh2Info();

	/**	Other info	**/
	OSC_voidInitOtherInfo();
}







