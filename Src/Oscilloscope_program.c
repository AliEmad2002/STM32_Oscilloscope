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
 * Extern global variables:
 ******************************************************************************/
extern volatile TFT2_t Global_LCD;
extern volatile u16* Global_ImgBufferArr[2];

extern volatile u16 Global_SampleBuffer[2 * NUMBER_OF_SAMPLES];

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

extern volatile b8 Global_IsCh1Enabled;
extern volatile b8 Global_IsCh2Enabled;

extern volatile u8 Global_LastRead1;
extern volatile u8 Global_LastRead2;

extern volatile u8 Global_Smaller1;
extern volatile u8 Global_Larger1;

extern volatile u8 Global_Smaller2;
extern volatile u8 Global_Larger2;

extern volatile b8 Global_Ch1LastReadWasInRange;
extern volatile b8 Global_Ch2LastReadWasInRange;

extern volatile s32 Global_Offset1MicroVolts;
extern volatile s32 Global_Offset2MicroVolts;

extern volatile OSC_RunningMode_t Global_CurrentRunningMode;

extern volatile u8 Global_NotInUseImgBufferIndex;

extern volatile OSC_Cursor_t Cursor_v1;
extern volatile OSC_Cursor_t Cursor_v2;
extern volatile OSC_Cursor_t Cursor_t1;
extern volatile OSC_Cursor_t Cursor_t2;

extern void OSC_voidDrawInfo(void);

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

/*******************************************************************************
 * Brightness control:
 ******************************************************************************/
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
 * Offset control:
 ******************************************************************************/
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

/*******************************************************************************
 * channel on/off control:
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
 * Button / rotary encoder callbacks:
 ******************************************************************************/
