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
TFT2_t Global_LCD;

/*	array of pixels in which 1/4 of the full display is stored	*/
u16 Global_QuarterOfTheDisplay[40][128];

/*	binary semaphore for LCD interfacing line	*/
b8 Global_LCDIsUnderUsage;

/*	NVIC indexes	*/
NVIC_Interrupt_t Global_LCDDmaInterruptNumber;
NVIC_Interrupt_t Global_RefreshQuarterOfTheDisplayTimerInterruptNumber;

/*
 * Peak to peak value in a single frame.
 * Is calculated as the difference between the largest and smallest values in
 * current frame.
 * "current frame" starts when "tftScrollCounter" equals zero, and ends when it
 * equals "tftScrollCounterMax".
 * Unit is: [TFT screen pixels].
 */
u8 Global_PeakToPeakValueInCurrentFrame;
u8 Global_LargestVlaueInCurrentFrame;
u8 Global_SmallestVlaueInCurrentFrame;

/*	These two, determine how often and when info image is updated	*/
u8 Global_NumberOfsentQuartersSinceLastInfoUpdate;
u8 Global_NumberOfsentQuartersRequieredForInfoUpdate;

/*	state of enter button	*/
b8 Global_Enter;

/*	current resolution values	*/
u32 Global_CurrentMicroVoltsPerPix;
u32 Global_CurrentMicroSecondsPerPix;
u8 Global_CurrentUsedAdcChannelIndex;

/*	current running state of the machine	*/
OSC_RunningState_t Global_RunningState;

/*	pausing of display	*/
b8 Global_Paused;

void OSC_voidStartSignalDrawing(void)
{
	/*	start drawing	*/
	(void)TIM_u64InitTimTrigger(
		// TODO: set this '100' to a configurable startup value
		LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER, 100000ul,
		100000ul * 2, OSC_voidTimRefreshQuarterCallback);
}
