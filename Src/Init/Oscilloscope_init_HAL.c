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
#include "Oscilloscope_init_HAL.h"

extern void OSC_voidDMATransferCompleteCallback(void);

extern TFT2_t Global_LCD;

void OSC_voidInitTFT(void)
{
	TFT2_voidInit(
		&Global_LCD, LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, LCD_RST_PIN, LCD_A0_PIN,
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	/*	set maximum brightness by default	*/
	TFT2_voidSetBrightness(&Global_LCD, POW_TWO(16) - 1);
}

void OSC_voidDisplayStartupScreeen(void)
{
	/*	set bounds	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

	/*	start data write operation	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);

	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	DMA send	*/
	TFT2_voidFillDMA(&Global_LCD, &colorBlackU8Val, 128 * 160);

	/*	wait for DMA to get done and clear flags	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

	/*	give user time to see startup screen	*/
	Delay_voidBlockingDelayMs(LCD_STARTUP_SCREEN_DELAY_MS);
}

void OSC_voidInitSignalDrawing(void)
{
//	/*	enable interrupt (to be used for less drawing overhead)	*/
//	TFT2_voidSetDMATransferCompleteCallback(
//		&Global_LCD, OSC_voidDMATransferCompleteCallback);
//
//	TFT2_voidEnableDMATransferCompleteInterrupt(&Global_LCD);
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
	TFT2_voidInitScroll(&Global_LCD, 0, 130, 32);
}

void OSC_voidInitHAL(void)
{
	OSC_voidInitTFT();

	OSC_voidDisplayStartupScreeen();

	OSC_voidInitSignalDrawing();

	OSC_voidInitInfoDrawing();

	static volatile u32 foo = 55;
	foo++;
}

