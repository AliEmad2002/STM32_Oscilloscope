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

extern TFT2_t OSC_LCD;
extern u32 currentMicroSecondsPerPix;
extern u32 currentMicroVoltsPerPix;
extern b8 tftIsUnderUsage;
extern u8 tftScrollCounter;
extern u8 tftScrollCounterMax;
extern u8 currentUsedAdcChannel;
extern b8 paused;
extern u8 peakToPeakValueInCurrentFrame;
extern OSC_LineDrawingState_t drawingState;
extern u16 infoPixArr[1][1];
extern b8 isInfoPixArrPrepared;
extern u8 largestVlaueInCurrentFrame;
extern u8 smallestVlaueInCurrentFrame;
extern u8 OSC_largest;
extern u8 OSC_smallest;
extern OSC_RunningState_t runningState;
extern NVIC_Interrupt_t tftDmaInterruptNumber;
extern u64 lineDrawingRatemHzMin;





/*******************************************************************************
 * Mode switching:
 ******************************************************************************/
void OSC_voidStartNormalMode(void)
{
	OSC_voidStartSignalDrawing();

	OSC_voidStartInfoDrawing();
}

void OSC_voidEnterNormalMode(void)
{
	OSC_voidResume();
}

void OSC_voidEnterMathMode(void)
{
	// TODO: as memory is never enough, let the menu take 1/3 of the screen only
}

