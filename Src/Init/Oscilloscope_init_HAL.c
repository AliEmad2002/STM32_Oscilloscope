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
#include "Rotary_Encoder_Interface.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_init_HAL.h"

extern volatile TFT2_t Global_LCD;

extern void OSC_voidSetDisplayBoundariesForSignalArea(void);

void OSC_voidInitTFT(void)
{
	TFT2_voidInit(
		&Global_LCD, LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, LCD_RST_PIN, LCD_A0_PIN,
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	/*	set maximum brightness by default	*/
	TFT2_voidSetBrightness(&Global_LCD, POW_TWO(15) - 1);
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

void OSC_voidInitHAL(void)
{
	OSC_voidInitTFT();

	OSC_voidDisplayStartupScreeen();

	OSC_voidSetDisplayBoundariesForSignalArea();

	Rotary_Encoder_voidInit();
}

