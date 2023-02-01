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
#include "Oscilloscope_init_Global.h"
#include "Oscilloscope_init_MCAL.h"
#include "Oscilloscope_init_HAL.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_Menu_Interface.h"
#include "Oscilloscope_interface.h"

/*******************************************************************************
 * Extern global variables (from private.c file):
 ******************************************************************************/
extern volatile u32 stkTicksPerSecond;

extern volatile TFT2_t Global_LCD;
extern volatile u16* Global_ImgBufferArr[2];
extern volatile u8 Global_PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_LargestVlaueInCurrentFrame;
extern volatile u8 Global_SmallestVlaueInCurrentFrame;
extern volatile u32 Global_CurrentMicroVoltsPerPix;
extern volatile u64 Global_CurrentNanoSecondsPerPix;
extern volatile u8 Global_CurrentUsedAdcChannelIndex;
extern volatile b8 Global_Paused;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_IsMenuOpen;
extern volatile u16 Global_SampleBuffer[NUMBER_OF_SAMPLES];
extern volatile char Global_Str[128];
extern volatile u16 Global_InfoImg[12 * 5 * 8];
//extern volatile u16 Global_PixArr[2 * LINES_PER_IMAGE_BUFFER * 128];
extern volatile u16 Global_PixArr[32 * 128];

extern volatile Rotary_Encoder_t OSC_RotaryEncoder;

extern volatile OSC_Cursor_t Cursor_v1;
extern volatile OSC_Cursor_t Cursor_v2;
extern volatile OSC_Cursor_t Cursor_t1;
extern volatile OSC_Cursor_t Cursor_t2;

void OSC_voidEnterMathMode(void)
{

}

/*******************************************************************************
 * Div. control:
 ******************************************************************************/
void OSC_voidIncrementVoltageDiv(void)
{
	u32 currentMilliVoltsPerDiv =
		(Global_CurrentMicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV) / 1000;

	for (u8 i = 0; i < NUMBER_OF_VOLT_DIVS; i++)
	{
		if (currentMilliVoltsPerDiv <= OSC_mVoltsPerDivArr[i])
		{
			if (i < NUMBER_OF_VOLT_DIVS - 1)
				Global_CurrentMicroVoltsPerPix =
					(OSC_mVoltsPerDivArr[i + 1] * 1000) /
					PIXELS_PER_VOLTAGE_DIV;

			return;
		}
	}
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
}

void OSC_voidDecrementVoltageDiv(void)
{
	u32 currentMilliVoltsPerDiv =
		(Global_CurrentMicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV) / 1000;

	for (u8 i = 0; i < NUMBER_OF_VOLT_DIVS; i++)
	{
		if (currentMilliVoltsPerDiv <= OSC_mVoltsPerDivArr[i])
		{
			if (i > 0)
				Global_CurrentMicroVoltsPerPix =
					(OSC_mVoltsPerDivArr[i - 1] * 1000) /
					PIXELS_PER_VOLTAGE_DIV;

			return;
		}
	}
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
}

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

/*******************************************************************************
 * ISR's:
 ******************************************************************************/
void OSC_voidTrigPauseResume(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
	{
		return;
	}

	if (!Global_Paused)
	{
		Global_Paused = true;
		GPIO_SET_PIN_HIGH(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);
	}

	else
	{
		Global_Paused = false;
		GPIO_SET_PIN_LOW(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);
	}

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}

void OSC_voidUpButtonCallBack(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		ROTARY_DEBOUNCING_TIME_MS * 72000ul)
	{
		return;
	}

	switch (Global_UpDownTarget)
	{
	case OSC_Up_Down_Target_ChangeVoltageDiv:
		OSC_voidIncrementVoltageDiv();
		break;

	case OSC_Up_Down_Target_ChangeTimeDiv:
		OSC_voidIncrementTimeDiv();
		break;

	case OSC_Up_Down_Target_ChangeVoltageCursor1Position:
		OSC_voidIncrementCursorV1();
		break;

	case OSC_Up_Down_Target_ChangeVoltageCursor2Position:
		OSC_voidIncrementCursorV2();
		break;

	case OSC_Up_Down_Target_ChangeTimeCursor1Position:
		OSC_voidIncrementCursorT1();
		break;

	case OSC_Up_Down_Target_ChangeTimeCursor2Position:
		OSC_voidIncrementCursorT2();
		break;

	case OSC_Up_Down_Target_ChangeBrightness:
		OSC_voidIncrementBrightness();
		break;
	}

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}