void OSC_voidPause(void)
{
	if (runningState == OSC_RunningState_NormalMode)
	{
		/*	stop line drawing trigger counter	*/
		TIM_voidDisableCounter(LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

		/*	stop info drawing trigger counter	*/
		TIM_voidDisableCounter(LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);
	}

	/*	update flag	*/
	paused = true;
}

void OSC_voidResume(void)
{
	if (runningState == OSC_RunningState_NormalMode)
	{
		/*	start line drawing trigger counter	*/
		TIM_voidEnableCounter(LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

		/*	start info drawing trigger counter	*/
		TIM_voidEnableCounter(LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);
	}

	/*	update flag	*/
	paused = false;
}

void OSC_voidTrigPauseResume(void)
{
	if (!paused)
	{
		OSC_voidPause();
	}

	else
	{
		OSC_voidResume();
	}
}

/*******************************************************************************
 * Main thread functions:
 ******************************************************************************/
extern void OSC_voidOpenMenu(void);

void OSC_voidMainSuperLoop(void)
{
	//OSC_voidOpenMenu();

	//while(1);

	/*	start normal (Y-t) mode	*/
	OSC_voidStartNormalMode();

	OSC_voidPause();

	/*	Auto calibrate at startup	*/
	OSC_voidAutoCalibVoltAndTimePerDiv();

	OSC_voidResume();

	while (1)
	{
		if (paused)
			continue;

		if (runningState == OSC_RunningState_NormalMode)
		{
			if (isInfoPixArrPrepared == false)
				OSC_voidPrepareInfoPixArray();
		}
	}
}

void OSC_voidPrepareInfoPixArray(void)
{
	u64 freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	char hzPre;

	u8 str[128];
	if (freqmHz < 1000)
		hzPre = 'm';
	else if (freqmHz < 1000000)
	{
		freqmHz /= 1000;
		hzPre = ' ';
	}
	else if (freqmHz < 1000000000)
	{
		freqmHz /= 1000000;
		hzPre = 'k';
	}
	else if (freqmHz < 1000000000000)
	{
		freqmHz /= 1000000000;
		hzPre = 'M';
	}
	else
	{
		freqmHz = 0;
		hzPre = ' ';
	}

	u64 vPP = peakToPeakValueInCurrentFrame * currentMicroVoltsPerPix;

	char voltPre;

	u32 vPPInt, vPPFrac;

	if (vPP < 1000)
	{
		voltPre = 'u';
		vPPInt = vPP;
		vPPFrac = 0;
	}
	else if (vPP < 1000000)
	{
		voltPre = 'm';
		vPPInt = vPP / 1000;
		vPPFrac = vPP % 1000;
	}
	else
	{
		voltPre = ' ';
		vPPInt = vPP / 1000000;
		vPPFrac = vPP % 1000000;
	}

	while (vPPFrac > 100)
		vPPFrac /= 10;

	sprintf(
		(char*)str, "F = %u%cHz\nVpp = %u.%u%cV",
		(u32)freqmHz, hzPre, vPPInt, vPPFrac, voltPre);

	Txt_voidCpyStrToStaticPixArrNormalOrientation(
			str, colorRed.code565, colorBlack.code565, 1,
			Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
			0, 0, 30, 128, infoPixArr);

	/*	raise prepared flag	*/
	isInfoPixArrPrepared = true;
}

void OSC_voidAutoCalibVoltAndTimePerDiv(void)
{
	/*	get signal frequency	*/
	u64 freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);
	if (freqmHz > 1000000000)
		freqmHz = 0;

	/*
	 * set time per pix such that user can see 3 periods of the signal in one
	 frame	*/
	currentMicroSecondsPerPix = 1000000000ul / freqmHz / tftScrollCounterMax;

	u64 lineDrawingRatemHz = tftScrollCounterMax *  freqmHz;

	if (lineDrawingRatemHz > lineDrawingRatemHzMax)
		lineDrawingRatemHz = lineDrawingRatemHzMax;
	else if (lineDrawingRatemHz < lineDrawingRatemHzMin)
		lineDrawingRatemHz = (lineDrawingRatemHzMax) / 1;

	TIM_u64SetFreqByChangingArr(
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, lineDrawingRatemHz);

	/**	find the proper gain to display	**/
	for (u8 i = 0; i < CHANNEL_1_NUMBER_OF_LEVELS; i++)
	{
		ADC_ChannelNumber_t adcCh = oscCh1AdcChannels[i].adcChannelNumber;
		/*	make adcCh the one to be converted and watchdog-ed	*/
		ADC_voidSetSequenceRegular(
			ADC_UnitNumber_1, ADC_RegularSequenceNumber_1, adcCh);

		/*	Clear AED flag, in case previous rise	*/
		ADC_voidClearStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_AWD);

		/*	wait for 2 periods of this signal, at least 1 ms	*/
		u32 delayTimeMs = 2000000u / freqmHz;
		if (delayTimeMs == 0 || delayTimeMs > 1)
			delayTimeMs = 1;

		Delay_voidBlockingDelayMs(delayTimeMs);

		/**	check analog watchdog flag	**/
		/*
		 * if it was set, then this gain is not the proper one, clear AWD flag
		 */
		if (ADC_b8GetStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_AWD) == true)
		{
			ADC_voidClearStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_AWD);
		}
		/*	otherwise, use this gain	*/
		else
		{
			currentMicroVoltsPerPix =
				(oscCh1AdcChannels[i].maxVPPinMilliVolts * 1000ul) / 128;

			currentUsedAdcChannel = i;
		}
	}
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
		TFT2_voidFillDMA(&OSC_LCD, &colorRedU8Val, OSC_largest - OSC_smallest + 1);
		/*	update state	*/
		drawingState = OSC_LineDrawingState_2;
	break;

	case OSC_LineDrawingState_2: // case the just ended operation is of state 2.
		/*	start state 3	*/
		TFT2_voidFillDMA(&OSC_LCD, &colorBlackU8Val, 125 - OSC_largest);
		/*	update state	*/
		drawingState = OSC_LineDrawingState_3;
	break;

	case OSC_LineDrawingState_3: // case the just ended operation is of state 3.
		TFT2_voidClearDMATCFlag(&OSC_LCD);
		TFT2_voidDisableDMAChannel(&OSC_LCD);
		/*	scroll TFT display	*/
		TFT2_voidScroll(&OSC_LCD, tftScrollCounter);
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
		//ErrorHandler_voidExecute(9);
		trace_printf("dsa\n");
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

	TFT2_SET_Y_BOUNDARIES(&OSC_LCD, tftScrollCounter, tftScrollCounter);

	TFT2_WRITE_CMD(&OSC_LCD, TFT_CMD_MEM_WRITE);

	TFT2_ENTER_DATA_MODE(&OSC_LCD);
									// i.e.: OSC_smallest - 1 shifted up by 2
	TFT2_voidFillDMA(&OSC_LCD, &colorBlackU8Val, OSC_smallest + 1);

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
	TFT2_SET_X_BOUNDARIES(&OSC_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&OSC_LCD, 130, 159);

	/*	Send info image by DMA	*/
	TFT2_voidSendPixels(&OSC_LCD, (u16*)infoPixArr, 128 * 30);

	/*
	 * Wait for transfer complete,
	 * Then clear DMA transfer complete flag,
	 * Then disable DMA.
	 */
	TFT2_voidWaitCurrentDataTransfer(&OSC_LCD);

	/*	Enable/resume DMA transfer complete interrupt	*/
	NVIC_voidEnableInterrupt(tftDmaInterruptNumber);

	/*	Releases TFT semaphore	*/
	tftIsUnderUsage = false;

	/*	clear infoPixArr ready flag	*/
	isInfoPixArrPrepared = false;

	TIM_voidClearStatusFlag(
		LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER, TIM_Status_Update);
}




































