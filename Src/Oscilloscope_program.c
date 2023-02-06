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
extern volatile u8 Global_Ch1PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch1MinValueInCurrentFrame;
extern volatile u8 Global_Ch1MaxValueInCurrentFrame;
extern volatile u8 Global_Ch2PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch2MinValueInCurrentFrame;
extern volatile u8 Global_Ch2MaxValueInCurrentFrame;
extern volatile u32 Global_CurrentCh1MicroVoltsPerPix;
extern volatile u32 Global_CurrentCh2MicroVoltsPerPix;
extern volatile u64 Global_CurrentNanoSecondsPerPix;
extern volatile b8 Global_Paused;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_IsMenuOpen;
extern volatile u16 Global_SampleBuffer[2 * NUMBER_OF_SAMPLES];
extern volatile char Global_Str[128];
extern volatile u16 Global_InfoImg[12 * 5 * 8];
//extern volatile u16 Global_PixArr[2 * LINES_PER_IMAGE_BUFFER * 128];
extern volatile u16 Global_PixArr[32 * 128];
extern volatile b8 Global_IsCh1Enabled;
extern volatile b8 Global_IsCh2Enabled;
extern volatile u8 Global_LastRead1;
extern volatile u8 Global_LastRead2;
extern volatile u8 Global_Smaller1;
extern volatile u8 Global_Larger1;
extern volatile u8 Global_Smaller2;
extern volatile u8 Global_Larger2;
extern volatile s8 Global_Offset1;
extern volatile s8 Global_Offset2;
extern volatile OSC_RunningMode_t Global_CurrentRunningMode;

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

void OSC_voidIncrementCh1Offset(void)
{
	if (Global_IsCh1Enabled)
	{
		Global_Offset1++;
	}
}

void OSC_voidIncrementCh2Offset(void)
{
	if (Global_IsCh2Enabled)
	{
		Global_Offset2++;
	}
}

void OSC_voidDecrementCh1Offset(void)
{
	if (Global_IsCh1Enabled)
	{
		Global_Offset1--;
	}
}

void OSC_voidDecrementCh2Offset(void)
{
	if (Global_IsCh2Enabled)
	{
		Global_Offset2--;
	}
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
	case OSC_Up_Down_Target_ChangeCh1VoltageDiv:
		OSC_voidIncrementCh1VoltageDiv();
		break;

	case OSC_Up_Down_Target_ChangeCh2VoltageDiv:
		OSC_voidIncrementCh2VoltageDiv();
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

	case OSC_Up_Down_Target_ChangeCh1Offset:
		OSC_voidIncrementCh1Offset();
		break;

	case OSC_Up_Down_Target_ChangeCh2Offset:
		OSC_voidIncrementCh2Offset();
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
	case OSC_Up_Down_Target_ChangeCh1VoltageDiv:
		OSC_voidDecrementCh1VoltageDiv();
		break;

	case OSC_Up_Down_Target_ChangeCh2VoltageDiv:
		OSC_voidDecrementCh2VoltageDiv();
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

	case OSC_Up_Down_Target_ChangeCh1Offset:
		OSC_voidDecrementCh1Offset();
		break;

	case OSC_Up_Down_Target_ChangeCh2Offset:
		OSC_voidDecrementCh2Offset();
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
//	/*	check debouncing first	*/
//	static u64 lastPressTime = 0;
//
//	if (
//		STK_u64GetElapsedTicks() - lastPressTime <
//		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
//	{
//		return;
//	}

	/*	set opened menu flag	*/
	Global_IsMenuOpen = true;

//	/*	debouncing timestamp	*/
//	lastPressTime = STK_u64GetElapsedTicks();
}

void OSC_voidAutoEnterMenuButtonCallback(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
	{
		return;
	}

	/*
	 * wait 500ms, if button was not yet released enter menu,
	 * otherwise do auto calibration
	 */
	Delay_voidBlockingDelayMs(BUTTON_DEBOUNCING_TIME_MS * 2);

	if (
		GPIO_DIGITAL_READ(
			BUTTON_AUTO_ENTER_MENU_PIN / 16, BUTTON_AUTO_ENTER_MENU_PIN % 16)
	)
	{
		/*	wait for button release	*/
		while (
			GPIO_DIGITAL_READ(
				BUTTON_AUTO_ENTER_MENU_PIN / 16,
				BUTTON_AUTO_ENTER_MENU_PIN % 16)
		);

		/*	debouncing	*/
		Delay_voidBlockingDelayMs(BUTTON_DEBOUNCING_TIME_MS);

		/*	execute menu callback	*/
		OSC_voidMenuButtonCallback();
	}

	else
	{
		OSC_voidAutoCalibrate();
	}

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}

