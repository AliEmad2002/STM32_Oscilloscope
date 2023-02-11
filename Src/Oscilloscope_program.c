/*
 * Oscilloscope_program.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
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
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_init_MCAL.h"
#include "Oscilloscope_init_HAL.h"
#include "Oscilloscope_init_Global.h"
#include "Oscilloscope_Menu_Interface.h"
#include "Oscilloscope_Conversions.h"
#include "Oscilloscope_ISRs.h"
#include "Oscilloscope_Sampling.h"
#include "Oscilloscope_interface.h"

/*******************************************************************************
 * Drawing functions and macros:
 ******************************************************************************/

/*
 * fills a segment in selected line in selected image buffer with any color
 */
#define FILL_SEGMENT(line, start, end, color)             \
{                                                                         \
	DMA_voidSetMemoryAddress(                                             \
		DMA_UnitNumber_1, LINE_SEGMENT_DMA_CHANNEL,                       \
		(void*)&Global_ImgBufferArr[Global_NotInUseImgBufferIndex][(line) * SIGNAL_LINE_LENGTH + (start)]);  \
                                                                          \
	DMA_voidSetNumberOfData(                                              \
		DMA_UnitNumber_1, LINE_SEGMENT_DMA_CHANNEL,                       \
		(end) - (start) + 1);                                             \
                                                                          \
		DMA_voidSetPeripheralAddress(                                     \
			DMA_UnitNumber_1, LINE_SEGMENT_DMA_CHANNEL,                   \
			(void*)&color);                                               \
                                                                          \
	DMA_voidEnableChannel(                                                \
		DMA_UnitNumber_1, LINE_SEGMENT_DMA_CHANNEL);                      \
                                                                          \
	/*                                                                    \
	 * wait for DMA transfer complete                                     \
	 * (starting drawing next segments before this operation is done      \
	 * may result in them being overwritten with this)                    \
	 */                                                                   \
	DMA_voidWaitTillChannelIsFreeAndDisableIt(                            \
		DMA_UnitNumber_1, LINE_SEGMENT_DMA_CHANNEL);                      \
}

#define CLEAR_IMG_BUFFER	    \
{										        \
	FILL_SEGMENT(                               \
		0, 0,                 \
		LINES_PER_IMAGE_BUFFER * SIGNAL_LINE_LENGTH - 1,       \
		LCD_BACKGROUND_COLOR_U16                \
	)                                           \
}

#define WAIT_NEXT_FRAME_AND_UPDATE_TIMESTAMP(lastFrameTimeStamp)           \
{                                                                          \
	while(                                                                 \
		STK_u64GetElapsedTicks() - lastFrameTimeStamp <                    \
		STK_u32GetTicksPerSecond() / LCD_FPS                                        \
	);                                                                     \
                                                                           \
	(lastFrameTimeStamp) = STK_u64GetElapsedTicks();                       \
}

#define DRAW_TIME_CURSOR(pos, color)			 \
{																 \
	u8 lineNumber = (pos) % LINES_PER_IMAGE_BUFFER;              \
                                                                 \
	for (u8 k = 0; k < SIGNAL_LINE_LENGTH; k += SUM_OF_DASH_LEN)                \
	{                                                            \
		for (u8 l = 0; l < 3 && k < SIGNAL_LINE_LENGTH; l++)                    \
		{                                                        \
			u16 index = lineNumber * SIGNAL_LINE_LENGTH + k + l;                \
			Global_ImgBufferArr[Global_NotInUseImgBufferIndex][index] =       \
				(color);                                         \
		}                                                        \
	}                                                            \
}

