/*
 * Oscilloscope_Brigtness.c
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
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Brightness.h"

void OSC_voidIncrementBrightness(void)
{
	u32 brightness = TFT2_u16GetBrightness((TFT2_t*)&Global_LCD);

	brightness += 1000;

	if (brightness > POW_TWO(16) - 1)
		brightness = POW_TWO(16) - 1;

	TFT2_voidSetBrightness((TFT2_t*)&Global_LCD, brightness);
}

void OSC_voidDecrementBrightness(void)
{
	u16 brightness = TFT2_u16GetBrightness((TFT2_t*)&Global_LCD);

	if (brightness > 3000)
		brightness -= 1000;

	TFT2_voidSetBrightness((TFT2_t*)&Global_LCD, brightness);
}