/*******************************************************************************
 * channel on/off:
 ******************************************************************************/
void OSC_voidEnableCh1(void)
{
	Global_IsCh1Enabled = true;
}

void OSC_voidEnableCh2(void)
{
	Global_IsCh2Enabled = true;
}

void OSC_voidDisableCh1(void)
{
	Global_IsCh1Enabled = false;
}

void OSC_voidDisableCh2(void)
{
	Global_IsCh2Enabled = false;
}
/*******************************************************************************
 * Main thread functions:
 ******************************************************************************/
inline void OSC_voidWaitForSignalRisingEdge(void)
{
	/*	normally sync on ch1 input, unless disabled, then sync on ch2 input	*/
	u8 syncTimUnit;
	if (Global_IsCh1Enabled)
		syncTimUnit = FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER;
	else
		syncTimUnit = FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER;

	/*
	 * Wait for a rising edge of the signal before starting sampling. This gives
	 * a much nicer display because of syncing signal and display.
	 *
	 * Timeout: 0.1 second.
	 */
	u64 startTime = STK_u64GetElapsedTicks();

	TIM_CLEAR_STATUS_FLAG(syncTimUnit, TIM_Status_CC1);

	while(!TIM_b8GetStatusFlag(syncTimUnit, TIM_Status_CC1))
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
		if (freqSamplingmHz > ADC_MAX_SAMPLING_FREQUENCY_MILLI_HZ)
		{
			freqSamplingmHz = ADC_MAX_SAMPLING_FREQUENCY_MILLI_HZ;
		}

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

inline void OSC_voidTakeNewSamples(void)
{
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
	if (Global_CurrentNanoSecondsPerPix < ADC_MIN_CONV_TIME_NANO_SECOND)
		OSC_voidInterpolate();
}

/*
 * when t_pix  < t_conv, interpolate / predict samples in between real samples.
 */
void OSC_voidInterpolate(void)
{
	u16 sampleBufferInterpolated[NUMBER_OF_SAMPLES];

	/*	interpolate channel1	*/
	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		volatile u64 t = i * Global_CurrentNanoSecondsPerPix;

		volatile u8 j = t  / ADC_MIN_CONV_TIME_NANO_SECOND;

		volatile s64 t1 = j * ADC_MIN_CONV_TIME_NANO_SECOND;

		volatile s16 s1 = Global_SampleBuffer[2 * j];
		volatile s16 s2 = Global_SampleBuffer[2 * j + 2];

		volatile s16 s =
			(s64)((s2 - s1) * (t - t1)) / ADC_MIN_CONV_TIME_NANO_SECOND + s1;

		if (s > 4000)
			s = 4000;
		else if (s < 10)
			s = 10;

		sampleBufferInterpolated[i] = (u16)s;
	}

	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		Global_SampleBuffer[2 * i] = sampleBufferInterpolated[i];
	}

	/*	interpolate channel2	*/
	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		volatile u64 t = i * Global_CurrentNanoSecondsPerPix;

		volatile u8 j = t  / ADC_MIN_CONV_TIME_NANO_SECOND;

		volatile s64 t1 = j * ADC_MIN_CONV_TIME_NANO_SECOND;

		volatile s16 s1 = Global_SampleBuffer[2 * j + 1];
		volatile s16 s2 = Global_SampleBuffer[2 * j + 3];

		volatile s16 s =
			(s64)((s2 - s1) * (t - t1)) / ADC_MIN_CONV_TIME_NANO_SECOND + s1;

		if (s > 4000)
			s = 4000;
		else if (s < 10)
			s = 10;

		sampleBufferInterpolated[i] = (u16)s;
	}

	for (u8 i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		Global_SampleBuffer[2 * i + 1] = sampleBufferInterpolated[i];
	}
}

