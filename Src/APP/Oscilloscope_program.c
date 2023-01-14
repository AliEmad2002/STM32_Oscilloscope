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
#include "TFT_interface_V1.h"
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_interface.h"

/*	enum that describes the different states of line drawing	*/
typedef enum{
	OSC_LineDrawingState_1,	// means that the last started operation is:
							// scroll, read ADC, set new drawing boundaries
							// and draw black segment from 0 to
							// 'OSC_smallest' - 1.

	OSC_LineDrawingState_2,	// means that the last started operation is:
							// Drawing red segment from 'OSC_smallest' to
							// 'OSC_largeest'.

	OSC_LineDrawingState_3,	// means that the last started operation is:
							// Drawing red segment from 'OSC_largeest' + 1 to
							// 127.
}OSC_LineDrawingState_t;

/*	static objects	*/
static TFT2_t LCD;

static u8 OSC_smallest = 0;		// these two variables represent smallest and
static u8 OSC_largest = 0;		// largest points in the current line's active
								// (red) segment.

static OSC_LineDrawingState_t drawingState = OSC_LineDrawingState_3;

static u8 tftScrollCounter = 0;

/*	binary semaphore for TFT interfacing line	*/
static b8 tftIsUnderUsage = false;

static NVIC_Interrupt_t tftDmaInterruptNumber = 0;

/*
 * Peak to peak value in a single frame.
 * Is calculated as the difference between the largest and smallest values in
 * current frame.
 * "current frame" starts when "tftScrollCounter" equals zero, and ends when it
 * equals "tftScrollCounterMax".
 * Unit is: [TFT screen pixels].
 */
static u8 peakToPeakValueInCurrentFrame = 0;
static u8 largestVlaueInCurrentFrame = 0;
static u8 smallestVlaueInCurrentFrame = 0;

/*	Defines based on configuration file	*/
#define ADC_1_CHANNEL		(ANALOG_INPUT_1_PIN % 16)
#define ADC_2_CHANNEL		(ANALOG_INPUT_2_PIN % 16)

/*	constant values based on experimental tests	*/
const u64 lineDrawingRatemHzMax = 10000000;

/*	ISR callbacks	*/
void OSC_voidDMATransferCompleteCallback(void);
void OSC_voidTimToStartDrawingNextLineCallback(void);
void OSC_voidTimToStartDrawingInfoCallback(void);

/*
 * Inits all (MCAL) hardware resources configured in "Oscilloscope_configh.h"
 * file.
 */
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
	SCB_voidSetPriorityGroupsAndSubGroupsNumber(SCB_PRIGROUP_group4_sub4);

	/**************************************************************************
	 * NVIC init:
	 *************************************************************************/
	NVIC_Interrupt_t timTrigLineDrawingInterrupt =
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
		timTrigInfoDrawingInterrupt, 1, 1);
}

/*
 * Inits all (HAL) hardware resources configured in "Oscilloscope_configh.h"
 * file, and static objects defined in "Oscilloscope_program.c".
 */
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
				LCD_INFO_DRAWING_TRIGGER_FREQUENCY_MILLI_HZ,
				LCD_INFO_DRAWING_TRIGGER_FREQUENCY_MILLI_HZ,
				OSC_voidTimToStartDrawingInfoCallback);
	}

}

/*	main super loop (no OS version)	*/
void OSC_voidMainSuperLoop(void)
{
	/*Delay_voidBlockingDelayMs(5000);
	TIM_u64SetFreqByChangingArr(
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, 10000);*/
	while (1)
	{
		Delay_voidBlockingDelayMs(500);
	}
}

/*
 * this function is called whenever DMA finishes a transfer to TFT, it starts
 * the next "OSC_LineDrawingState_t" operation, and clears DMA completion flags.
 *
 */
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

/*
 * this function is called to start executing what's mentioned in
 * "OSC_LineDrawingState_1" description.
 *
 * It is executed periodically as what user had configured time axis.
 *
 * minimum call rate of this function is important, as executing it in a high
 * rate would overlap drawing of multiple lines!
 * minimum call rate when F_SYS = 72MHz and using default TFT settings is about
 * 90~100 us.
 */
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

/*
 * This function is periodically called/triggered by a configured timer unit.
 * It draws signal info on the screen.
 * This function:
 *  - Prepares info image.
 * 	- Pulls TFT semaphore till released.
 * 	- Takes it.
 *	- Pauses DMA transfer complete interrupt.
 *	- Sends info image by DMA.
 *	- Waits for transfer complete.	  --|    all three are implemented in:
 *	- Clears DMA transfer complete flag.|==>"TFT2_voidWaitCurrentDataTransfer()"
 *	- Disables DMA.                   --|
 *	- Enables/resumes DMA transfer complete interrupt.
 *	- Releases TFT semaphore.
 *
 */
void OSC_voidTimToStartDrawingInfoCallback(void)
{
	/*	Prepare info image	*/
	u64 freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	u16 pixColorArr[30][128];


	/*	Pull TFT semaphore till released	*/
	while(tftIsUnderUsage == true);

	/*	Take it	*/
	tftIsUnderUsage = true;

	/*	Pause DMA transfer complete interrupt	*/
	NVIC_voidDisableInterrupt(tftDmaInterruptNumber);

	/*	Send info image by DMA	*/
	TFT2_voidSendPixels(&LCD, (u16*)pixColorArr, 30 * 128);

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
}




































