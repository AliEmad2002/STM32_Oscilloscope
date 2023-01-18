/*
 * Oscilloscope_init_HAL.c
 *
 *  Created on: Jan 18, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/

#include "Std_Types.h"
#include "Bit_Math.h"
#include "Debug_active.h"

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
#include "Oscilloscope_init_HAL.h"


void OSC_voidInitTFT(void)
{
	TFT2_voidInit(
		&LCD, LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, LCD_RST_PIN, LCD_A0_PIN,
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	/*	set maximum brightness by default	*/
	TFT2_voidSetBrightness(&LCD, POW_TWO(16) - 1);
}

void OSC_voidDisplayStartupScreeen(void)
{
	/*	set bounds	*/
	TFT2_SET_X_BOUNDARIES(&LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&LCD, 0, 159);

	/*	start data write operation	*/
	TFT2_WRITE_CMD(&LCD, TFT_CMD_MEM_WRITE);

	TFT2_ENTER_DATA_MODE(&LCD);

	/*	DMA send	*/
	TFT2_voidFillDMA(&LCD, &colorBlackU8Val, 128 * 160);

	/*	wait for DMA to get done and clear flags	*/
	TFT2_voidWaitCurrentDataTransfer(&LCD);

	TFT2_voidClearDMATCFlag(&LCD);

	TFT2_voidDisableDMAChannel(&LCD);

	/*	give user time to see startup screen	*/
	Delay_voidBlockingDelayMs(LCD_STARTUP_SCREEN_DELAY_MS);
}

void OSC_voidInitSignalDrawing(void)
{
	/*	enable interrupt (to be used for less drawing overhead)	*/
	TFT2_voidSetDMATransferCompleteCallback(
		&LCD, OSC_voidDMATransferCompleteCallback);

	TFT2_voidEnableDMATransferCompleteInterrupt(&LCD);
}

void OSC_voidInitInfoDrawing(void)
{
	/*
	 * split display to two parts, large one for displaying signal (0-130),
	 * and small one for signal data (131-160).
	 * TODO: (based on saved user settings).
	 *
	 * The split can be cancelled from settings.
	 */
	TFT2_voidInitScroll(&LCD, 0, 130, 32);
}

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

void OSC_voidInitHAL(void)
{
	OSC_voidInitTFT();

	OSC_voidDisplayStartupScreeen();

	OSC_voidInitSignalDrawing();

	OSC_voidInitInfoDrawing();

	OSC_voidStartSignalDrawing();

	OSC_voidStartInfoDrawing();
}

