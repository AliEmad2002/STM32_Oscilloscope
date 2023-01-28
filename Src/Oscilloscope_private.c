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



/*	TFT LCD object	*/
volatile TFT2_t Global_LCD;

/*
 * Peak to peak value in a single frame.
 * Is calculated as the difference between the largest and smallest values in
 * current frame.
 * "current frame" starts when "tftScrollCounter" equals zero, and ends when it
 * equals "tftScrollCounterMax".
 * Unit is: [TFT screen pixels].
 */
volatile u8 Global_PeakToPeakValueInCurrentFrame;
volatile u8 Global_LargestVlaueInCurrentFrame;
volatile u8 Global_SmallestVlaueInCurrentFrame;

/*	current resolution values	*/
volatile u32 Global_CurrentMicroVoltsPerPix;
volatile u64 Global_CurrentNanoSecondsPerPix;
volatile u8 Global_CurrentUsedAdcChannelIndex;

/*	pausing of display	*/
volatile b8 Global_Paused;

volatile u64 Global_LastMeasuredFreq;

volatile OSC_Up_Down_Target_t Global_UpDownTarget;

volatile b8 Global_ReturnedFromMenu;