#define DRAW_VOLTAGE_CURSOR(pos, color)	                               \
{                                                                      \
	for (u8 i = 0;i < LINES_PER_IMAGE_BUFFER; i += SUM_OF_DASH_LEN)                              \
	{                                                                  \
		for (u8 k = 0; k < DASHED_LINE_DRAWN_SEGMENT_LEN; k++)                                     \
		{                                                              \
			u16 index = (i + k) * SIGNAL_LINE_LENGTH + (pos);          \
                                                                       \
			if (index >= IMG_BUFFER_SIZE)                       \
				break;                                \
			Global_ImgBufferArr[Global_NotInUseImgBufferIndex][index] =\
				(color);                      						   \
		}                                                              \
	}                                                                  \
}

#define DRAW_TIME_AXIS                      \
{                                                           \
	for (u16 i = SIGNAL_LINE_LENGTH / 2; i < LINES_PER_IMAGE_BUFFER * SIGNAL_LINE_LENGTH; i+=SIGNAL_LINE_LENGTH)     \
	{                                                       \
		Global_ImgBufferArr[Global_NotInUseImgBufferIndex][i] =          \
			LCD_AXIS_DRAWING_COLOR_U16;                     \
	}                                                       \
}

#define CONVERT_UV_TO_PIX_CH1(uv)((uv) / (s32)Global_CurrentCh1MicroVoltsPerPix)

#define CONVERT_UV_TO_PIX_CH2(uv)((uv) / (s32)Global_CurrentCh2MicroVoltsPerPix)

#define DRAW_OFFSET_POINTER_CH1							\
{                                                                       \
	/*	if channel is enabled and offset is in displayable range	*/  \
	if (                                                                \
		Global_IsCh1Enabled &&                                          \
		CONVERT_UV_TO_PIX_CH1(Global_Offset1MicroVolts) + SIGNAL_LINE_LENGTH / 2 >          \
		OFFSET_POINTER_LEN &&                                           \
		CONVERT_UV_TO_PIX_CH1(Global_Offset1MicroVolts) + SIGNAL_LINE_LENGTH / 2 <          \
		SIGNAL_LINE_LENGTH - OFFSET_POINTER_LEN                                        \
	)                                                                   \
	{                                                                   \
		u8 start =                                                      \
			CONVERT_UV_TO_PIX_CH1(Global_Offset1MicroVolts) + SIGNAL_LINE_LENGTH / 2 -      \
			OFFSET_POINTER_LEN / 2;                                     \
		u8 n = OFFSET_POINTER_LEN;                                      \
		for (u8 j = 0; j < OFFSET_POINTER_LEN / 2 + 1; j++)             \
		{                                                               \
			for (u8 k = start; k < start + n; k++)                      \
			{                                                           \
				Global_ImgBufferArr[Global_NotInUseImgBufferIndex][SIGNAL_LINE_LENGTH * j + k] =    \
				LCD_OFFSET_POINTER1_DRAWING_COLOR_U16;                  \
			}                                                           \
			start++;                                                    \
			n -= 2;                                                     \
		}                                                               \
	}                                                                   \
}

#define DRAW_OFFSET_POINTER_CH2							\
{                                                                       \
	/*	if channel is enabled and offset is in displayable range	*/  \
	if (                                                                \
		Global_IsCh2Enabled &&                                          \
		CONVERT_UV_TO_PIX_CH2(Global_Offset2MicroVolts) + SIGNAL_LINE_LENGTH / 2 >          \
		OFFSET_POINTER_LEN &&                                           \
		CONVERT_UV_TO_PIX_CH2(Global_Offset2MicroVolts) + SIGNAL_LINE_LENGTH / 2 <          \
		SIGNAL_LINE_LENGTH - OFFSET_POINTER_LEN                                        \
	)                                                                   \
	{                                                                   \
		u8 start =                                                      \
			CONVERT_UV_TO_PIX_CH2(Global_Offset2MicroVolts) + SIGNAL_LINE_LENGTH / 2 -      \
			OFFSET_POINTER_LEN / 2;                                     \
		u8 n = OFFSET_POINTER_LEN;                                      \
		for (u8 j = 0; j < OFFSET_POINTER_LEN / 2 + 1; j++)             \
		{                                                               \
			for (u8 k = start; k < start + n; k++)                      \
			{                                                           \
				Global_ImgBufferArr[Global_NotInUseImgBufferIndex][SIGNAL_LINE_LENGTH * j + k] =    \
				LCD_OFFSET_POINTER2_DRAWING_COLOR_U16;                  \
			}                                                           \
			start++;                                                    \
			n -= 2;                                                     \
		}                                                               \
	}                                                                   \
}

