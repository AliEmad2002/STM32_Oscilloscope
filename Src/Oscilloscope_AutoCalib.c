/*
 * Oscilloscope_AutoCalib.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "My_Math.h"

/*	MCAL	*/
#include "RCC_interface.h"
#include "STK_interface.h"
#include "ADC_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "DMA_interface.h"
#include "SPI_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Conversions.h"
#include "Oscilloscope_AutoCalib.h"

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