/*
* Eqn.:
* "v_ch" is the very main input that user can handle.
* "v_opamp" is op-amp's output voltage.
* "v_adc" is the converted 12-bit value by internal ADC.
*
* v_ch 	= 5 - v_opamp / 0.33			[volts]
* 		= 5 - 10 * v_adc / 4096			[volts]
*
* v_pix = v_ch * pixels_per_volt + 128 / 2
*
* 		 = 	5 * pixels_per_volt -
* 		 	(10 * pixels_per_volt * v_adc) / 4096 +
* 		 	64							[pixels]
*/
#define GET_CH1_V_IN_PIXELS(v_adc)                                           \
(	                                                                   	     \
	5 * 1000000 / Global_CurrentCh1MicroVoltsPerPix -						 \
	(10000000 * (s64)(v_adc)) / Global_CurrentCh1MicroVoltsPerPix / 4096 +   \
	63 + Global_Offset1                                                      \
)

#define GET_CH2_V_IN_PIXELS(v_adc)                                           \
(	                                                                   	     \
	5 * 1000000 / Global_CurrentCh2MicroVoltsPerPix -						 \
	(10000000 * (s64)(v_adc)) / Global_CurrentCh2MicroVoltsPerPix / 4096 +   \
	63 + Global_Offset2                                                      \
)

#define SORT_VALUES(new, smaller, larger)             \
{                                                     \
	if ((new) > (larger))                             \
	{                                                 \
		(larger) = (new);                             \
	}                                                 \
	                                                  \
	else if ((new) < (smaller))                       \
	{                                                 \
		(smaller) = (new);                            \
	}                                                 \
}

#define GET_SMALLER(a, b)		((a < b) ? a : b)

/*	if (b < a)	==>>	a = b	*/
#define WRITE_IF_SMALLER(a, b)	\
{                               \
	if ((b) < (a))              \
		(a) = (b);              \
}

#define GET_LARGER(a, b)		((a > b) ? a : b)

/*	if (b > a)	==>>	a = b	*/
#define WRITE_IF_LARGER(a, b)	\
{                               \
	if ((b) > (a))              \
		(a) = (b);              \
}

/*
 * fills a segment in selected line in selected image buffer with any color
 */
#define FILL_SEGMENT(imgBufferIndex, line, start, end, color)             \
{                                                                         \
	DMA_voidSetMemoryAddress(                                             \
		DMA_UnitNumber_1, LINE_SEGMENT_DMA_CHANNEL,                       \
		(void*)&Global_ImgBufferArr[(imgBufferIndex)][(line) * 128 + (start)]);  \
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

#define CLEAR_IMG_BUFFER(imgBufferIndex)	    \
{										        \
	FILL_SEGMENT(                               \
		(imgBufferIndex), 0, 0,                 \
		LINES_PER_IMAGE_BUFFER * 128 - 1,       \
		LCD_BACKGROUND_COLOR_U16                \
	)                                           \
}

#define WAIT_NEXT_FRAME_AND_UPDATE_TIMESTAMP(lastFrameTimeStamp)           \
{                                                                          \
	while(                                                                 \
		STK_u64GetElapsedTicks() - lastFrameTimeStamp <                    \
		stkTicksPerSecond / LCD_FPS                                        \
	);                                                                     \
                                                                           \
	(lastFrameTimeStamp) = STK_u64GetElapsedTicks();                       \
}

#define DRAW_TIME_CURSOR(imgBufferIndex, pos, color)			 \
{																 \
	u8 lineNumber = (pos) % LINES_PER_IMAGE_BUFFER;              \
                                                                 \
	for (u8 k = 0; k < 128; k += SUM_OF_DASH_LEN)                \
	{                                                            \
		for (u8 l = 0; l < 3 && k < 128; l++)                    \
		{                                                        \
			u16 index = lineNumber * 128 + k + l;                \
			Global_ImgBufferArr[(imgBufferIndex)][index] =       \
				(color);                                         \
		}                                                        \
	}                                                            \
}

#define DRAW_VOLTAGE_CURSOR(imgBufferIndex, pos, color)			        \
{                                                                       \
	for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i += SUM_OF_DASH_LEN)    \
	{                                                                   \
		for (u8 k = 0; k < 3; k++)                                      \
		{                                                               \
			u16 index = (i + k) * 128 + (pos);                          \
                                                                        \
			Global_ImgBufferArr[(imgBufferIndex)][index] =              \
				(color);                      							\
		}                                                               \
	}                                                                   \
}

