/*
 * Oscilloscope_private.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"

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



/*	static objects	*/
TFT2_t OSC_LCD;

u8 OSC_smallest;		// these two variables represent smallest and
u8 OSC_largest;			// largest points in the current line's active
						// (red) segment.

OSC_LineDrawingState_t drawingState;

u8 tftScrollCounter;

/*	binary semaphore for TFT interfacing line	*/
b8 tftIsUnderUsage;

NVIC_Interrupt_t tftDmaInterruptNumber;

/*
 * Peak to peak value in a single frame.
 * Is calculated as the difference between the largest and smallest values in
 * current frame.
 * "current frame" starts when "tftScrollCounter" equals zero, and ends when it
 * equals "tftScrollCounterMax".
 * Unit is: [TFT screen pixels].
 */
u8 peakToPeakValueInCurrentFrame;
u8 largestVlaueInCurrentFrame;
u8 smallestVlaueInCurrentFrame;

NVIC_Interrupt_t timTrigLineDrawingInterrupt;

u16 infoPixArr[30][128];
b8 isInfoPixArrPrepared;

/*	state of enter button	*/
b8 enter;

u32 currentMicroVoltsPerPix;
u32 currentMicroSecondsPerPix;

u8 currentUsedAdcChannel;

/*******************************************************************************
 * run time changeable settings:
 ******************************************************************************/
u64 lineDrawingRatemHzMin;

u8 tftScrollCounterMax;



OSC_RunningState_t runningState;

b8 paused;






extern void OSC_voidTimToStartDrawingNextLineCallback(void);

extern void OSC_voidTimToStartDrawingInfoCallback(void);

void OSC_voidStartSignalDrawing(void)
{
	/*	start drawing	*/
	lineDrawingRatemHzMin = TIM_u64InitTimTrigger(
		// TODO: set this '100' to a configurable startup value
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, lineDrawingRatemHzMax / 100,
		lineDrawingRatemHzMax, OSC_voidTimToStartDrawingNextLineCallback);
}

void OSC_voidStartInfoDrawing(void)
{
	(void)TIM_u64InitTimTrigger(
		LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER,
		// TODO: set these numbers to a configurable startup values
		1600,
		1500000, // a value that ensures a possible rate of 2Hz
		OSC_voidTimToStartDrawingInfoCallback);
}

