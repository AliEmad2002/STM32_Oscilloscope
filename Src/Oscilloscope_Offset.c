/*
 * Oscilloscope_Offset.c
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
#include "Oscilloscope_Offset.h"


void OSC_voidIncrementCh1Offset(void)
{
	if (Global_IsCh1Enabled)
	{
		Global_Offset1MicroVolts += Global_CurrentCh1MicroVoltsPerPix;
	}
}

void OSC_voidIncrementCh2Offset(void)
{
	if (Global_IsCh2Enabled)
	{
		Global_Offset2MicroVolts += Global_CurrentCh2MicroVoltsPerPix;
	}
}

void OSC_voidDecrementCh1Offset(void)
{
	if (Global_IsCh1Enabled)
	{
		Global_Offset1MicroVolts -= Global_CurrentCh1MicroVoltsPerPix;
	}
}

void OSC_voidDecrementCh2Offset(void)
{
	if (Global_IsCh2Enabled)
	{
		Global_Offset2MicroVolts -= Global_CurrentCh2MicroVoltsPerPix;
	}
}

