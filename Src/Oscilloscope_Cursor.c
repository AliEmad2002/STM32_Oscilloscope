/*
 * Oscilloscope_Cursor.c
 *
 *  Created on: Jan 28, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Menu_config.h"
#include "Menu_interface.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Txt_interface.h"
#include "Delay_interface.h"
#include "diag/trace.h"

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
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"

volatile OSC_Cursor_t Cursor_v1;
volatile OSC_Cursor_t Cursor_v2;
volatile OSC_Cursor_t Cursor_t1;
volatile OSC_Cursor_t Cursor_t2;

/*	increment cursor position (if enabled)	*/
void OSC_voidIncrementCursorV1(void)
{
	if (Cursor_v1.isEnabled)
	{
		if (Cursor_v1.pos < 127)
			Cursor_v1.pos++;
	}
}

void OSC_voidIncrementCursorV2(void)
{
	if (Cursor_v2.isEnabled)
	{
		if (Cursor_v2.pos < 127)
			Cursor_v2.pos++;
	}
}

void OSC_voidIncrementCursorT1(void)
{
	if (Cursor_t1.isEnabled)
	{
		if (Cursor_t1.pos < NUMBER_OF_SAMPLES - 1)
			Cursor_t1.pos++;
	}
}

void OSC_voidIncrementCursorT2(void)
{
	if (Cursor_t2.isEnabled)
	{
		if (Cursor_t2.pos < NUMBER_OF_SAMPLES - 1)
			Cursor_t2.pos++;
	}
}

/*	decrement cursor position (if enabled)	*/
void OSC_voidDecrementCursorV1(void)
{
	if (Cursor_v1.isEnabled)
	{
		if (Cursor_v1.pos > 0)
			Cursor_v1.pos--;
	}
}

void OSC_voidDecrementCursorV2(void)
{
	if (Cursor_v2.isEnabled)
	{
		if (Cursor_v2.pos > 0)
			Cursor_v2.pos--;
	}
}

void OSC_voidDecrementCursorT1(void)
{
	if (Cursor_t1.isEnabled)
	{
		if (Cursor_t1.pos > 0)
			Cursor_t1.pos--;
	}
}

void OSC_voidDecrementCursorT2(void)
{
	if (Cursor_t2.isEnabled)
	{
		if (Cursor_t2.pos > 0)
			Cursor_t2.pos--;
	}
}








