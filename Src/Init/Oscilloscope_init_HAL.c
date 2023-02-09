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
#include "math.h"

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

	/*	clear display (fill with black color)	*/
	TFT2_voidClearDisplay(&Global_LCD);

	/*	draw time axis	*/
	for (u8 i = 0; i < 160; i++)
	{
		TFT2_SET_PIXEL(&Global_LCD, 128 / 2, i, LCD_AXIS_DRAWING_COLOR_U16);
		Delay_voidBlockingDelayMs(2);
	}

	/*	draw voltage axis	*/
	for (u8 i = 0; i < 128; i++)
	{
		TFT2_SET_PIXEL(&Global_LCD, i, 160 / 2 - 1, LCD_AXIS_DRAWING_COLOR_U16);
		Delay_voidBlockingDelayMs(2);
	}

//	/*	draw 1.25 of a sine wave on 70% of the display	*/
//	u8 lastX = 64;
//	for (u8 y = 0; y < 112; y++)
//	{
//		u8 x = 64 + 45.0 * sin(2.0 * M_PI / 45.0 * (d64)y);
//
//		/*	connect x, lastX	*/
//
//
//		TFT2_SET_PIXEL(&Global_LCD, x, y, LCD_MAIN_DRAWING_COLOR_U16);
//		Delay_voidBlockingDelayMs(1);
//	}
//
//
//	/*	draw line after that sine	*/

	/*	give user time to see startup screen	*/
	//Delay_voidBlockingDelayMs(LCD_STARTUP_SCREEN_DELAY_MS);

	/*	clear display (fill with black color)	*/
	//TFT2_voidClearDisplay(&Global_LCD);
}

void OSC_voidInitHAL(void)
{
	OSC_voidInitTFT();

	OSC_voidDisplayStartupScreeen();

	OSC_voidSetDisplayBoundariesForSignalArea();

	Rotary_Encoder_voidInit();
}