void OSC_voidDownButtonCallBack(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		ROTARY_DEBOUNCING_TIME_MS * 72000ul)
	{
		return;
	}

	switch (Global_UpDownTarget)
	{
	case OSC_Up_Down_Target_ChangeVoltageDiv:
		OSC_voidDecrementVoltageDiv();
		break;

	case OSC_Up_Down_Target_ChangeTimeDiv:
		OSC_voidDecrementTimeDiv();
		break;

	case OSC_Up_Down_Target_ChangeVoltageCursor1Position:
		OSC_voidDecrementCursorV1();
		break;

	case OSC_Up_Down_Target_ChangeVoltageCursor2Position:
		OSC_voidDecrementCursorV2();
		break;

	case OSC_Up_Down_Target_ChangeTimeCursor1Position:
		OSC_voidDecrementCursorT1();
		break;

	case OSC_Up_Down_Target_ChangeTimeCursor2Position:
		OSC_voidDecrementCursorT2();
		break;

	case OSC_Up_Down_Target_ChangeBrightness:
		OSC_voidDecrementBrightness();
		break;
	}

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}

void OSC_voidMenuButtonCallback(void)
{
	/*	check debouncing first	*/
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
	{
		return;
	}

	/*	set opened menu flag	*/
	Global_IsMenuOpen = true;

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}

/*******************************************************************************
 * Main thread functions:
 ******************************************************************************/
inline void OSC_voidWaitForSignalRisingEdge(void)
{
	/*
	 * Wait for a rising edge of the signal before starting sampling. This gives
	 * a much nicer display because of syncing signal and display.
	 *
	 * Timeout: 0.1 second.
	 */
	u64 startTime = STK_u64GetElapsedTicks();

	TIM_CLEAR_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1);

	while(!TIM_b8GetStatusFlag(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
	{
		if (STK_u64GetElapsedTicks() - startTime > stkTicksPerSecond / 10)
			break;
	}
}

void OSC_voidPreFillSampleBuffer(void)
{
	/*	disable trigger timer counter	*/
	TIM_DISABLE_COUNTER(3);

	/*	set freq (one sample per pixel)	*/
	static u64 lastFreq = 0;
	u64 freqSamplingmHz = 1e12 / Global_CurrentNanoSecondsPerPix;

	if (freqSamplingmHz != lastFreq)
	{
		/*	set timer3 frequency to sampling frequency	*/
		TIM_u64SetFrequency(3, freqSamplingmHz);

		lastFreq = freqSamplingmHz;
	}
}

inline void OSC_voidStartFillingSampleBuffer(void)
{
	/*	start sample capturing	*/
	DMA_voidEnableChannel(DMA_UnitNumber_1, ADC_DMA_CHANNEL);

	/*	enable ADC trigger source	*/
		TIM_ENABLE_COUNTER(3);
}

void OSC_voidWaitSampleBufferFill(void)
{
	/*	wait for sampling complete, disable DMA channel	*/
	DMA_voidWaitTillChannelIsFreeAndDisableIt(
		DMA_UnitNumber_1, ADC_DMA_CHANNEL);

	/*	disable trigger timer counter	*/
	TIM_DISABLE_COUNTER(3);
}

/*
 * when t_pix  < t_conv, interpolate / predict samples in between real samples.
 */
void OSC_voidInterpolate(void)
{
	u16 sampleBufferInterpolated[NUMBER_OF_SAMPLES];

	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		volatile u64 t = i * Global_CurrentNanoSecondsPerPix;

		volatile u8 j = t  / 1000;

		volatile s64 t1 = j * 1000;

		volatile s16 s1 = Global_SampleBuffer[j];
		volatile s16 s2 = Global_SampleBuffer[j + 1];

		volatile s16 s = (s64)((s2 - s1) * (t - t1)) / 1000 + s1;

		if (s > 4000)
			s = 4000;
		else if (s < 10)
			s = 10;

		sampleBufferInterpolated[i] = (u16)s;
	}

	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		Global_SampleBuffer[i] = sampleBufferInterpolated[i];
	}
}

