/*
 * Oscilloscope_Sampling.c
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
#include "Oscilloscope_Sampling.h"

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
