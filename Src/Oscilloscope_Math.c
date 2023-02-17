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


/**
 * TODO:
 * Math mode is not done yet.
 **/
void OSC_voidDrawXYModeFrame(void)
{
//	static u8 xPixPrev = 0;
//	static u8 yPixPrev = 0;
//
//	/*	clear display (fill with BG color)	*/
//	OSC_voidFillDisplayWithBGColor();
//
//	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
//	{
//		/*	get readings in volts	*/
//		volatile f32 ch1Volts =
//			((f32)GET_CH1_V_IN_MICRO_VOLTS(Global_SampleBuffer[2 * i])) /
//			1000000.0f;
//
//		volatile f32 ch2Volts =
//			((f32)GET_CH2_V_IN_MICRO_VOLTS(Global_SampleBuffer[2 * i + 1])) /
//			1000000.0f;
//
//		/*	parse	*/
//		/*	x-axis parsed value multiplied by 1e6	*/
//		volatile s32 xParsed =
//			1000000.0f * (f32)MathParser_d64Evaluate(
//				(MathParser_t*)&Global_xMathParser,
//				ch1Volts, ch2Volts, 0.0);
//
//		/*	y-axis parsed value multiplied by 1e6	*/
//		volatile s32 yParsed =
//			1000000.0f * (f32)MathParser_d64Evaluate(
//				(MathParser_t*)&Global_yMathParser,
//				ch1Volts, ch2Volts, 0.0);
//
//		/*	convert to pixels	*/
//		volatile s32 xPix = GET_X_PIX_MATH_MODE(xParsed);
//
//		volatile s32 yPix = GET_Y_PIX_MATH_MODE(yParsed);
//
//
//		/*	limit current readings to displayable range (chop readings)	*/
//		CHOP_X_VALUE(xPix);
//		CHOP_Y_VALUE(yPix);
//
//		/*
//		 * connect {x, y}, {xPrev, yPrev} by a line.
//		 * (input axis are swapped in this function call, as the TFT is used
//		 * rotated)
//		 */
//		TFT2_voidDrawLine(
//			(TFT2_t*)&Global_LCD, yPix, xPix, yPixPrev, xPixPrev,
//			LCD_MAIN_DRAWING_COLOR_U16);
//
//		/*	update prev's	*/
//		xPixPrev = xPix;
//		yPixPrev = yPix;
//	}
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