#define DRAW_TIME_AXIS(imgBufferIndex)                      \
{                                                           \
	for (u16 i = 64; i < LINES_PER_IMAGE_BUFFER * 128; i+=128)     \
	{                                                       \
		Global_ImgBufferArr[(imgBufferIndex)][i] =          \
			LCD_AXIS_DRAWING_COLOR_U16;                     \
	}                                                       \
}

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

	/*	index of the image buffer to use	*/
	u8 imgBufferIndex = 0;

	/*
	 * draw current image frame of screen	*/
	while(1)
	{
		/*	clear image buffer number 'j'	*/
		CLEAR_IMG_BUFFER(imgBufferIndex);

		/*	draw time axis (voltage = 0)	*/
		DRAW_TIME_AXIS(imgBufferIndex);

		/*	Draw/process image buffer number 'j'	*/
		for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i++)
		{
			/*	read ADC converted value and check range	*/
			if (Global_IsCh1Enabled)
			{
				currentRead1Pix =
					GET_CH1_V_IN_PIXELS(Global_SampleBuffer[2 * readCount]);
			}

			if (currentRead1Pix < 0 || currentRead1Pix > 127)
				isRead1InRange = false;
			else
				isRead1InRange = true;

			if (Global_IsCh2Enabled)
			{
				currentRead2Pix =
					GET_CH2_V_IN_PIXELS(Global_SampleBuffer[2 * readCount + 1]);
			}

			if (currentRead2Pix < 0 || currentRead2Pix > 127)
				isRead2InRange = false;
			else
				isRead2InRange = true;

			/*	peak to peak calculation	*/
			WRITE_IF_SMALLER(Global_Ch1MinValueInCurrentFrame, currentRead1Pix);
			WRITE_IF_LARGER(Global_Ch1MaxValueInCurrentFrame, currentRead1Pix);

			WRITE_IF_SMALLER(Global_Ch2MinValueInCurrentFrame, currentRead2Pix);
			WRITE_IF_LARGER(Global_Ch2MaxValueInCurrentFrame, currentRead2Pix);

			/*	sort readings and previous readings	 */
			if (Global_IsCh1Enabled && isRead1InRange)
			{
				Global_Smaller1 = GET_SMALLER(currentRead1Pix, Global_LastRead1);
				Global_Larger1 = GET_LARGER(currentRead1Pix, Global_LastRead1);
				Global_LastRead1 = currentRead1Pix;
			}

			if (Global_IsCh2Enabled && isRead2InRange)
			{
				Global_Smaller2 = GET_SMALLER(currentRead2Pix, Global_LastRead2);
				Global_Larger2 = GET_LARGER(currentRead2Pix, Global_LastRead2);
				Global_LastRead2 = currentRead2Pix;
			}

			/*	draw secondary color from "smaller2" to "larger2"	*/
			if (Global_IsCh2Enabled && isRead2InRange)
			{
				FILL_SEGMENT(
					imgBufferIndex, i, Global_Smaller2, Global_Larger2,
					LCD_SECONDARY_DRAWING_COLOR_U16);
			}

			/*	draw main color from "smaller1" to "larger1"	*/
			if (Global_IsCh1Enabled && isRead1InRange)
			{
				FILL_SEGMENT(
					imgBufferIndex, i, Global_Smaller1, Global_Larger1,
					LCD_MAIN_DRAWING_COLOR_U16);
			}

			/*	draw time cursors (if any)	*/
			if (Cursor_t1.isEnabled && Cursor_t1.pos == readCount)
			{
				DRAW_TIME_CURSOR(
					imgBufferIndex, readCount, LCD_CURSOR1_DRAWING_COLOR_U16);
			}

			if (Cursor_t2.isEnabled && Cursor_t2.pos == readCount)
			{
				DRAW_TIME_CURSOR(
					imgBufferIndex, readCount, LCD_CURSOR2_DRAWING_COLOR_U16);
			}

			/*	increment readCount	*/
			readCount++;
		}

		/*	draw voltage cursors (if any)	*/
		if (Cursor_v1.isEnabled)
		{
			DRAW_VOLTAGE_CURSOR(
				imgBufferIndex, Cursor_v1.pos, LCD_CURSOR1_DRAWING_COLOR_U16);
		}

		if (Cursor_v2.isEnabled)
		{
			DRAW_VOLTAGE_CURSOR(
				imgBufferIndex, Cursor_v2.pos, LCD_CURSOR2_DRAWING_COLOR_U16);
		}

		/*	Send buffer to TFT using DMA (Internally waits for DMA TC)	*/
		TFT2_voidSendPixels(
			(TFT2_t*)&Global_LCD, (u16*)Global_ImgBufferArr[imgBufferIndex],
			LINES_PER_IMAGE_BUFFER * 128ul);

		/*	Update imgBufferIndex	*/
		if (imgBufferIndex == 0)
			imgBufferIndex = 1;
		else // if (imgBufferIndex == 1)
			imgBufferIndex = 0;

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