void OSC_voidMainSuperLoop(void)
{
	/*	voltage in pixels	*/
	u8 lastRead = 0;
	u8 currentRead = 0;

	u8 currentSmaller;
	u8 currentLarger;

	/*	last time info was drawn. (timestamp)	*/
	u64 lastInfoDrawTime = 0;

	/*	periodic time to draw info (in STK ticks)	*/
	volatile u64 infoDrawPeriod =
		INFO_DRAWING_PERIODIC_TIME_MS * (u64)stkTicksPerSecond / 1000ul;

	u8 j = 0;

	u64 lastFrameTimeStamp = 0;

	while (1)
	{
		/*	wait for frame time to come (according to configured FPS)	*/
		u64 timeSinceLastFrame = STK_u64GetElapsedTicks() - lastFrameTimeStamp;

		while(timeSinceLastFrame < stkTicksPerSecond / LCD_FPS)
			timeSinceLastFrame = STK_u64GetElapsedTicks() - lastFrameTimeStamp;

		lastFrameTimeStamp = STK_u64GetElapsedTicks();

		/*
		 * if user opened menu, enter it.
		 * (menu internally clears flag and resets display boundaries on exit)
		 */
		if (Global_IsMenuOpen)
		{
			OSC_voidOpenMainMenu();
		}

		/*	only if device is not paused	*/
		if (!Global_Paused)
		{
			/*	if info drawing time has passed	*/
			if (STK_u64GetElapsedTicks() - lastInfoDrawTime > infoDrawPeriod)
			{
				/*	draw info in screen	*/
				OSC_voidDrawInfo();
				/*	update timestamp	*/
				lastInfoDrawTime = STK_u64GetElapsedTicks();
			}

			/*	prepare ADC for sampling at wanted speed	*/
			OSC_voidPreFillSampleBuffer();

			/*	sync on signal rising edge	*/
			OSC_voidWaitForSignalRisingEdge();

			/*	start filling sample buffer	*/
			OSC_voidStartFillingSampleBuffer();

			/*	wait sample buffer fill	*/
			OSC_voidWaitSampleBufferFill();

			/*
			 * interpolate samples if and only if: t_pix < t_conv,
			 * i.e.: t_pix < 1uS
			 */
			if (Global_CurrentNanoSecondsPerPix < 1000)
				OSC_voidInterpolate();
		}

		/*	counter of the processed samples of current image frame	*/
		u8 readCount = 0;

		/*	draw current image frame of screen	*/
		while(1)
		{
			/*	Draw/process one of the two image buffers	*/
			for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i++)
			{
				/*
				 * read ADC converted value
				 * Eqn.: voltage_in_pix = voltage_in_volt * N_pixels_per_volt
				 *  = 10^5 * 33 * Vadc / microVoltsPerPix / 4096
				 */
				u64 currentReadU64 =
					((3300000ul * (u64)Global_SampleBuffer[readCount]) /
					(u64)Global_CurrentMicroVoltsPerPix) >> 12;

				if (currentReadU64 > 127)
				{
					currentRead = 127;
				}
				else
				{
					currentRead = (u8)currentReadU64;
				}

				/*
				 * find the smaller and the larger of 'currentRead' and
				 * 'lastRead'
				 */
				if (currentRead > lastRead)
				{
					currentSmaller = lastRead;
					currentLarger = currentRead;
				}
				else
				{
					currentSmaller = currentRead;
					currentLarger = lastRead;
				}

				lastRead = currentRead;

				/*
				 * wait for DMA 1st, 2nd, 3rd channels transfer complete
				 */
				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);

				/*
				 * draw background color from zero to just before smaller
				 */
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
					&Global_ImgBufferArr[j][i * 128]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
					currentSmaller);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw main color from smaller to larger	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
					&Global_ImgBufferArr[j][i * 128 + currentSmaller]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
					currentLarger - currentSmaller + 1);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw background color from after larger to 127	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
					&Global_ImgBufferArr[j][i * 128 + currentLarger + 1]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
					127 - currentLarger);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw time cursors (if any)	*/
				if (Cursor_t1.isEnabled && Cursor_t1.pos == readCount)
				{
					u8 lineNumber = Cursor_t1.pos % LINES_PER_IMAGE_BUFFER;

					for (u8 k = 0; k < 128; k += SUM_OF_DASH_LEN)
					{
						for (u8 l = 0; l < 3 && k < 128; l++)
						{
							u16 index = lineNumber * 128 + k + l;
							Global_ImgBufferArr[j][index] =
								LCD_CURSOR1_DRAWING_COLOR_U16;
						}

					}
				}

				if (Cursor_t2.isEnabled && Cursor_t2.pos == readCount)
				{
					u8 lineNumber = Cursor_t2.pos % LINES_PER_IMAGE_BUFFER;

					for (u8 k = 0; k < 128; k += SUM_OF_DASH_LEN)
					{
						for (u8 l = 0; l < 3 && k < 128; l++)
						{
							u16 index = lineNumber * 128 + k + l;
							Global_ImgBufferArr[j][index] =
								LCD_CURSOR2_DRAWING_COLOR_U16;
						}

					}
				}

				readCount++;
			}

			/*	draw voltage cursors (if any)	*/
			if (Cursor_v1.isEnabled)
			{
				for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i += SUM_OF_DASH_LEN)
				{
					for (u8 k = 0; k < 3; k++)
					{
						u16 index = (i + k) * 128 + Cursor_v1.pos;

						Global_ImgBufferArr[j][index] =
							LCD_CURSOR1_DRAWING_COLOR_U16;
					}
				}
			}

			if (Cursor_v2.isEnabled)
			{
				for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i += SUM_OF_DASH_LEN)
				{
					for (u8 k = 0; k < 3; k++)
					{
						u16 index = (i + k) * 128 + Cursor_v2.pos;

						Global_ImgBufferArr[j][index] =
							LCD_CURSOR2_DRAWING_COLOR_U16;
					}
				}
			}

			/*	Send them using DMA (Internally waits for DMA TC)	*/
			TFT2_voidSendPixels(
				(TFT2_t*)&Global_LCD, (u16*)Global_ImgBufferArr[j],
				LINES_PER_IMAGE_BUFFER * 128ul);

			if (j == 0)
				j = 1;
			else // if (j == 1)
				j = 0;

			if (readCount == NUMBER_OF_SAMPLES)
				break;
		}
	}
}

