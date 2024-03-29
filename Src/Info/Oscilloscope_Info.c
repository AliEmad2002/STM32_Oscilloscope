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
#include "Img_interface.h"
#include "Colors.h"
#include "Txt_interface.h"
#include <stdio.h>
#include "LinkedList.h"
#include "MathParser.h"

/*	MCAL	*/
#include "RCC_interface.h"
#include "DMA_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "STK_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_Private.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Info.h"

/*	global info array	*/
volatile OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

/**	Drawing functions	**/
void OSC_voidDrawInfo(void)
{
	/*	counter of how many info's were drawn in this call	*/
	u8 drawnInfoCount = 0;

	for (u8 i = 0; i < NUMBER_OF_INFO && drawnInfoCount < 4; i++)
	{
		if (Global_InfoArr[i].enabled == false)
			continue;

		/*	get the string to print	*/
		OSC_voidGetInfoStringToPrint((char*)Global_Str, i);

		/*	wait for previous img data transfer complete	*/
		TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);

		/*	draw the string on img buffer	*/
		Txt_voidPrintStrOnPixArrRightOrientation(
			(char*)Global_Str, LCD_MAIN_DRAWING_COLOR_U16,
			LCD_BACKGROUND_COLOR_U16,
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
			(TFT2_t*)&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex], 80 * 8);

		/*	increment drawn info counter	*/
		drawnInfoCount++;
	}

	/*	wait for last img data transfer complete	*/
	TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);

	/*	set TFT boundaries back to the signal drawing area	*/
	OSC_voidSetDisplayBoundariesForSignalArea();
}

void OSC_voidGetNumberPrintableVersion(
	s64 valInNano, s32* valInteger, u32* valFraction, char* unitPrefix)
{
	u64 valInNanoAbs = (valInNano < 0) ? -valInNano : valInNano;

	if (valInNanoAbs < (u64)1e3)
	{
		*unitPrefix = 'n';
		*valInteger = valInNano;
		*valFraction = 0;
	}
	else if (valInNanoAbs < (u64)1e6)
	{
		*unitPrefix = 'u';
		*valInteger = valInNano / 1e3;
		*valFraction = (valInNanoAbs % (u64)1e3) / (u64)1e2;
	}
	else if (valInNanoAbs < (u64)1e9)
	{
		*unitPrefix = 'm';
		*valInteger = valInNano / (u64)1e6;
		*valFraction = (valInNanoAbs % (u64)1e6) / (u64)1e5;
	}
	else if (valInNanoAbs < (u64)1e12)
	{
		*unitPrefix = ' ';
		*valInteger = valInNano / (u64)1e9;
		*valFraction = (valInNano % (u64)1e9) / (u64)1e8;
	}
	else if (valInNanoAbs < (u64)1e15)
	{
		*unitPrefix = 'k';
		*valInteger = valInNano / (u64)1e12;
		*valFraction = (valInNano % (u64)1e12) / (u64)1e13;
	}
	else if (valInNanoAbs < (u64)1e18)
	{
		*unitPrefix = 'M';
		*valInteger = valInNano / (u64)1e15;
		*valFraction = (valInNano % (u64)1e15) / (u64)1e14;
	}

	/*	fraction is maximumly of 1 digit	*/
	while (*valFraction > 10)
		*valFraction /= 10;
}

