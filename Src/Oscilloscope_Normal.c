/*
 * Oscilloscope_Normal.c
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
#include "Oscilloscope_Normal.h"

void DRAW_DIVS(void)
{
	/*	voltage Div.s	*/
	for (u16 i = 0; i <= VOLTAGE_DIVS_PER_LINE; i++)
	{
		for (u16 j = 0; j < LINES_PER_IMAGE_BUFFER; j++)
		{
			u16 index = j * SIGNAL_LINE_LENGTH + i * PIXELS_PER_VOLTAGE_DIV + 1;
			Global_ImgBufferArr[Global_NotInUseImgBufferIndex][index] =
				LCD_DIV_DRAWING_COLOR_U16;
		}
	}

	/*	Time Div.s	*/
	/*	it is known that there's one per image div	*/
	FILL_SEGMENT(
		LINES_PER_IMAGE_BUFFER - 1, 0, SIGNAL_LINE_LENGTH - 1,
		LCD_DIV_DRAWING_COLOR_U16);
}

void OSC_voidDrawNormalModeFrame(void)
{
	/*
	 * data is to be extracted from sample buffers and converted to these two.
	 */
	s32 currentRead1Pix = 0;
	s32 currentRead2Pix = 0;

	/*	counter of the processed samples of current image frame	*/
	u8 readCount = 0;

	/*
	 * these are set to the first reading in the sample buffer, in order to
	 * obtain valid vPP.
	 */
	Global_Ch1MinValueInCurrentFrame =
		GET_CH1_V_IN_PIXELS(Global_SampleBuffer[0]);

	Global_Ch1MaxValueInCurrentFrame = Global_Ch1MinValueInCurrentFrame;

	Global_Ch2MinValueInCurrentFrame =
		GET_CH2_V_IN_PIXELS(Global_SampleBuffer[1]);

	Global_Ch2MaxValueInCurrentFrame = Global_Ch2MinValueInCurrentFrame;

	Global_Ch2SumOfCurrentFrame = 0;
	Global_Ch1SumOfCurrentFrame = 0;

	/*
	 * when v_ch is out of the displayable range, it's important to not tell
	 * DMA to fill using them, as it causes DMA fault.
	 */
	b8 isRead1InRange = false;
	b8 isRead2InRange = false;

	/*	draw current image frame of screen	*/
	while(readCount < NUMBER_OF_SAMPLES)
	{
		/*	clear image buffer number 'j'	*/
		CLEAR_IMG_BUFFER;

		/*	draw time axis (voltage = 0)	*/
		DRAW_TIME_AXIS;

		/*	draw Div.s	*/
		DRAW_DIVS();

		/*	Draw/process image buffer number 'j'	*/
		for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i++)
		{
			if (Global_IsCh1Enabled)
			{
				/*	read ADC converted value	*/
				currentRead1Pix =
					GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * readCount]);

				/*	add to sum (used in calculating avg	*/
				Global_Ch1SumOfCurrentFrame += currentRead1Pix;

				/*	check range of display	*/
				isRead1InRange = IS_PIX_IN_DISPLAYABLE_RANGE(currentRead1Pix);

				/*
				 * if last reading was in range, and this one was not, chop this
				 * one.
				 */
				if (!isRead1InRange && Global_Ch1LastReadWasInRange)
					CHOP_Y_VALUE(currentRead1Pix);

				/*	peak to peak calculation	*/
				WRITE_IF_SMALLER(
					Global_Ch1MinValueInCurrentFrame, currentRead1Pix);
				WRITE_IF_LARGER(
					Global_Ch1MaxValueInCurrentFrame, currentRead1Pix);

				/*	sort readings and previous readings	 */
				if (isRead1InRange || Global_Ch1LastReadWasInRange)
				{
					Global_Smaller1 =
						GET_SMALLER(currentRead1Pix, Global_LastRead1);
					Global_Larger1 =
						GET_LARGER(currentRead1Pix, Global_LastRead1);
					Global_LastRead1 = currentRead1Pix;

					/*	draw main color from "smaller1" to "larger1"	*/
					FILL_SEGMENT(
						i, Global_Smaller1, Global_Larger1,
						LCD_MAIN_DRAWING_COLOR_U16);
				}

				/*	update "in_range" flag	*/
				Global_Ch1LastReadWasInRange = isRead1InRange;
			}

			if (Global_IsCh2Enabled)
			{
				/*	read ADC converted value	*/
				currentRead2Pix =
					GET_CH2_V_IN_PIXELS(Global_SampleBuffer[2 * readCount + 1]);

				/*	add to sum (used in calculating avg	*/
				Global_Ch2SumOfCurrentFrame += currentRead2Pix;

				/*	check range of display	*/
				isRead2InRange = IS_PIX_IN_DISPLAYABLE_RANGE(currentRead2Pix);

				/*
				 * if last reading was in range, and this one was not, chop this
				 * one.
				 */
				if (!isRead2InRange && Global_Ch2LastReadWasInRange)
					CHOP_Y_VALUE(currentRead2Pix);

				/*	peak to peak calculation	*/
				WRITE_IF_SMALLER(
					Global_Ch2MinValueInCurrentFrame, currentRead2Pix);
				WRITE_IF_LARGER(
					Global_Ch2MaxValueInCurrentFrame, currentRead2Pix);

				/*	sort readings and previous readings	 */
				if (isRead2InRange || Global_Ch2LastReadWasInRange)
				{
					Global_Smaller2 =
						GET_SMALLER(currentRead2Pix, Global_LastRead2);
					Global_Larger2 =
						GET_LARGER(currentRead2Pix, Global_LastRead2);
					Global_LastRead2 = currentRead2Pix;

					/*	draw secondary color from "smaller2" to "larger2"	*/
					FILL_SEGMENT(
						i, Global_Smaller2, Global_Larger2,
						LCD_SECONDARY_DRAWING_COLOR_U16);
				}

				/*	update "in_range" flag	*/
				Global_Ch2LastReadWasInRange = isRead2InRange;
			}

			/*	draw time cursors (if any)	*/
			if (Cursor_t1.isEnabled && Cursor_t1.pos == readCount)
			{
				DRAW_TIME_CURSOR(
					readCount, LCD_CURSOR1_DRAWING_COLOR_U16);
			}

			if (Cursor_t2.isEnabled && Cursor_t2.pos == readCount)
			{
				DRAW_TIME_CURSOR(
					readCount, LCD_CURSOR2_DRAWING_COLOR_U16);
			}

			/*	increment readCount	*/
			readCount++;
		}

		/*	draw voltage cursors (if any)	*/
		if (Cursor_v1.isEnabled)
		{
			DRAW_VOLTAGE_CURSOR(
				Cursor_v1.pos, LCD_CURSOR1_DRAWING_COLOR_U16);
		}

		if (Cursor_v2.isEnabled)
		{
			DRAW_VOLTAGE_CURSOR(
				Cursor_v2.pos, LCD_CURSOR2_DRAWING_COLOR_U16);
		}

		/*	draw offset pointer (case first segment of the frame only)	*/
		if (readCount == NUMBER_OF_SAMPLES / NUMBER_OF_IMAGE_BUFFERS_PER_FRAME)
		{
			DRAW_OFFSET_POINTER_CH1;
			DRAW_OFFSET_POINTER_CH2;
		}

		/*	Send buffer to TFT using DMA (Internally waits for DMA TC)	*/
		TFT2_voidSendPixels(
			(TFT2_t*)&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex],
			LINES_PER_IMAGE_BUFFER * SIGNAL_LINE_LENGTH);

		/*	Update imgBufferIndex	*/
		if (Global_NotInUseImgBufferIndex == 0)
			Global_NotInUseImgBufferIndex = 1;
		else // if (Global_InUseImgBufferIndex == 1)
			Global_NotInUseImgBufferIndex = 0;
	}

	/*	update peak to peak value	*/
	Global_Ch1PeakToPeakValueInCurrentFrame =
		Global_Ch1MaxValueInCurrentFrame - Global_Ch1MinValueInCurrentFrame;

	Global_Ch2PeakToPeakValueInCurrentFrame =
		Global_Ch2MaxValueInCurrentFrame - Global_Ch2MinValueInCurrentFrame;
}