void OSC_voidTrigPauseResume(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
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
		ROTARY_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
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
		ROTARY_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
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
//		BUTTON_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
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
		BUTTON_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
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
 * Sampling functions and macros:
 ******************************************************************************/
void OSC_voidWaitForSignalRisingEdge(void)
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
		if (
			STK_u64GetElapsedTicks() - startTime >
			RISING_EDGE_WAIT_TIMEOUT_MS * STK_TICKS_PER_MS
		)
		{
			break;
		}
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

/*******************************************************************************
 * Drawing functions and macros:
 ******************************************************************************/
/*
* Eqn.:
* "v_ch" is the very main input that user can handle.
* "v_opamp" is op-amp's output voltage.
* "v_adc" is the converted 12-bit value by internal ADC.
*
* v_ch 	= 5 - v_opamp / 0.33			[volts]
* 		= 5 - 10 * v_adc / 4096			[volts]
*
* v_pix = v_ch * pixels_per_volt + SIGNAL_LINE_LENGTH / 2
*
* 		 = 	5 * pixels_per_volt -
* 		 	(10 * pixels_per_volt * v_adc) / 4096 +
* 		 	64							[pixels]
*/
#define GET_CH1_V_IN_PIXELS(v_adc)                                            \
(	                                                                   	      \
	(5 * 1000000 + Global_Offset1MicroVolts) /                                \
	Global_CurrentCh1MicroVoltsPerPix -						                  \
	(10000000 * (s64)(v_adc)) / Global_CurrentCh1MicroVoltsPerPix / 4096 + SIGNAL_LINE_LENGTH / 2 	                                                      \
)

#define GET_CH2_V_IN_PIXELS(v_adc)                                            \
(	                                                                   	      \
	(5 * 1000000 + Global_Offset2MicroVolts) /                                \
	Global_CurrentCh2MicroVoltsPerPix -						                  \
	(10000000 * (s64)(v_adc)) / Global_CurrentCh2MicroVoltsPerPix / 4096 + SIGNAL_LINE_LENGTH / 2 	                                                      \
)

#define GET_V_IN_MICRO_VOLTS(v_adc)                 \
(	                                                \
	5 * 1000000 - (10000000 * (s64)(v_adc)) / 4096	\
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

#define DRAW_VOLTAGE_CURSOR(pos, color)			        \
{                                                                       \
	for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i += SUM_OF_DASH_LEN)    \
	{                                                                   \
		for (u8 k = 0; k < 3; k++)                                      \
		{                                                               \
			u16 index = (i + k) * SIGNAL_LINE_LENGTH + (pos);                          \
                                                                        \
			Global_ImgBufferArr[Global_NotInUseImgBufferIndex][index] =              \
				(color);                      							\
		}                                                               \
	}                                                                   \
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
	currentRead1Pix >= SIGNAL_IMG_X_MIN &&  \
	currentRead1Pix < SIGNAL_LINE_LENGTH    \
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

/*******************************************************************************
 * Auto calibration:
 ******************************************************************************/
typedef enum{
	OSC_Channel_1,
	OSC_Channel_2
}OSC_Channel_t;

void OSC_voidFindMaxAndMinOfChxInTwoSeconds(
	OSC_Channel_t ch, u16* maxPtr, u16* minPtr)
{
	/*	set external trigger to be SWSTART	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_SWSTART);

	/*	start new conversion	*/
	ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);

	/*	set max and min to the last converted value	*/
	*maxPtr = (u16)(ADC_u32GetDataRegularDual() >> (16 * ch));
	*minPtr = *maxPtr;

	/*	start time counting	*/
	u64 startTime = STK_u64GetElapsedTicks();

	/*	while 2 seconds have not yet passed:	*/
	while(STK_u64GetElapsedTicks() - startTime < 2000ul * 72000ul)
	{
		/*	wait for conversion end	*/
		while(!ADC_b8GetStatusFlag(ADC_UnitNumber_1, ADC_StatusFlag_EOC));

		/*	take the newly converted value	*/
		u16 newReading = (u16)(ADC_u32GetDataRegularDual() >> (16 * ch));;

		/*	start next conversion	*/
		ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);

		/*	if larger than max, make it max	*/
		WRITE_IF_LARGER(*maxPtr, newReading);

		/*	if smaller than min, make it min	*/
		WRITE_IF_SMALLER(*minPtr, newReading);
	}

	/*	set external trigger back to be TIM3TRGO	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_TIM3TRGO);
}

void OSC_voidFindMaxAndMinOfBothChannelsInTwoSeconds(
	u16* max1Ptr, u16* min1Ptr, u16* max2Ptr, u16* min2Ptr)
{
	/*	set external trigger to be SWSTART	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_SWSTART);

	/*	enable continuous mode	*/
	ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_1);
	ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_2);

	/*	start conversion	*/
	ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);

	/*	set max and min to the last converted value	*/
	u32 bothChannelsConverted = ADC_u32GetDataRegularDual();
	*max1Ptr = (u16)bothChannelsConverted;
	*min1Ptr = *max1Ptr;

	*max2Ptr = (u16)(bothChannelsConverted >> 16);
	*min2Ptr = *max2Ptr;

	/*	start time counting	*/
	u64 startTime = STK_u64GetElapsedTicks();

	/*	while 2 seconds have not yet passed:	*/
	while(STK_u64GetElapsedTicks() - startTime < 2000ul * 72000ul)
	{
		/*	take the newly converted value	*/
		bothChannelsConverted = ADC_u32GetDataRegularDual();
		u16 ch1Converted = (u16)bothChannelsConverted;
		u16 ch2Converted = (u16)(bothChannelsConverted >> 16);

		/*	if larger than max, make it max	*/
		WRITE_IF_LARGER(*max1Ptr, ch1Converted);
		WRITE_IF_LARGER(*max2Ptr, ch2Converted);

		/*	if smaller than min, make it min	*/
		WRITE_IF_SMALLER(*min1Ptr, ch1Converted);
		WRITE_IF_SMALLER(*min2Ptr, ch2Converted);
	}

	/*	disable continuous mode	*/
	ADC_voidEnableSingleConversionMode(ADC_UnitNumber_1);
	ADC_voidEnableSingleConversionMode(ADC_UnitNumber_2);

	/*	set external trigger back to be TIM3TRGO	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_TIM3TRGO);
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

	/**	Voltage Div. calibration	**/
	/*
	 * run enabled oscilloscope channels @ maximum speed for 2 seconds,
	 * to find the maximum and the minimum readings of them during this time
	 * period.
	 */
	u16 max1, min1, max2, min2;

	OSC_voidFindMaxAndMinOfBothChannelsInTwoSeconds(
		&max1, &min1, &max2, &min2);

	/*	convert these values to micro volts	*/
	s64 max1Uv = GET_V_IN_MICRO_VOLTS(min1);
	s64 min1Uv = GET_V_IN_MICRO_VOLTS(max1);
	s64 max2Uv = GET_V_IN_MICRO_VOLTS(min2);
	s64 min2Uv = GET_V_IN_MICRO_VOLTS(max2);

	/*
	 * volts per pixel = (v_max - v_min) / (SIGNAL_LINE_LENGTH * 70%)
	 * (The 90% means that the signal would take only 70% of the display, which
	 * is nicer to user that fully taking the screen)
	 *
	 * (Min is 100mV per whole display, can be later changed)
	 */
	Global_CurrentCh1MicroVoltsPerPix = (max1Uv - min1Uv) / ((70 * SIGNAL_LINE_LENGTH) / 100);
	if (Global_CurrentCh1MicroVoltsPerPix <= 100000 / SIGNAL_LINE_LENGTH)
		Global_CurrentCh1MicroVoltsPerPix = 100000 / SIGNAL_LINE_LENGTH;

	Global_CurrentCh2MicroVoltsPerPix = (max2Uv - min2Uv) / ((70 * SIGNAL_LINE_LENGTH) / 100);
	if (Global_CurrentCh2MicroVoltsPerPix <= 100000 / SIGNAL_LINE_LENGTH)
		Global_CurrentCh2MicroVoltsPerPix = 100000 / SIGNAL_LINE_LENGTH;

	/*	channel offset in micro volts = - avg{v_max, v_min}	*/
	Global_Offset1MicroVolts = - (max1Uv + min1Uv) / 2;
	Global_Offset2MicroVolts = - (max2Uv + min2Uv) / 2;

	/*	update global min, max and vpp	*/
	Global_Ch1MaxValueInCurrentFrame = GET_CH1_V_IN_PIXELS(min1);
	Global_Ch1MinValueInCurrentFrame = GET_CH1_V_IN_PIXELS(max1);
	Global_Ch1PeakToPeakValueInCurrentFrame =
		Global_Ch1MaxValueInCurrentFrame - Global_Ch1MinValueInCurrentFrame;

	Global_Ch2MaxValueInCurrentFrame = GET_CH2_V_IN_PIXELS(min2);
	Global_Ch2MinValueInCurrentFrame = GET_CH2_V_IN_PIXELS(max2);
	Global_Ch2PeakToPeakValueInCurrentFrame =
		Global_Ch2MaxValueInCurrentFrame - Global_Ch2MinValueInCurrentFrame;

	/** Time calibration	**/
	u8 syncTimUnit;

	/*
	 * if both channels were enabled, calibrate time on that of larger peak to
	 * peak voltage.
	 */
	if (Global_IsCh1Enabled && Global_IsCh2Enabled)
	{
		u32 vpp1 =
			Global_Ch1PeakToPeakValueInCurrentFrame *
			Global_CurrentCh1MicroVoltsPerPix;

		u32 vpp2 =
			Global_Ch2PeakToPeakValueInCurrentFrame *
			Global_CurrentCh2MicroVoltsPerPix;

		if (vpp1 > vpp2)
			syncTimUnit = FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER;
		else
			syncTimUnit = FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER;
	}

	/*	else if only ch1 was enabled, calibrate time on it	*/
	else if (Global_IsCh1Enabled)
		syncTimUnit = FREQ_MEASURE_CH1_TIMER_UNIT_NUMBER;

	/*	else if only ch2 was enabled, calibrate time on it	*/
	else
		syncTimUnit = FREQ_MEASURE_CH2_TIMER_UNIT_NUMBER;

	/*
	 * Get signal frequency.
	 * Wait for CC1IF to raise (using timeout: "FREQ_MEASURE_TIMEOUT_MS"),
	 * as it may be cleared by a previous frequency read, which would lead to
	 * a fault measurement of a zero frequency.
	 */
	volatile u64 freqmHz = 0;

u64 startTime = STK_u64GetElapsedTicks();

	while(!TIM_GET_STATUS_FLAG(syncTimUnit, TIM_Status_CC1))
	{
		if (
			STK_u64GetElapsedTicks() - startTime >
			FREQ_MEASURE_TIMEOUT_MS * 72000
		)
			break;
	}

	/*
	 * if CC1IF was not raised before reading CCR1, then no transition have
	 * not happened. i.e.: freq = 0 (no change)
	 */
	if (TIM_GET_STATUS_FLAG(syncTimUnit, TIM_Status_CC1))
		freqmHz =
			TIM_u64GetFrequencyMeasured(syncTimUnit);

	/*
	 * set time per pix such that user can see 3 periods of the signal in one
	 * frame. (only if frequency is larger than zero)
	 * eqn: time_per_pix = 3 * T / N_SAMPLES,
	 * where T is the periodic time of the signal.
	 */
	if (freqmHz == 0)
		Global_CurrentNanoSecondsPerPix = 1000;
	else
		Global_CurrentNanoSecondsPerPix =
			3e12 / freqmHz / NUMBER_OF_SAMPLES;

	/**	debouncing timestamp	**/
	lastPressTime = STK_u64GetElapsedTicks();
}
