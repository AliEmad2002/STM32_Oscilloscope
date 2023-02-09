/*
 * Oscilloscope_Info.c
 *
 *  Created on: Feb 8, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Delay_interface.h"
#include "Img_config.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Debug_active.h"
#include "Target_config.h"
#include "Error_Handler_interface.h"
#include "Txt_interface.h"
#include <stdio.h>
#include <stdlib.h>

/*	MCAL	*/
#include "RCC_interface.h"
#include "SCB_interface.h"
#include "NVIC_interface.h"
#include "STK_interface.h"
#include "DMA_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "EXTI_interface.h"
#include "ADC_interface.h"
#include "MDAC_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Info.h"

extern volatile TFT2_t Global_LCD;
extern volatile char Global_Str[128];

extern volatile u16* Global_ImgBufferArr[2];

extern volatile u8 Global_NotInUseImgBufferIndex;

extern void OSC_voidSetDisplayBoundariesForSignalArea(void);

OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

#define START_INDEX_OF_CH2_INFO		6
#define START_INDEX_OF_OTHER_INFO	(START_INDEX_OF_CH2_INFO * 2)

u64 fooCallback(void)
{
	return 15;
}

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

	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
	{
		Global_InfoArr[i].getValInNanoCallback = fooCallback;
		Global_InfoArr[i].enabled = false;
	}

	for (u8 i = 0; i < 4; i++)
	{
		Global_InfoArr[i].enabled = true;
	}

}

void OSC_voidEnableInfo(u8 infoIndex)
{
	Global_InfoArr[infoIndex].enabled = true;
}

void OSC_voidDisableInfo(u8 infoIndex)
{
	Global_InfoArr[infoIndex].enabled = false;
}

void OSC_voidDrawInfo(void)
{
	/*	counter of how many info's were drawn in this call	*/
	u8 drawnInfoCount = 0;

	for (u8 i = 0; i < NUMBER_OF_INFO && drawnInfoCount < 4; i++)
	{
		if (Global_InfoArr[i].enabled == false)
			continue;

		/*	get the string to print	*/
		OSC_voidGetInfoStringToPrint(Global_Str, i);

		/*	wait for previous img data transfer complete	*/
		TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

		/*	draw the string on img buffer	*/
		Txt_voidPrintStrOnPixArrRightOrientation(
			Global_Str, LCD_MAIN_DRAWING_COLOR_U16, LCD_BACKGROUND_COLOR_U16,
			0, 0,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex],
			8, 160);

		/*
		 * set TFT boundaries to these of the info to be printed.
		 * (based on "drawnInfoCount")
		 */
		OSC_voidSSetTFTBoundariesToInfo(drawnInfoCount);

		/*	send info image buffer	*/
		TFT2_voidSendPixels(
			&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex], 80 * 8);

		/*	increment drawn info counter	*/
		drawnInfoCount++;
	}

	/*	wait for last img data transfer complete	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

	/*	set TFT boundaries back to the signal drawing area	*/
	OSC_voidSetDisplayBoundariesForSignalArea();
}

void OSC_voidGetNumberPrintableVersion(
	u64 valInNano, u32* valInteger, u32* valFraction, char* unitPrefix)
{
	if (valInNano < 1e3)
	{
		*unitPrefix = 'n';
		*valInteger = valInNano;
		*valFraction = 0;
	}
	else if (valInNano < 1e6)
	{
		*unitPrefix = 'u';
		*valInteger = valInNano / 1e3;
		*valFraction = (valInNano - 1e3) / 1e2;
	}
	else if (valInNano < 1e9)
	{
		*unitPrefix = 'm';
		*valInteger = valInNano / 1e6;
		*valFraction = (valInNano - 1e6) / 1e5;
	}
	else if (valInNano < 1e12)
	{
		*unitPrefix = ' ';
		*valInteger = valInNano / 1e9;
		*valFraction = (valInNano - 1e9) / 1e8;
	}
	else if (valInNano < 1e15)
	{
		*unitPrefix = 'k';
		*valInteger = valInNano / 1e12;
		*valFraction = (valInNano - 1e12) / 1e11;
	}
	else if (valInNano < 1e18)
	{
		*unitPrefix = 'M';
		*valInteger = valInNano / 1e15;
		*valFraction = (valInNano - 1e15) / 1e14;
	}

	/*	fraction is maximumly of 1 digit	*/
	while (*valFraction > 10)
		*valFraction /= 10;
}

void OSC_voidGetInfoStringToPrint(char* str, u8 infoIndex)
{
	/*	get info current val in nano	*/
	u64 valInNano = Global_InfoArr[infoIndex].getValInNanoCallback();

	/*	get the printable version of that value	*/
	u32 valInteger, valFraction;
	char unitPrefix;
	OSC_voidGetNumberPrintableVersion(
		valInNano, &valInteger, &valFraction, &unitPrefix);

	/*	copy data to string	*/
	sprintf(
		str, "%s=%d.%d%c%s",
		Global_InfoArr[infoIndex].name,
		valInteger, valFraction, unitPrefix,
		Global_InfoArr[infoIndex].unit
	);
}

void OSC_voidSSetTFTBoundariesToInfo(u8 infoIndex)
{
	const u8 xMinArr[] = {120, 120, 0, 0};
	const u8 xMaxArr[] = {127, 127, 7, 7};
	const u8 yMinArr[] = {0, 80, 0, 80};
	const u8 yMaxArr[] = {79, 159, 79, 159};

	TFT2_SET_X_BOUNDARIES(&Global_LCD, xMinArr[infoIndex], xMaxArr[infoIndex]);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, yMinArr[infoIndex], yMaxArr[infoIndex]);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);
}