#define IS_PIX_IN_DISPLAYABLE_RANGE(pix)	\
(                                           \
	pix >= SIGNAL_IMG_X_MIN &&  \
	pix < SIGNAL_LINE_LENGTH    \
)

#define CHOP_X_VALUE(xVal)                      \
{                                               \
	if ((xVal) < 0)                             \
		(xVal) = 0;                             \
                                                \
	else if ((xVal) >= NUMBER_OF_SAMPLES)       \
		(xVal) = NUMBER_OF_SAMPLES - 1;         \
}

#define CHOP_Y_VALUE(yVal)                      \
{                                               \
	if ((yVal) < 0)                             \
		(yVal) = 0;                             \
                                                \
	else if ((yVal) >= SIGNAL_LINE_LENGTH)      			    \
		(yVal) = SIGNAL_LINE_LENGTH - 1;       			    \
}

void OSC_voidSetDisplayBoundariesForSignalArea(void)
{
	/*	set screen boundaries for full signal image area	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, SIGNAL_IMG_X_MIN, SIGNAL_IMG_X_MAX);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, NUMBER_OF_SAMPLES - 1);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);
}

/*******************************************************************************
 * Normal mode frame generation:
 ******************************************************************************/
void OSC_voidDrawNormalModeFrame(void)
{
	/*	data is to be extracted from sample buffers to these two	*/
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

	/*
	 * when v_ch is out of the displayable range, it's important to not tell
	 * DMA to fill using them, as it causes DMA fault.
	 */
	b8 isRead1InRange = false;
	b8 isRead2InRange = false;

	/*
	 * draw current image frame of screen	*/
	while(1)
	{
		/*	clear image buffer number 'j'	*/
		CLEAR_IMG_BUFFER;

		/*	draw time axis (voltage = 0)	*/
		DRAW_TIME_AXIS;

		/*	Draw/process image buffer number 'j'	*/
		for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i++)
		{
			if (Global_IsCh1Enabled)
			{
				/*	read ADC converted value	*/
				currentRead1Pix =
					GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * readCount]);

				/*	check range of display	*/
				isRead1InRange = IS_PIX_IN_DISPLAYABLE_RANGE(currentRead1Pix);

				/*
				 * if last reading was in range, and this one was no, chop this
				 * one.
				 */
				if (!isRead1InRange && Global_Ch1LastReadWasInRange)
					CHOP_Y_VALUE(currentRead1Pix);

				/*	peak to peak calculation	*/
				WRITE_IF_SMALLER(Global_Ch1MinValueInCurrentFrame, currentRead1Pix);
				WRITE_IF_LARGER(Global_Ch1MaxValueInCurrentFrame, currentRead1Pix);

				/*	sort readings and previous readings	 */
				if (isRead1InRange || Global_Ch1LastReadWasInRange)
				{
					Global_Smaller1 = GET_SMALLER(currentRead1Pix, Global_LastRead1);
					Global_Larger1 = GET_LARGER(currentRead1Pix, Global_LastRead1);
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

				/*	check range of display	*/
				isRead2InRange = IS_PIX_IN_DISPLAYABLE_RANGE(currentRead2Pix);

				/*
				 * if last reading was in range, and this one was no, chop this
				 * one.
				 */
				if (!isRead2InRange && Global_Ch2LastReadWasInRange)
					CHOP_Y_VALUE(currentRead2Pix);

				/*	peak to peak calculation	*/
				WRITE_IF_SMALLER(Global_Ch2MinValueInCurrentFrame, currentRead2Pix);
				WRITE_IF_LARGER(Global_Ch2MaxValueInCurrentFrame, currentRead2Pix);

				/*	sort readings and previous readings	 */
				if (isRead2InRange || Global_Ch2LastReadWasInRange)
				{
					Global_Smaller2 = GET_SMALLER(currentRead2Pix, Global_LastRead2);
					Global_Larger2 = GET_LARGER(currentRead2Pix, Global_LastRead2);
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
			(TFT2_t*)&Global_LCD, (u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex],
			LINES_PER_IMAGE_BUFFER * SIGNAL_LINE_LENGTH);

		/*	Update imgBufferIndex	*/
		if (Global_NotInUseImgBufferIndex == 0)
			Global_NotInUseImgBufferIndex = 1;
		else // if (Global_InUseImgBufferIndex == 1)
			Global_NotInUseImgBufferIndex = 0;

		/*	check end of frame	*/
		if (readCount == NUMBER_OF_SAMPLES)
			break;
	}

	/*	update peak to peak value	*/
	Global_Ch1PeakToPeakValueInCurrentFrame =
		Global_Ch1MaxValueInCurrentFrame - Global_Ch1MinValueInCurrentFrame;

	Global_Ch2PeakToPeakValueInCurrentFrame =
		Global_Ch2MaxValueInCurrentFrame - Global_Ch2MinValueInCurrentFrame;
}

/*******************************************************************************
 * X-Y mode frame generation:
 ******************************************************************************/
void OSC_voidDrawXYModeFrame(void)
{
	/*	clear display (fill with BG color)	*/

	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		s32 xCurrent = GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * i]);
		s32 yCurrent = GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * i + 1]);

		/*	limit current readings to displayable range (chop readings)	*/
		CHOP_X_VALUE(xCurrent);
		CHOP_Y_VALUE(yCurrent);

		/*
		 * connect {xCurrent, yCurrent}, {Global_LastRead1, Global_LastRead2}
		 * by a line.
		 */

		/*	update {Global_LastRead1, Global_LastRead2}	*/
		Global_LastRead1 = xCurrent;
		Global_LastRead2 = yCurrent;
	}
}

