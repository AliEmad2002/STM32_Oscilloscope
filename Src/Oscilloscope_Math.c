/*
 * Oscilloscope_Math.c
 *
 *  Created on: Feb 16, 2023
 *      Author: ali20
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Delay_interface.h"
#include "Img_config.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Debug_active.h"
#include "Target_config.h"
#include "Error_Handler_interface.h"
#include "Txt_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "My_Math.h"
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
#include "Rotary_Encoder_Interface.h"
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_init_MCAL.h"
#include "Oscilloscope_init_HAL.h"
#include "Oscilloscope_init_Global.h"
#include "Oscilloscope_Menu_Interface.h"
#include "Oscilloscope_Conversions.h"
#include "Oscilloscope_ISRs.h"
#include "Oscilloscope_Sampling.h"
#include "Oscilloscope_Draw.h"
#include "Oscilloscope_Math.h"


void OSC_voidDrawXYModeFrame(void)
{
	/*	clear display (fill with BG color)	*/
	OSC_voidFillDisplayWithBGColor();

	volatile d64 xPrev = 0.0;
	volatile d64 yPrev = 0.0;

	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		d64 ch1Current = GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * i]);
		d64 ch2Current = GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * i + 1]);

		volatile d64 x =
			MathParser_d64Evaluate(
				(MathParser_t*)&Global_xMathParser,
				ch1Current, ch2Current, 0.0);

		volatile d64 y =
			MathParser_d64Evaluate(
				(MathParser_t*)&Global_yMathParser,
				ch1Current, ch2Current, 0.0);

		/*	limit current readings to displayable range (chop readings)	*/
		CHOP_X_VALUE(x);
		CHOP_Y_VALUE(y);

		/*	connect {x, y}, {xPrev, yPrev} by a line.	*/


		/*	update prev's	*/
		xPrev = x;
		yPrev = y;
	}
}

void OSC_voidEditXAxisMathExpression(void)
{
	OSC_voidEditStrtingOnDislpay((char*)Global_xMathParser.str);

	Global_xMathParser.n = strlen((char*)Global_xMathParser.str);
}

void OSC_voidEditYAxisMathExpression(void)
{
	OSC_voidEditStrtingOnDislpay((char*)Global_yMathParser.str);

	Global_yMathParser.n = strlen((char*)Global_yMathParser.str);
}







