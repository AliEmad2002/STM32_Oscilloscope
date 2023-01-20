/*
 * Oscilloscope_init_APP.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Debug_active.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Delay_interface.h"

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
#include "Oscilloscope_init_APP.h"

extern u64 currentMicroSecondsPerPix;
extern u64 currentMicroVoltsPerPix;
extern b8 tftIsUnderUsage;
extern u8 tftScrollCounter;
extern u8 tftScrollCounterMax;
extern ADC_ChannelNumber_t currentUsedAdcChannel;
extern b8 paused;
extern b8 enter;
extern u8 peakToPeakValueInCurrentFrame;
extern OSC_LineDrawingState_t drawingState;
extern b8 isInfoPixArrPrepared;
extern u8 largestVlaueInCurrentFrame;
extern u8 smallestVlaueInCurrentFrame;
extern u8 OSC_largest;
extern u8 OSC_smallest;
extern OSC_RunningState_t runningState;
extern NVIC_Interrupt_t tftDmaInterruptNumber;
extern NVIC_Interrupt_t timTrigLineDrawingInterrupt;

void OSC_voidInitApp(void)
{
	OSC_smallest = 0;
	OSC_largest = 0;

	drawingState = OSC_LineDrawingState_3;

	tftScrollCounter = 0;

	tftIsUnderUsage = false;

	tftDmaInterruptNumber = 0;

	peakToPeakValueInCurrentFrame = 0;
	largestVlaueInCurrentFrame = 0;
	smallestVlaueInCurrentFrame = 0;

	timTrigLineDrawingInterrupt = 0;

	isInfoPixArrPrepared = false;

	enter = false;

	currentMicroVoltsPerPix = 26000;
	currentMicroSecondsPerPix = 100;

	currentUsedAdcChannel = 0;

	tftScrollCounterMax = 128;

	runningState = OSC_RunningState_NormalMode;

	paused = false;
}