/*******************************************************************************
 * Main thread super-loop:
 ******************************************************************************/
void OSC_voidMainSuperLoop(void)
{
	/*	last time info was drawn. (timestamp)	*/
	u64 lastInfoDrawTime = 0;

	/*	periodic time to draw info (in STK ticks)	*/
	volatile u64 infoDrawPeriod =
		INFO_DRAWING_PERIODIC_TIME_MS * STK_TICKS_PER_MS;

	u64 lastFrameTimeStamp = 0;

	while (1)
	{
		/*	wait for frame time to come (according to configured FPS)	*/
		WAIT_NEXT_FRAME_AND_UPDATE_TIMESTAMP(lastFrameTimeStamp);

		/*
		 * if user opened menu, enter it.
		 * (menu internally clears flag and resets display boundaries on exit)
		 */
		if (Global_IsMenuOpen)
		{
			OSC_voidOpenMainMenu();
		}

		/*	if info drawing time has passed, draw info	*/
		if (STK_u64GetElapsedTicks() - lastInfoDrawTime > infoDrawPeriod)
		{
			/*	draw info on screen	*/
			OSC_voidDrawInfo();
			/*	update timestamp	*/
			lastInfoDrawTime = STK_u64GetElapsedTicks();
		}

		/*	only if device is not paused, take new samples	*/
		if (!Global_Paused)
		{
			OSC_voidTakeNewSamples();
		}

		/*	draw a frame based on the current running mode	*/
		switch(Global_CurrentRunningMode)
		{
		case OSC_RunningMode_Normal:
			OSC_voidDrawNormalModeFrame();
			break;
		}
	}
}