void OSC_voidGetInfoStringToPrint(char* str, u8 infoIndex)
{
	/*	get info current val in nano	*/
	s64 valInNano = Global_InfoArr[infoIndex].getValInNanoCallback();

	/*	get the printable version of that value	*/
	s32 valInteger;
	u32 valFraction;
	char unitPrefix;
	OSC_voidGetNumberPrintableVersion(
		valInNano, &valInteger, &valFraction, &unitPrefix);

	/*	copy data to string	*/
	sprintf(
		str, "%s=%d.%u%c%s",
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

/**	Hide, show functions	**/
/*	Ch1	*/
void OSC_voidShowCh1Info(void)
{

}
/*	Ch2	*/
void OSC_voidShowCh2Info(void)
{

}

/**	Enable, disable functions	**/
/*	disables all enabled info's	*/
void OSC_voidDisableAllInfo(void)
{
	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
	{
		Global_InfoArr[i].enabled = false;
	}
}

/*	enables info of certain index in infoArr	*/
void OSC_voidEnableInfo(u8 i)
{
	/*	range check 'i'	*/
	if (i >= NUMBER_OF_INFO)
		return;

	/*	if passed the check, enable	*/
	Global_InfoArr[i].enabled = true;
}

/*	disables info of certain index in infoArr	*/
void OSC_voidDisableInfo(u8 i)
{
	/*	range check 'i'	*/
	if (i >= NUMBER_OF_INFO)
		return;

	/*	if passed the check, disable	*/
	Global_InfoArr[i].enabled = false;
}

/*
 * gets indexes of the first four enabled info's, grouped in 32-bit single
 * variable.
 */
u32 OSC_u32GetEnabledInfo(void)
{
	u32 enabled = 0;

	u8 enabledCount = 0;

	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
	{
		if (Global_InfoArr[i].enabled)
		{
			enabled |= (i << (enabledCount * 8));

			enabledCount++;
		}
	}

	return enabled;
}

/**	Callbacks	**/
/*	freq1	*/
s64 OSC_s64GetFreq1Info(void)
{
	static u64 lastMeasureTimestamp = 0;

	/*
	 * store last measured frequency. saved in case user pauses, so the shown
	 * is the same before pausing.
	 */
	static u64 lastFreqMeasureBeforePause = 0;

	/*
	 * if paused, frequency does not change, it is the last saved before pausing
	 */
	if (Global_Paused)
	{	}

	/*	otherwise	*/
	/*	check if a rising edge occurred	*/
	else if (
		TIM_GET_STATUS_FLAG(
			FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER, TIM_Status_CC1)
	)
	{
		/*	if so, update frequency measurement	*/
		lastFreqMeasureBeforePause =
				TIM_u64GetFrequencyMeasured(FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER);

		lastMeasureTimestamp = STK_u64GetElapsedTicks();
	}

	/*
	 * otherwise, check rising edge time out. If it has not occurred in that
	 * time period, frequency is changed to zero.
	 */
	else if (
		STK_u64GetElapsedTicks() - lastMeasureTimestamp >
		FREQ_MEASURE_TIMEOUT_MS * STK_TICKS_PER_MS
	)
	{
		lastFreqMeasureBeforePause = 0;
	}

	/*
	 * otherwise, if a rising edge has not occurred, but timeout has not yet
	 * passed, last measured frequency is kept unchanged.
	 */
	else
	{	}

	/*	return frequency in nano-Hz	*/
	return lastFreqMeasureBeforePause * 1e6;
}

/*	freq2	*/
s64 OSC_s64GetFreq2Info(void)
{
	static u64 lastMeasureTimestamp = 0;

	/*
	 * store last measured frequency. saved in case user pauses, so the shown
	 * is the same before pausing.
	 */
	static u64 lastFreqMeasureBeforePause = 0;

	/*
	 * if paused, frequency does not change, it is the last saved before pausing
	 */
	if (Global_Paused)
	{	}

	/*	otherwise	*/
	/*	check if a rising edge occurred	*/
	else if (
		TIM_GET_STATUS_FLAG(
			FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER, TIM_Status_CC1)
	)
	{
		/*	if so, update frequency measurement	*/
		lastFreqMeasureBeforePause =
				TIM_u64GetFrequencyMeasured(FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER);

		lastMeasureTimestamp = STK_u64GetElapsedTicks();
	}

	/*
	 * otherwise, check rising edge time out. If it has not occurred in that
	 * time period, frequency is changed to zero.
	 */
	else if (
		STK_u64GetElapsedTicks() - lastMeasureTimestamp >
		FREQ_MEASURE_TIMEOUT_MS * STK_TICKS_PER_MS
	)
	{
		lastFreqMeasureBeforePause = 0;
	}

	/*
	 * otherwise, if a rising edge has not occurred, but timeout has not yet
	 * passed, last measured frequency is kept unchanged.
	 */
	else
	{	}

	/*	return frequency in nano-Hz	*/
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
	volatile s32 v1 = (s16)Global_Ch1MinValueInCurrentFrame - SIGNAL_LINE_LENGTH / 2;

	volatile s32 v2 = v1 * Global_CurrentCh1MicroVoltsPerPix;

	volatile s32 v3 = v2 - Global_Offset1MicroVolts;

	volatile s64 v4 = v3 * 1e3;

	return v4;
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
	s64 avgPix = (s32)Global_Ch1SumOfCurrentFrame / NUMBER_OF_SAMPLES;
	avgPix -= SIGNAL_LINE_LENGTH / 2;

	s64 avgMicroVolts = avgPix * Global_CurrentCh1MicroVoltsPerPix;
	avgMicroVolts -= Global_Offset1MicroVolts;

	return avgMicroVolts * 1e3;
}

/*	vAvg2	*/
s64 OSC_s64GetVavg2Info(void)
{
	s64 avgPix = (s32)Global_Ch2SumOfCurrentFrame / NUMBER_OF_SAMPLES;
	avgPix -= SIGNAL_LINE_LENGTH / 2;

	s64 avgMicroVolts = avgPix * Global_CurrentCh2MicroVoltsPerPix;
	avgMicroVolts -= Global_Offset2MicroVolts;

	return avgMicroVolts * 1e3;
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

/*	tOn1	*/
s64 OSC_s64GetTon1Info(void)
{
	/*	if frequency equals zero, there's no active time	*/
	if (OSC_s64GetFreq1Info() == 0)
		return 0;

	/*	otherwise, calculate it	*/
	return TIM_u16GetActiveTimeNanoSecond(FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER);
}

/*	tOn2	*/
s64 OSC_s64GetTon2Info(void)
{
	/*	if frequency equals zero, there's no active time	*/
	if (OSC_s64GetFreq2Info() == 0)
		return 0;

	/*	otherwise, calculate it	*/
	return TIM_u16GetActiveTimeNanoSecond(FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER);
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