void OSC_voidSetDisplayBoundariesForSignalArea(void)
{
	/*	set screen boundaries for full signal image area	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, NUMBER_OF_SAMPLES - 1);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);
}

void OSC_voidGetNumberPritableVersion(
	volatile u64 valInNano, u32* valInteger, u32* valFraction, char* unitPrefix)
{
	if (valInNano < 1e3)
	{
		*unitPrefix = 'n';
		*valInteger = valInNano;
		*valFraction = 0;
	}
	else if (valInNano < 1e6)
	{
		*unitPrefix = 'u';
		*valInteger = valInNano / 1e3;
		*valFraction = (valInNano - 1e3) / 1e2;
	}
	else if (valInNano < 1e9)
	{
		*unitPrefix = 'm';
		*valInteger = valInNano / 1e6;
		*valFraction = (valInNano - 1e6) / 1e5;
	}
	else if (valInNano < 1e12)
	{
		*unitPrefix = ' ';
		*valInteger = valInNano / 1e9;
		*valFraction = (valInNano - 1e9) / 1e8;
	}
	else if (valInNano < 1e15)
	{
		*unitPrefix = 'k';
		*valInteger = valInNano / 1e12;
		*valFraction = (valInNano - 1e12) / 1e11;
	}
	else if (valInNano < 1e18)
	{
		*unitPrefix = 'M';
		*valInteger = valInNano / 1e15;
		*valFraction = (valInNano - 1e15) / 1e14;
	}

	/*	fraction is maximumly of 1 digit	*/
	while (*valFraction > 10)
		*valFraction /= 10;
}

