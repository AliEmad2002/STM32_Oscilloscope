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
#include "DMA_interface.h"
#include "SCB_interface.h"
#include "NVIC_interface.h"
#include "RCC_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "STK_interface.h"
#include "ADC_private.h"
#include "ADC_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_interface.h"

/*******************************************************************************
 * Init functions:
 ******************************************************************************/
void OSC_voidInitMCAL(void)
{
	/**************************************************************************
	 * RCC init:
	 *************************************************************************/
	RCC_voidSysClockInit();

	/*	GPIO/AFIO	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPA);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPB);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_AFIO);

	/*	ADC	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC1);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC2);
	RCC_voidSetAdcPrescaler(RCC_ADC_Prescaler_PCLK2_by6);

	/**************************************************************************
	 * SysTick init: (used for time-stamping)
	 *************************************************************************/
	STK_voidInit();
	STK_voidStartTickMeasure(STK_TickMeasureType_OverflowCount);
	STK_voidEnableSysTick();

	/**************************************************************************
	 * GPIO init:
	 *************************************************************************/
	GPIO_voidSetPinGpoPushPull(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);
	GPIO_voidSetPinMode(
		ANALOG_INPUT_1_PIN / 16, ANALOG_INPUT_1_PIN % 16,
		GPIO_Mode_Input_Analog);
	GPIO_voidSetPinMode(
		ANALOG_INPUT_2_PIN / 16, ANALOG_INPUT_2_PIN % 16,
		GPIO_Mode_Input_Analog);
	GPIO_voidSetPinInputPullDown(BUTTON_1_PIN / 16, BUTTON_1_PIN % 16);

	/**************************************************************************
	 * ADC init:
	 *************************************************************************/
	// enable continuous mode:
	ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_1);
	// set channel sample time:
	ADC_voidSetSampleTime(
		ADC_UnitNumber_1, ADC_1_CHANNEL, ADC_SAMPLE_TIME);
	// write channel in regular sequence:
	ADC_voidSetSequenceRegular(
		ADC_UnitNumber_1, ADC_RegularSequenceNumber_1, ADC_1_CHANNEL);
	// set regular sequence len to 1, as only one channel is to be converted:
	ADC_voidSetSequenceLenRegular(ADC_UnitNumber_1, 1);
	// set regular channels trigger to SWSTART:
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_SWSTART);
	// enable conversion on external event:
	ADC_voidEnableExternalTriggerRegular(ADC_UnitNumber_1);
	// power on:
	ADC_voidEnablePower(ADC_UnitNumber_1);
	// calibrate:
	ADC_voidStartCalibration(ADC_UnitNumber_1);
	ADC_voidWaitCalibration(ADC_UnitNumber_1);
	// trigger start of conversion:
	ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);

	/**************************************************************************
	 * TIM init:
	 *************************************************************************/
	/*	start frequency measurement	*/
	TIM_voidInitFreqAndDutyMeasurement(
		FREQ_MEASURE_TIMER_UNIT_NUMBER, FREQ_MEASURE_TIMER_UNIT_AFIO_MAP, 100);

	/**************************************************************************
	 * SCB init:
	 *************************************************************************/
	/*	configure number of NVIC groups and sub groups	*/
	//SCB_voidSetPriorityGroupsAndSubGroupsNumber(SCB_PRIGROUP_group16_sub0);
	//while(1);
	/**************************************************************************
	 * NVIC init:
	 *************************************************************************/
	timTrigLineDrawingInterrupt =
			TIM_u8GetUpdateEventInterruptNumber(
				LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

	NVIC_Interrupt_t timTrigInfoDrawingInterrupt =
			TIM_u8GetUpdateEventInterruptNumber(
				LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);

	tftDmaInterruptNumber = DMA_u8GetInterruptVectorIndex(
		DMA_UnitNumber_1,
		(LCD_SPI_UNIT_NUMBER == SPI_UnitNumber_1 ?
			DMA_ChannelNumber_3 : DMA_ChannelNumber_5));

	NVIC_voidEnableInterrupt(timTrigLineDrawingInterrupt);
	NVIC_voidEnableInterrupt(timTrigInfoDrawingInterrupt);

	NVIC_voidSetInterruptPriority(
		tftDmaInterruptNumber, 0, 0);

	NVIC_voidSetInterruptPriority(
		timTrigLineDrawingInterrupt, 1, 0);

	NVIC_voidSetInterruptPriority(
		timTrigInfoDrawingInterrupt, 1, 0);
}

void OSC_voidInitHAL(void)
{
	/**************************************************************************
	 * LCD init:
	 *************************************************************************/
	TFT2_voidInit(
		&LCD, LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, LCD_RST_PIN, LCD_A0_PIN,
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	/*	set maximum brightness by default	*/
	TFT2_voidSetBrightness(&LCD, POW_TWO(13));

	/*	display startup screen	*/
	TFT2_SET_X_BOUNDARIES(&LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&LCD, 0, 159);

	TFT2_WRITE_CMD(&LCD, TFT_CMD_MEM_WRITE);

	TFT2_ENTER_DATA_MODE(&LCD);

	TFT2_voidFillDMA(&LCD, &colorBlackU8Val, 128 * 160);
	TFT2_voidWaitCurrentDataTransfer(&LCD);
	TFT2_voidClearDMATCFlag(&LCD);
	TFT2_voidDisableDMAChannel(&LCD);

	/*	enable interrupt (to be used for less drawing overhead)	*/
	TFT2_voidSetDMATransferCompleteCallback(
		&LCD, OSC_voidDMATransferCompleteCallback);

	TFT2_voidEnableDMATransferCompleteInterrupt(&LCD);

	/*	give user chance to see startup screen	*/
	//Delay_voidBlockingDelayMs(1500);

	/*
	 * split display to two parts, large one for displaying signal (0-130),
	 * and small one for signal data (131-160). (based on saved user settings).
	 *
	 * The split can be cancelled from settings.
	 */
	TFT2_voidInitScroll(&LCD, 0, 130, 32);

	/*	start drawing	*/
	lineDrawingRatemHzMin = TIM_u64InitTimTrigger(
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, lineDrawingRatemHzMax / 100,
		lineDrawingRatemHzMax, OSC_voidTimToStartDrawingNextLineCallback);

	/*
	 * enable info drawing (frequency, peak to peak value, etc..)
	 * (only if info section was turned on by default or by user settings).
	 */
	if (tftScrollCounterMax == 128)	//	if info section is turned on
	{
		(void)TIM_u64InitTimTrigger(
				LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER,
				1600,
				1500000, // a value that ensures a possible rate of 2Hz
				OSC_voidTimToStartDrawingInfoCallback);
	}
}

/*******************************************************************************
 * Mode switching:
 ******************************************************************************/
void OSC_voidEnterMenuMode(void);

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




































