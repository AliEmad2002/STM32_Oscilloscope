/*
 * Oscilloscope_Draw.h
 *
 *  Created on: Feb 16, 2023
 *      Author: ali20
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_DRAW_H_
#define INCLUDE_APP_OSCILLOSCOPE_DRAW_H_

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

void OSC_voidSetDisplayBoundariesForSignalArea(void);

void OSC_voidFillDisplayWithBGColor(void);

void OSC_voidEditStrtingOnDislpay(char* str);

#endif /* INCLUDE_APP_OSCILLOSCOPE_DRAW_H_ */
