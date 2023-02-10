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
#include <string.h>

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
#include "Oscilloscope_Private.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Info.h"

/*	global variables extern	*/
extern volatile TFT2_t Global_LCD;
extern volatile char Global_Str[128];
extern volatile u16* Global_ImgBufferArr[2];
extern volatile u8 Global_NotInUseImgBufferIndex;
extern volatile b8 Global_Paused;
extern volatile u8 Global_Ch1PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch1MinValueInCurrentFrame;
extern volatile u8 Global_Ch1MaxValueInCurrentFrame;
extern volatile u8 Global_Ch2PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch2MinValueInCurrentFrame;
extern volatile u8 Global_Ch2MaxValueInCurrentFrame;
extern volatile u32 Global_CurrentCh1MicroVoltsPerPix;
extern volatile u32 Global_CurrentCh2MicroVoltsPerPix;
extern volatile u64 Global_CurrentNanoSecondsPerPix;
extern volatile s32 Global_Offset1MicroVolts;
extern volatile s32 Global_Offset2MicroVolts;

extern volatile OSC_Cursor_t Cursor_v1;
extern volatile OSC_Cursor_t Cursor_v2;
extern volatile OSC_Cursor_t Cursor_t1;
extern volatile OSC_Cursor_t Cursor_t2;

/*	set boundaries function (from Oscilloscope_program.c)	*/
extern void OSC_voidSetDisplayBoundariesForSignalArea(void);

/*	global info array	*/
OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

#define START_INDEX_OF_CH2_INFO		6
#define START_INDEX_OF_OTHER_INFO	(START_INDEX_OF_CH2_INFO * 2)

/**	Callbacks	**/
/*	freq1	*/
s64 OSC_s64GetFreq1Info(void)
{
	static u64 lastFreqMeasureBeforePause = 0;

	if (!Global_Paused)
	{
		u64 freqmHz;
		if (
			TIM_GET_STATUS_FLAG(
				FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER, TIM_Status_CC1)
		)
			lastFreqMeasureBeforePause =
				TIM_u64GetFrequencyMeasured(FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER);
	}

	return lastFreqMeasureBeforePause * 1e6;
}

void OSC_voidEnableFreq1Info(void)
{
	Global_InfoArr[0].enabled = true;
}

void OSC_voidDisableFreq1Info(void)
{
	Global_InfoArr[0].enabled = true;
}

/*	freq2	*/
s64 OSC_s64GetFreq2Info(void)
{
	static u64 lastFreqMeasureBeforePause = 0;

	if (!Global_Paused)
	{
		u64 freqmHz;
		if (
			TIM_GET_STATUS_FLAG(
				FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER, TIM_Status_CC1)
		)
			lastFreqMeasureBeforePause =
				TIM_u64GetFrequencyMeasured(FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER);
	}

	return lastFreqMeasureBeforePause * 1e6;
}

/*	vpp1	*/
s64 OSC_s64GetVpp1Info(void)
{
	return
		Global_Ch1PeakToPeakValueInCurrentFrame *
		Global_CurrentCh1MicroVoltsPerPix * 1e3;
}

/*	vpp2	*/
s64 OSC_s64GetVpp2Info(void)
{
	return
		Global_Ch2PeakToPeakValueInCurrentFrame *
		Global_CurrentCh2MicroVoltsPerPix * 1e3;
}

/*	vMin1	*/
s64 OSC_s64GetVmin1Info(void)
{
	return
		((Global_Ch1MinValueInCurrentFrame - SIGNAL_LINE_LENGTH / 2) *
		Global_CurrentCh1MicroVoltsPerPix - Global_Offset1MicroVolts) * 1e3;
}

/*	vMin2	*/
s64 OSC_s64GetVmin2Info(void)
{
	return
		((Global_Ch2MinValueInCurrentFrame - SIGNAL_LINE_LENGTH / 2) *
		Global_CurrentCh2MicroVoltsPerPix - Global_Offset2MicroVolts) * 1e3;
}

/*	vMax1	*/
s64 OSC_s64GetVmax1Info(void)
{
	return
		((Global_Ch1MaxValueInCurrentFrame - SIGNAL_LINE_LENGTH / 2) *
		Global_CurrentCh1MicroVoltsPerPix - Global_Offset1MicroVolts) * 1e3;
}

/*	vMax2	*/
s64 OSC_s64GetVmax2Info(void)
{
	return
		((Global_Ch2MaxValueInCurrentFrame - SIGNAL_LINE_LENGTH / 2) *
		Global_CurrentCh2MicroVoltsPerPix - Global_Offset2MicroVolts) * 1e3;
}

/*	vAvg1	*/
s64 OSC_s64GetVavg1Info(void)
{
	return (OSC_s64GetVmax1Info() - OSC_s64GetVmin1Info()) / 2;
}

/*	vAvg2	*/
s64 OSC_s64GetVavg2Info(void)
{
	return (OSC_s64GetVmax2Info() - OSC_s64GetVmin2Info()) / 2;
}

/*	vDiv1	*/
s64 OSC_s64GetVdiv1Info(void)
{
	return Global_CurrentCh1MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV * 1e3;
}

/*	vDiv2	*/
s64 OSC_s64GetVdiv2Info(void)
{
	return Global_CurrentCh2MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV * 1e3;
}