void OSC_voidGetInfoStr(char* str)
{
	/**	Frequency	**/
	/*
	 * read frequency value (this reading is based on the value of OC register
	 * at time of reading).
	 */
	u64 freqmHz = 0;
	/*
	 * if CC1IF was not raised before reading CCR1, then no transition have
	 * not happened. i.e.: freq = 0
	 */
	if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	char freqUnitPrefix;
	u32 freqInteger, freqFraction;

	OSC_voidGetNumberPritableVersion(
		freqmHz * 1000000, &freqInteger, &freqFraction, &freqUnitPrefix);

	/**	peak to peak voltage	**/
	u64 vpp =
		Global_PeakToPeakValueInCurrentFrame * Global_CurrentMicroVoltsPerPix;

	char vppUnitPrefix;
	u32 vppInteger, vppFraction;

	OSC_voidGetNumberPritableVersion(
		vpp * 1000, &vppInteger, &vppFraction, &vppUnitPrefix);

	/**	volts per div	**/
	u64 voltsPerDiv = Global_CurrentMicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV;

	char voltsPerDivUnitPrefix;
	u32 voltsPerDivInteger, voltsPerDivFraction;

	OSC_voidGetNumberPritableVersion(
		voltsPerDiv * 1000, &voltsPerDivInteger,
		&voltsPerDivFraction, &voltsPerDivUnitPrefix);

	/**	seconds per div	**/
	u64 secondsPerDiv = Global_CurrentNanoSecondsPerPix * PIXELS_PER_TIME_DIV;

	char secondsPerDivUnitPrefix;
	u32 secondsPerDivInteger, secondsPerDivFraction;

	OSC_voidGetNumberPritableVersion(
		secondsPerDiv, &secondsPerDivInteger,
		&secondsPerDivFraction, &secondsPerDivUnitPrefix);

	/**	v1	**/
	u64 v1 = Cursor_v1.pos * Global_CurrentMicroVoltsPerPix;

	char v1UnitPrefix;
	u32 v1Integer, v1Fraction;

	OSC_voidGetNumberPritableVersion(
		v1 * 1000, &v1Integer,
		&v1Fraction, &v1UnitPrefix);

	/**	v2	**/
	u64 v2 = Cursor_v2.pos * Global_CurrentMicroVoltsPerPix;

	char v2UnitPrefix;
	u32 v2Integer, v2Fraction;

	OSC_voidGetNumberPritableVersion(
		v2 * 1000, &v2Integer,
		&v2Fraction, &v2UnitPrefix);

	/**	t1	**/
	u64 t1 = Cursor_t1.pos * Global_CurrentNanoSecondsPerPix;

	char t1UnitPrefix;
	u32 t1Integer, t1Fraction;

	OSC_voidGetNumberPritableVersion(
		t1, &t1Integer,
		&t1Fraction, &t1UnitPrefix);

	/**	t2	**/
	u64 t2 = Cursor_t2.pos * Global_CurrentNanoSecondsPerPix;

	char t2UnitPrefix;
	u32 t2Integer, t2Fraction;

	OSC_voidGetNumberPritableVersion(
		t2, &t2Integer,
		&t2Fraction, &t2UnitPrefix);

	sprintf(
		(char*)str,
		"F=%u.%u%cHz Vpp=%u.%u%cV\nVd=%u.%u%cV td=%u.%u%cS\nV1=%u.%u%cV t1=%u.%u%cS\nV2=%u.%u%cV t2=%u.%u%cS",
		freqInteger, freqFraction, freqUnitPrefix,
		vppInteger, vppFraction, vppUnitPrefix,
		voltsPerDivInteger, voltsPerDivFraction, voltsPerDivUnitPrefix,
		secondsPerDivInteger, secondsPerDivFraction, secondsPerDivUnitPrefix,
		v1Integer, v1Fraction, v1UnitPrefix,
		t1Integer, t1Fraction, t1UnitPrefix,
		v2Integer, v2Fraction, v2UnitPrefix,
		t2Integer, t2Fraction, t2UnitPrefix
	);
}

void OSC_voidDrawFrequencyInfo(void)
{
	static u64 lastFreqmHz = 0;
	/**
	 * read frequency value (this reading is based on the value of OC register
	 * at time of reading).
	 **/
	u64 freqmHz = 0;
	/*
	 * if CC1IF was not raised before reading CCR1, then no transition have
	 * not happened. i.e.: freq = 0
	 */
	if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	/*	draw only if value has changed	*/
	if (freqmHz == lastFreqmHz)
		return;

	/**	print value on char array	**/
	char freqUnitPrefix;
	u32 freqInteger, freqFraction;

	OSC_voidGetNumberPritableVersion(
		freqmHz * 1000000, &freqInteger, &freqFraction, &freqUnitPrefix);

	sprintf(
		Global_Str, "F=%u.%u%cHz", freqInteger, freqFraction, freqUnitPrefix);

	/*	print char array on image buffer	*/
	Txt_voidCpyStrToStaticPixArrNormalOrientation(
		(u8*)Global_Str, LCD_BACKGROUND_COLOR_U16, LCD_MAIN_DRAWING_COLOR_U16,
		1, Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
		0, 0, 8, 60, (u16(*)[])Global_InfoImg);

	/**	send image buffer to display TFT	**/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 12 * 5);
	TFT2_SET_Y_BOUNDARIES(
		&Global_LCD, NUMBER_OF_SAMPLES, NUMBER_OF_SAMPLES + 8);

	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	send info image	*/
	TFT2_voidSendPixels(&Global_LCD, (u16*)Global_InfoImg, 12 * 5 * 8);

	/*	wait for transfer complete	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

	/*	update "last" value	*/
	lastFreqmHz = freqmHz;
}

