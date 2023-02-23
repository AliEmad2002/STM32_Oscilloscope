/*
 * Oscilloscope_Div.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Debug_active.h"
#include "Img_interface.h"
#include "Colors.h"
#include "LinkedList.h"
#include "MathParser.h"

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
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_SavedConfig.h"
#include "Oscilloscope_Div.h"


void OSC_voidIncrementCh1VoltageDiv(void)
{
	u32 currentMilliVoltsPerDiv =
		(Global_CurrentCh1MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV) / 1000;

	for (u8 i = 0; i < NUMBER_OF_VOLT_DIVS; i++)
	{
		if (currentMilliVoltsPerDiv <= OSC_mVoltsPerDivArr[i])
		{
			if (i < NUMBER_OF_VOLT_DIVS - 1)
				Global_CurrentCh1MicroVoltsPerPix =
					(OSC_mVoltsPerDivArr[i + 1] * 1000) /
					PIXELS_PER_VOLTAGE_DIV;

			return;
		}
	}

	/*	update saved configuration in flash	*/
	OSC_voidWriteCurrentConfigOnFlash();
}

void OSC_voidIncrementCh2VoltageDiv(void)
{
	u32 currentMilliVoltsPerDiv =
		(Global_CurrentCh2MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV) / 1000;

	for (u8 i = 0; i < NUMBER_OF_VOLT_DIVS; i++)
	{
		if (currentMilliVoltsPerDiv <= OSC_mVoltsPerDivArr[i])
		{
			if (i < NUMBER_OF_VOLT_DIVS - 1)
				Global_CurrentCh2MicroVoltsPerPix =
					(OSC_mVoltsPerDivArr[i + 1] * 1000) /
					PIXELS_PER_VOLTAGE_DIV;

			return;
		}
	}

	/*	update saved configuration in flash	*/
	OSC_voidWriteCurrentConfigOnFlash();
}

void OSC_voidIncrementTimeDiv(void)
{
	u64 currentNanoSecondsPerDiv =
		Global_CurrentNanoSecondsPerPix * PIXELS_PER_TIME_DIV;

	for (u8 i = 0; i < NUMBER_OF_TIME_DIVS; i++)
	{
		if (currentNanoSecondsPerDiv <= OSC_nSecondsPerDivArr[i])
		{
			if (i < NUMBER_OF_TIME_DIVS - 1)
				Global_CurrentNanoSecondsPerPix =
					OSC_nSecondsPerDivArr[i + 1] / PIXELS_PER_TIME_DIV;

			return;
		}
	}

	/*	update saved configuration in flash	*/
	OSC_voidWriteCurrentConfigOnFlash();
}

void OSC_voidDecrementCh1VoltageDiv(void)
{
	u32 currentMilliVoltsPerDiv =
		(Global_CurrentCh1MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV) / 1000;

	for (u8 i = 0; i < NUMBER_OF_VOLT_DIVS; i++)
	{
		if (currentMilliVoltsPerDiv <= OSC_mVoltsPerDivArr[i])
		{
			if (i > 0)
				Global_CurrentCh1MicroVoltsPerPix =
					(OSC_mVoltsPerDivArr[i - 1] * 1000) /
					PIXELS_PER_VOLTAGE_DIV;

			return;
		}
	}

	/*	update saved configuration in flash	*/
	OSC_voidWriteCurrentConfigOnFlash();
}

void OSC_voidDecrementCh2VoltageDiv(void)
{
	u32 currentMilliVoltsPerDiv =
		(Global_CurrentCh2MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV) / 1000;

	for (u8 i = 0; i < NUMBER_OF_VOLT_DIVS; i++)
	{
		if (currentMilliVoltsPerDiv <= OSC_mVoltsPerDivArr[i])
		{
			if (i > 0)
				Global_CurrentCh2MicroVoltsPerPix =
					(OSC_mVoltsPerDivArr[i - 1] * 1000) /
					PIXELS_PER_VOLTAGE_DIV;

			return;
		}
	}

	/*	update saved configuration in flash	*/
	OSC_voidWriteCurrentConfigOnFlash();
}

void OSC_voidDecrementTimeDiv(void)
{
	u64 currentNanoSecondsPerDiv =
		Global_CurrentNanoSecondsPerPix * PIXELS_PER_TIME_DIV;

	for (u8 i = 0; i < NUMBER_OF_TIME_DIVS; i++)
	{
		if (currentNanoSecondsPerDiv <= OSC_nSecondsPerDivArr[i])
		{
			if (i > 0)
				Global_CurrentNanoSecondsPerPix =
					OSC_nSecondsPerDivArr[i - 1] / PIXELS_PER_TIME_DIV;

			return;
		}
	}

	/*	update saved configuration in flash	*/
	OSC_voidWriteCurrentConfigOnFlash();
}