/*	tDiv	*/
s64 OSC_s64GetTdivInfo(void)
{
	return Global_CurrentNanoSecondsPerPix * PIXELS_PER_TIME_DIV;
}

/*	t1	*/
s64 OSC_s64GetT1Info(void)
{
	return Cursor_t1.pos * Global_CurrentNanoSecondsPerPix;
}

/*	t2	*/
s64 OSC_s64GetT2Info(void)
{
	return Cursor_t2.pos * Global_CurrentNanoSecondsPerPix;
}

/**	Init	**/
void OSC_voidInitCh1Info(void)
{
	/*	freq	*/
	strcpy(Global_InfoArr[0].name, "F1");
	strcpy(Global_InfoArr[0].unit, "Hz");
	Global_InfoArr[0].getValInNanoCallback = OSC_s64GetFreq1Info;

	/*	vpp	*/
	strcpy(Global_InfoArr[1].name, "vpp1");
	strcpy(Global_InfoArr[1].unit, "V");
	Global_InfoArr[1].getValInNanoCallback = OSC_s64GetVpp1Info;

	/*	vMin	*/
	strcpy(Global_InfoArr[2].name, "vMin1");
	strcpy(Global_InfoArr[2].unit, "V");
	Global_InfoArr[2].getValInNanoCallback = OSC_s64GetVmin1Info;

	/*	vMax	*/
	strcpy(Global_InfoArr[3].name, "vMax1");
	strcpy(Global_InfoArr[3].unit, "V");
	Global_InfoArr[3].getValInNanoCallback = OSC_s64GetVmax1Info;

	/*	vAvg	*/
	strcpy(Global_InfoArr[4].name, "vAvg1");
	strcpy(Global_InfoArr[4].unit, "V");
	Global_InfoArr[4].getValInNanoCallback = OSC_s64GetVavg1Info;

	/*	volts per Div.	*/
	strcpy(Global_InfoArr[5].name, "vDiv1");
	strcpy(Global_InfoArr[5].unit, "V");
	Global_InfoArr[5].getValInNanoCallback = OSC_s64GetVdiv1Info;
}

void OSC_voidInitCh2Info(void)
{
	/*	freq	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO].name, "F2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO].unit, "Hz");
	Global_InfoArr[START_INDEX_OF_CH2_INFO].getValInNanoCallback =
		OSC_s64GetFreq2Info;

	/*	vpp	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 1].name, "vpp2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 1].unit, "V");
	Global_InfoArr[START_INDEX_OF_CH2_INFO + 1].getValInNanoCallback =
		OSC_s64GetVpp2Info;

	/*	vMin	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 2].name, "vMin2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 2].unit, "V");
	Global_InfoArr[START_INDEX_OF_CH2_INFO + 2].getValInNanoCallback =
		OSC_s64GetVmin2Info;

	/*	vMax	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 3].name, "vMax2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 3].unit, "V");
	Global_InfoArr[START_INDEX_OF_CH2_INFO + 3].getValInNanoCallback =
		OSC_s64GetVmax2Info;

	/*	vAvg	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 4].name, "vAvg2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 4].unit, "V");
	Global_InfoArr[START_INDEX_OF_CH2_INFO + 4].getValInNanoCallback =
		OSC_s64GetVavg2Info;

	/*	volts per Div.	*/
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 5].name, "vDiv2");
	strcpy(Global_InfoArr[START_INDEX_OF_CH2_INFO + 5].unit, "V");
	Global_InfoArr[START_INDEX_OF_CH2_INFO + 5].getValInNanoCallback =
		OSC_s64GetVdiv2Info;
}

void OSC_voidInitOtherInfo(void)
{
	/*	seconds per Div.	*/
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO].name, "tDiv");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO].unit, "S");
	Global_InfoArr[START_INDEX_OF_OTHER_INFO].getValInNanoCallback =
		OSC_s64GetTdivInfo;

	/*	Voltage cursors	*/
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 1].name, "v1");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 1].unit, "V");

	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 2].name, "v2");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 2].unit, "V");

	/*	Time cursors	*/
	/*strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 3].name, "t1");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 3].unit, "S");
	Global_InfoArr[START_INDEX_OF_OTHER_INFO + 3].getValInNanoCallback =
		OSC_s64GetT1Info;*/

	/*strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 4].name, "t2");
	strcpy(Global_InfoArr[START_INDEX_OF_OTHER_INFO + 4].unit, "S");
	Global_InfoArr[START_INDEX_OF_OTHER_INFO + 4].getValInNanoCallback =
		OSC_s64GetT2Info;*/
}

void OSC_voidInitInfo(void)
{
	/**	Channel 1 info	**/
	OSC_voidInitCh1Info();

	/**	Channel 2 info	**/
	OSC_voidInitCh2Info();

	/**	Other info	**/
	OSC_voidInitOtherInfo();

	/**	Disable all	**/
	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
		Global_InfoArr[i].enabled = false;

	/**	Default enable	**/
	/*	freq1	*/
	Global_InfoArr[0].enabled = true;
	/*	vpp1	*/
	Global_InfoArr[1].enabled = true;
	/*	freq2	*/
	Global_InfoArr[START_INDEX_OF_CH2_INFO].enabled = true;
	/*	vpp2	*/
	Global_InfoArr[START_INDEX_OF_CH2_INFO + 1].enabled = true;
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