void OSC_voidDrawInfo(void)
{
	/*	wait for transfer complete	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

	/*	the string that info is stored at before drawing	*/
	char str[128];

	OSC_voidGetInfoStr(str);

	Txt_voidCpyStrToStaticPixArrNormalOrientation(
		(u8*)str, colorRed.code565, colorBlack.code565, 1,
		Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
		0, 0, 32, 128, (u16(*)[])Global_PixArr);

	/*	set screen boundaries for info image area	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, NUMBER_OF_SAMPLES, 159);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	send info image	*/
	TFT2_voidSendPixels(
		&Global_LCD, (u16*)Global_ImgBufferArr[0],
		32 * 128ul);

	/*	wait for transfer complete	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

	/*	set screen boundaries for full signal image area	*/
	OSC_voidSetDisplayBoundariesForSignalArea();
}

void OSC_voidAutoCalibrate(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
	{
		return;
	}

	/** Time calibration	**/
	/*
	 * Get signal frequency.
	 * Wait for CC1IF to raise, as it may be cleared by a previous frequency
	 * read, which would lead to a fault measurement of a zero frequency.
	 */
	u64 freqmHz = 0;
	/*
	 * if CC1IF was not raised before reading CCR1, then no transition have
	 * not happened. i.e.: freq = 0
	 */
	u64 startTime = STK_u64GetElapsedTicks();

	while(!TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
	{
		if (
			STK_u64GetElapsedTicks() - startTime >
			FREQ_MEASURE_TIMEOUT_MS * 72000
		)
			break;
	}

	if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz =
			TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	/*
	 * set time per pix such that user can see 3 periods of the signal in one
	 * frame.
	 * eqn: time_per_pix = 3 * T / 120
	 * where T is the periodic time of the signal,
	 * 120 is the width of the image.
	 */
	if (freqmHz == 0)
		Global_CurrentNanoSecondsPerPix = 1000;
	else
		Global_CurrentNanoSecondsPerPix =
			1000000000000ul / freqmHz / 40;

	/**	Gain and voltage calibration	**/
	/*	make ADC run in continuous mode	*/
	ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_1);

	/*	make external trigger source: SW trigger	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_SWSTART);

	/*	trigger start of conversion	*/
	ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);

	ADC_ChannelNumber_t adcCh;
	u8 i = 0;

	for (; i < CHANNEL_1_NUMBER_OF_LEVELS; i++)
	{
		adcCh = oscCh1AdcChannels[i].adcChannelNumber;

		/*	make 'adcCh' the one to be converted and watchdog-ed	*/
		ADC_voidSetSequenceRegular(
			ADC_UnitNumber_1, ADC_RegularSequenceNumber_1, adcCh);

		/*	Clear AWD flag, in case previous rise	*/
		ADC_voidClearStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_AWD);

		/*
		 * wait for 2 periods of this signal, at least 1 ms, and maximumly 500ms
		 */
		u32 delayTimeMs = 2000000u / freqmHz;
		if (delayTimeMs == 0)
			delayTimeMs = 1;
		else if (delayTimeMs > 500)
			delayTimeMs = 500;

		Delay_voidBlockingDelayMs(delayTimeMs);

		/*	check analog watchdog flag. If it was set, then this gain is not
		 * the proper one, clear AWD flag and continue in the loop.
		 */
		if (ADC_b8GetStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_AWD) == true)
		{
			ADC_voidClearStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_AWD);
			continue;
		}
		/*	otherwise, break	*/
		else
		{
			break;
		}
	}

	/*	if the proper gain was not found in loop, increment 'i'	*/
	if (i == CHANNEL_1_NUMBER_OF_LEVELS)
		i--;

	/*	tell ADC that this is the channel to be converted	*/
	ADC_voidSetSequenceRegular(
		ADC_UnitNumber_1, ADC_RegularSequenceNumber_1, adcCh);

	/*	disable ADC continuous mode	*/
	ADC_voidEnableSingleConversionMode(ADC_UnitNumber_1);

	/*	make external trigger source: TIM3TRGO	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_TIM3TRGO);

	/**	set global parameters	**/
	Global_CurrentMicroVoltsPerPix =
		(oscCh1AdcChannels[i].maxVPPinMilliVolts * 1000ul) / 128;

	Global_CurrentUsedAdcChannelIndex = i;

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}
