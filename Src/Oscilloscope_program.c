/*
 * Oscilloscope_program.c
 *
 *  Created on: Nov 25, 2022
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

/*	HAL	*/
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_init_MCAL.h"
#include "Oscilloscope_init_HAL.h"
#include "Oscilloscope_interface.h"


/*******************************************************************************
 * Mode switching:
 ******************************************************************************/
void OSC_voidEnterMenuMode(void)
{

}

void OSC_voidEnterMathMode(void)
{
	// TODO: as memory is never enough, let the menu take 1/3 of the screen only
}

void OSC_voidTrigPauseResume(void)
{
	/*	static flag	*/
	static b8 paused = false;

	if (paused)
	{
		// resume:
		/*	stop line drawing trigger counter	*/
		TIM_voidDisableCounter(LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

		/*	stop info drawing trigger counter	*/
		TIM_voidDisableCounter(LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);

		/*	update flag	*/
		paused = false;
	}

	else
	{
		/*	start line drawing trigger counter	*/
		TIM_voidEnableCounter(LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

		/*	start info drawing trigger counter	*/
		TIM_voidEnableCounter(LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);

		/*	update flag	*/
		paused = true;
	}
}

/*******************************************************************************
 * Main thrad functions:
 ******************************************************************************/
void OSC_voidMainSuperLoop(void)
{
	/*Delay_voidBlockingDelayMs(5000);
	TIM_u64SetFreqByChangingArr(
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, 10000);*/
	while (1)
	{
		if (isInfoPixArrPrepared == false)
			OSC_voidPrepareInfoPixArray();
	}
}

void OSC_voidPrepareInfoPixArray(void)
{
	u64 freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	u8 str[20];
	if (freqmHz < 1000)
		sprintf((char*)str, "F = %u mHz", (u32)freqmHz);
	else if (freqmHz < 1000000)
		sprintf((char*)str, "F = %u Hz", (u32)(freqmHz / 1000));
	else if (freqmHz < 1000000000)
		sprintf((char*)str, "F = %u KHz", (u32)(freqmHz / 1000000));
	else if (freqmHz < 1000000000000)
		sprintf((char*)str, "F = %u MHz", (u32)(freqmHz / 1000000000));
	else
		sprintf((char*)str, "F = 0 Hz");

	Txt_voidCpyStrToStaticPixArr(
			str, colorRed.code565, colorBlack.code565, 1,
			Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
			0, 0, 30, 128, infoPixArr);

	/*	raise prepared flag	*/
	isInfoPixArrPrepared = true;
}

void OSC_voidAutoCalibVoltAndTimePerDiv(void)
{

}

/*******************************************************************************
 * ISR callbacks:
 ******************************************************************************/
void OSC_voidDMATransferCompleteCallback(void)
{
	/*
	 * the word 'state' in the following comments is defined in description of
	 * "OSC_LineDrawingState_t" enum definition)
	 */
	switch(drawingState)
	{
	case OSC_LineDrawingState_1: // case the just ended operation is of state 1.
		/*	start state 2	*/
		TFT2_voidFillDMA(&LCD, &colorRedU8Val, OSC_largest - OSC_smallest + 1);
		/*	update state	*/
		drawingState = OSC_LineDrawingState_2;
	break;

	case OSC_LineDrawingState_2: // case the just ended operation is of state 2.
		/*	start state 3	*/
		TFT2_voidFillDMA(&LCD, &colorBlackU8Val, 125 - OSC_largest);
		/*	update state	*/
		drawingState = OSC_LineDrawingState_3;
	break;

	case OSC_LineDrawingState_3: // case the just ended operation is of state 3.
		TFT2_voidClearDMATCFlag(&LCD);
		TFT2_voidDisableDMAChannel(&LCD);
		/*	scroll TFT display	*/
		TFT2_voidScroll(&LCD, tftScrollCounter);
		/*	release semaphore	*/
		tftIsUnderUsage = false;
	break;
	}
}

void OSC_voidTimToStartDrawingNextLineCallback(void)
{
	static u8 lastRead = 0;

	/*
	 * check that previous line drawing is done. otherwise execute errorHandler.
	 */
	if (tftIsUnderUsage == true)
	{
		ErrorHandler_voidExecute(9);
		return;
	}

	/*	take semaphore	*/
	tftIsUnderUsage = true;

	/*	update state	*/
	drawingState = OSC_LineDrawingState_1;

	/*	read ADC	*/
	u8 adcRead = 125 -
		(u8)(((u32)ADC_u16GetDataRegular(ADC_UnitNumber_1)) * 125u / 4095u);

	if (adcRead > lastRead)
	{
		OSC_largest = adcRead;
		OSC_smallest = lastRead;
	}
	else
	{
		OSC_largest = lastRead;
		OSC_smallest = adcRead;
	}

	TFT2_SET_Y_BOUNDARIES(&LCD, tftScrollCounter, tftScrollCounter);

	TFT2_WRITE_CMD(&LCD, TFT_CMD_MEM_WRITE);

	TFT2_ENTER_DATA_MODE(&LCD);
									// i.e.: OSC_smallest - 1 shifted up by 2
	TFT2_voidFillDMA(&LCD, &colorBlackU8Val, OSC_smallest + 1);

	/*	update peak to peak calculation parameters	*/
	if (adcRead > largestVlaueInCurrentFrame)
		largestVlaueInCurrentFrame = adcRead;
	if (adcRead < smallestVlaueInCurrentFrame)
		smallestVlaueInCurrentFrame = adcRead;
	peakToPeakValueInCurrentFrame =
		largestVlaueInCurrentFrame - smallestVlaueInCurrentFrame;

	/*	iteration control	*/
	tftScrollCounter++;
	if (tftScrollCounter > tftScrollCounterMax)
	{
		tftScrollCounter = 0;
		/*
		 * for every new current frame, "largestVlaueInCurrentFrame" and
		 * "smallestVlaueInCurrentFrame" are both equal to the very last reading
		 * value of the just ended frame.
		 * (see definition of "current frame" in description of
		 * "static u8 peakToPeakValueInCurrentFrame" above in this file)
		 */
		largestVlaueInCurrentFrame = adcRead;
		smallestVlaueInCurrentFrame = adcRead;

	}
	lastRead = adcRead;

	TIM_voidClearStatusFlag(
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, TIM_Status_Update);
}

void OSC_voidTimToStartDrawingInfoCallback(void)
{
	/*
	 * if TFT is under usage or infoPixArr is not yet ready, return. As showing
	 * info is not so critical, and I don't want it to affect the signal drawing
	 */
	if (tftIsUnderUsage == true || isInfoPixArrPrepared == false)
	{
		TIM_voidClearStatusFlag(
			LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER, TIM_Status_Update);
		return;
	}

	/*	Take TFT semaphore	*/
	tftIsUnderUsage = true;

	/*	Pause DMA transfer complete interrupt	*/
	NVIC_voidDisableInterrupt(tftDmaInterruptNumber);

	/*	Set info image boundaries on TFT	*/
	TFT2_SET_X_BOUNDARIES(&LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&LCD, 130, 159);

	/*	Send info image by DMA	*/
	TFT2_voidSendPixels(&LCD, (u16*)infoPixArr, 128 * 30);

	/*
	 * Wait for transfer complete,
	 * Then clear DMA transfer complete flag,
	 * Then disable DMA.
	 */
	TFT2_voidWaitCurrentDataTransfer(&LCD);

	/*	Enable/resume DMA transfer complete interrupt	*/
	NVIC_voidEnableInterrupt(tftDmaInterruptNumber);

	/*	Releases TFT semaphore	*/
	tftIsUnderUsage = false;

	/*	clear infoPixArr ready flag	*/
	isInfoPixArrPrepared = false;

	TIM_voidClearStatusFlag(
		LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER, TIM_Status_Update);
}




