void OSC_voidMainSuperLoop(void)
{
	/*	last time info was drawn. (timestamp)	*/
	u64 lastInfoDrawTime = 0;

	/*	periodic time to draw info (in STK ticks)	*/
	volatile u64 infoDrawPeriod =
		INFO_DRAWING_PERIODIC_TIME_MS * (u64)stkTicksPerSecond / 1000ul;

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
	/*if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);*/

	char freqUnitPrefix;
	u32 freqInteger, freqFraction;

	OSC_voidGetNumberPritableVersion(
		freqmHz * 1000000, &freqInteger, &freqFraction, &freqUnitPrefix);

	/**	peak to peak voltage	**/
	u64 vpp =
		Global_Ch1PeakToPeakValueInCurrentFrame *
		Global_CurrentCh1MicroVoltsPerPix;

	char vppUnitPrefix;
	u32 vppInteger, vppFraction;

	OSC_voidGetNumberPritableVersion(
		vpp * 1000, &vppInteger, &vppFraction, &vppUnitPrefix);

	/**	volts per div	**/
	u64 voltsPerDiv =
		Global_CurrentCh1MicroVoltsPerPix * PIXELS_PER_VOLTAGE_DIV;

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
	u64 v1 = Cursor_v1.pos * Global_CurrentCh1MicroVoltsPerPix;

	char v1UnitPrefix;
	u32 v1Integer, v1Fraction;

	OSC_voidGetNumberPritableVersion(
		v1 * 1000, &v1Integer,
		&v1Fraction, &v1UnitPrefix);

	/**	v2	**/
	u64 v2 = Cursor_v2.pos * Global_CurrentCh2MicroVoltsPerPix;

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
	volatile u64 t2 = Cursor_t2.pos * Global_CurrentNanoSecondsPerPix;

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
	/*if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);*/

	/*	draw only if value has changed	*/
	if (freqmHz == lastFreqmHz)
		return;

	/**	print value on char array	**/
	char freqUnitPrefix;
	u32 freqInteger, freqFraction;

	OSC_voidGetNumberPritableVersion(
		freqmHz * 1000000, &freqInteger, &freqFraction, &freqUnitPrefix);

	sprintf(
		(char*)Global_Str, "F=%u.%u%cHz", freqInteger, freqFraction,
		freqUnitPrefix);

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

void OSC_voidDrawInfo()
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

	/*
	 * normally calibrate on ch1 input, unless disabled, then sync on ch2
	 * input
	 */
	u8 syncTimUnit;
	if (Global_IsCh1Enabled)
		syncTimUnit = FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER;
	else
		syncTimUnit = FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER;

	while(!TIM_GET_STATUS_FLAG(syncTimUnit, TIM_Status_CC1))
	{
		if (
			STK_u64GetElapsedTicks() - startTime >
			FREQ_MEASURE_TIMEOUT_MS * 72000
		)
			break;
	}

	if (TIM_GET_STATUS_FLAG(syncTimUnit, TIM_Status_CC1))
		freqmHz =
			TIM_u64GetFrequencyMeasured(syncTimUnit);

	/*
	 * set time per pix such that user can see 3 periods of the signal in one
	 * frame.
	 * eqn: time_per_pix = 3 * T / N_SAMPLES,
	 * where T is the periodic time of the signal.
	 */
	if (freqmHz == 0)
		Global_CurrentNanoSecondsPerPix = 1000;
	else
		Global_CurrentNanoSecondsPerPix =
			3e12 / freqmHz / NUMBER_OF_SAMPLES;

	/**	Gain and voltage calibration	**/

	Global_CurrentCh1MicroVoltsPerPix =
		(10000 * 1000ul) / 128;

	Global_CurrentCh2MicroVoltsPerPix =
		(10000 * 1000ul) / 128;

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}
