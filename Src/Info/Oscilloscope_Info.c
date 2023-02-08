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


volatile OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

void OSC_voidInitInfo(void)
{
	/**	Channel 1 info	**/
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
}







