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

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_init_Global.h"
#include "Oscilloscope_init_MCAL.h"
#include "Oscilloscope_init_HAL.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_interface.h"

/*******************************************************************************
 * Extern global variables (from private.c file):
 ******************************************************************************/
extern volatile u32 stkTicksPerSecond;

extern volatile TFT2_t Global_LCD;
extern volatile u8 Global_PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_LargestVlaueInCurrentFrame;
extern volatile u8 Global_SmallestVlaueInCurrentFrame;
extern volatile u32 Global_CurrentMicroVoltsPerPix;
extern volatile u64 Global_CurrentNanoSecondsPerPix;
extern volatile u8 Global_CurrentUsedAdcChannelIndex;
extern volatile b8 Global_Paused;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_ReturnedFromMenu;

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
	if (Global_CurrentMicroVoltsPerPix < 1000 / PIXELS_PER_VOLTAGE_DIV) // less than 1mV/Div
		Global_CurrentMicroVoltsPerPix += 50 / PIXELS_PER_VOLTAGE_DIV; // increment by 50uV/Div

	else if (Global_CurrentMicroVoltsPerPix < 20000 / PIXELS_PER_VOLTAGE_DIV) // less than 20mV/Div
		Global_CurrentMicroVoltsPerPix += 5000 / PIXELS_PER_VOLTAGE_DIV; // increment by 5mV/Div

	else if (Global_CurrentMicroVoltsPerPix < 100000 / PIXELS_PER_VOLTAGE_DIV) // less than 100mV/Div
		Global_CurrentMicroVoltsPerPix += 20000 / PIXELS_PER_VOLTAGE_DIV; // increment by 20mV/Div

	else if (Global_CurrentMicroVoltsPerPix < 1000000 / PIXELS_PER_VOLTAGE_DIV) // less than 1V/Div
		Global_CurrentMicroVoltsPerPix += 100000 / PIXELS_PER_VOLTAGE_DIV; // increment by 100mV/Div

	else if (Global_CurrentMicroVoltsPerPix < 10000000 / PIXELS_PER_VOLTAGE_DIV) // less than 10V/Div
		Global_CurrentMicroVoltsPerPix += 1000000 / PIXELS_PER_VOLTAGE_DIV; // increment by 1V/Div
}

void OSC_voidIncrementTimeDiv(void)
{

}

void OSC_voidDecrementVoltageDiv(void)
{
	if (Global_CurrentMicroVoltsPerPix > 1000000 / PIXELS_PER_VOLTAGE_DIV) // larger than 1V
		Global_CurrentMicroVoltsPerPix -= 1000000 / PIXELS_PER_VOLTAGE_DIV; // decrement by 1V

	else if (Global_CurrentMicroVoltsPerPix > 100000 / PIXELS_PER_VOLTAGE_DIV) // larger than 100mV
		Global_CurrentMicroVoltsPerPix -= 20000 / PIXELS_PER_VOLTAGE_DIV; // decrement by 20mV

	else if (Global_CurrentMicroVoltsPerPix > 20000 / PIXELS_PER_VOLTAGE_DIV) // larger than 20mV
		Global_CurrentMicroVoltsPerPix -= 5000 / PIXELS_PER_VOLTAGE_DIV; // decrement by 5mV

	else if (Global_CurrentMicroVoltsPerPix > 1000 / PIXELS_PER_VOLTAGE_DIV) // larger than 1mV
		Global_CurrentMicroVoltsPerPix -= 50 / PIXELS_PER_VOLTAGE_DIV; // decrement by 50uV
}

void OSC_voidDecrementTimeDiv(void)
{

}

void OSC_voidIncrementBrightness(void)
{
	u32 brightness = TFT2_u16GetBrightness(&Global_LCD);

	brightness += 1000;

	if (brightness > POW_TWO(16) - 1)
		brightness = POW_TWO(16) - 1;

	TFT2_voidSetBrightness(&Global_LCD, brightness);
}

void OSC_voidDecrementBrightness(void)
{
	u16 brightness = TFT2_u16GetBrightness(&Global_LCD);

	if (brightness > 3000)
		brightness -= 1000;

	TFT2_voidSetBrightness(&Global_LCD, brightness);
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
		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
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
		BUTTON_DEBOUNCING_TIME_MS * 72000ul)
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
/*******************************************************************************
 * Main thread functions:
 ******************************************************************************/
void OSC_voidWaitForSignalRisingEdge(void)
{
	/*
	 * if frequency measured was more than 10Hz, then wait for
	 * a rising edge of the signal before starting sampling. This gives
	 * a much nicer display because of syncing signal and display.
	 *
	 * Timeout: 0.1 second.
	 */
	u64 freqmHz = 0;
	/*
	 * if CC1IF was not raised before reading CCR1, then no transition have
	 * not happened. i.e.: freq = 0
	 */
	if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	if (freqmHz > 10000)
	{
		u64 startTime = STK_u64GetElapsedTicks();

		while(!TIM_b8GetStatusFlag(1, TIM_Status_CC1))
		{
			if (STK_u64GetElapsedTicks() - startTime > stkTicksPerSecond / 10)
				break;
		}

		TIM_voidClearStatusFlag(1, TIM_Status_CC1);
	}
}

void OSC_voidMainSuperLoop(void)
{
	/*
	 * ADC converted samples are stored here first.
	 */
	volatile u16 adcReadArr[NUMBER_OF_SAMPLES];

	/*
	 * This is the image buffer.
	 * This buffer is virtually divided into two buffers, one to be sent to TFT,
	 * and another one to be processed while the first is sent.
	 */
	u16 pixArr1[2 * LINES_PER_IMAGE_BUFFER * 128];
	u16* pixArr[] = {pixArr1, &pixArr1[LINES_PER_IMAGE_BUFFER * 128]};

	/*	voltage in pixels	*/
	u8 lastRead = 0;
	u8 currentRead = 0;

	u8 currentSmaller;
	u8 currentLarger;

	/*	last time info was drawn. (timestamp)	*/
	u64 lastInfoDrawTime = STK_u64GetElapsedTicks();

	/*	periodic time to draw info (in STK ticks)	*/
	volatile u64 infoDrawPeriod =
		INFO_DRAWING_PERIODIC_TIME_MS * (u64)stkTicksPerSecond / 1000ul;

	/*	set source address of each of the line segment drawing DMA channels	*/
	DMA_voidSetPeripheralAddress(
		DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
		(void*)&LCD_BACKGROUND_COLOR_U16);

	DMA_voidSetPeripheralAddress(
		DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
		(void*)&LCD_MAIN_DRAWING_COLOR_U16);

	DMA_voidSetPeripheralAddress(
		DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
		(void*)&LCD_BACKGROUND_COLOR_U16);

	while (1)
	{
		/*
		 * on menu return, set boundaries and initiate TFT data sending again
		 * to avoid mis-sync
		 */
		if (Global_ReturnedFromMenu)
		{
			/*	set screen boundaries for full signal image area	*/
			TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
			TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, NUMBER_OF_SAMPLES - 1);

			/*	start data writing mode on screen	*/
			TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
			TFT2_ENTER_DATA_MODE(&Global_LCD);

			/*	clear "Global_ReturnedFromMenu" flag	*/
			Global_ReturnedFromMenu = false;
		}
		/*
		 * calculate STK ticks to wait between each sample and the one next to
		 * it. This has to be done each loop, as user may change
		 * "Global_CurrentMicroSecondsPerPix" in an ISR.
		 * Eqn.:
		 * N_ticks_sample =
		 * 	(microSecondsPerPix * STK_TicksPerSecond) / 1000000
		 */
		volatile u64 ticksToWait =
			((u64)Global_CurrentNanoSecondsPerPix * stkTicksPerSecond) /
			1000000000ul;

		/*	take "NUMBER_OF_SAMPLES" samples with interval between each two of
		 * them equal to: "Global_CurrentMicroSecondsPerPix".
		 */
		if (!Global_Paused)
		{
			/*	if info drawing time has passed, draw it	*/
			if (STK_u64GetElapsedTicks() - lastInfoDrawTime > infoDrawPeriod)
			{
				/*	wait for transfer complete	*/
				TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

				OSC_voidDrawInfoOnPixArray(pixArr1);

				/*	set screen boundaries for info image area	*/
				TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
				TFT2_SET_Y_BOUNDARIES(&Global_LCD, NUMBER_OF_SAMPLES, 159);

				/*	start data writing mode on screen	*/
				TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
				TFT2_ENTER_DATA_MODE(&Global_LCD);

				/*	send info image	*/
				TFT2_voidSendPixels(
					&Global_LCD, (u16*)pixArr1, LINES_PER_IMAGE_BUFFER * 128ul);

				/*	wait for transfer complete	*/
				TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

				/*	set screen boundaries for full signal image area	*/
				TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
				TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, NUMBER_OF_SAMPLES - 1);

				/*	start data writing mode on screen	*/
				TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
				TFT2_ENTER_DATA_MODE(&Global_LCD);

				lastInfoDrawTime = STK_u64GetElapsedTicks();
			}

			/*	sync on signal rising edge	*/
			OSC_voidWaitForSignalRisingEdge();

			for (u16 i = 0; i < NUMBER_OF_SAMPLES; i++)
			{
				/*	timestamp start time	*/
				volatile u64 startTime = STK_u64GetElapsedTicks();

				/*	store ADC read	*/
				adcReadArr[i] = ADC_u16GetDataRegularUnit1();

				/*	wait for sampling periodic time to pass	*/
				while(STK_u64GetElapsedTicks() - startTime < ticksToWait);
			}
		}

		/*	counter of the processed samples of current image frame	*/
		u8 readCount = 0;

		/*	draw current image frame of screen	*/
		while(1)
		{
			for (u8 j = 0; j < 2; j++)
			{
				/*	Draw/process half of "pixArr[]".	*/
				for (u8 i = 0; i < LINES_PER_IMAGE_BUFFER; i++)
				{
					/*
					 * read ADC converted value
					 * Eqn.: voltage_in_pix = voltage_in_volt * N_pixels_per_volt
					 *  = 10^5 * 33 * Vadc / microVoltsPerPix / 4096
					 */
					u64 currentReadU64 =
						((3300000ul * (u64)adcReadArr[readCount]) /
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
						&pixArr[j][i * 128]);

					DMA_voidSetNumberOfData(
						DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
						currentSmaller);

					DMA_voidEnableChannel(
						DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

					/*	draw main color from smaller to larger	*/
					DMA_voidSetMemoryAddress(
						DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
						&pixArr[j][i * 128 + currentSmaller]);

					DMA_voidSetNumberOfData(
						DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
						currentLarger - currentSmaller + 1);

					DMA_voidEnableChannel(
						DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

					/*	draw background color from after larger to 127	*/
					DMA_voidSetMemoryAddress(
						DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
						&pixArr[j][i * 128 + currentLarger + 1]);

					DMA_voidSetNumberOfData(
						DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
						127 - currentLarger);

					DMA_voidEnableChannel(
						DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);

					/*	draw time cursors (if any)	*/
					if (Cursor_t1.isEnabled && Cursor_t1.pos == readCount)
					{
						u8 lineNumber = Cursor_t1.pos % LINES_PER_IMAGE_BUFFER;
						for (
							u8 k = 0; k < 128;
							k +=
								DASHED_LINE_DRAWN_SEGMENT_LEN +
								DASHED_LINE_BLANK_SEGMENT_LEN
						)
						{
							for (u8 l = 0; l < 3 && k < 128; l++)
								pixArr[j][(lineNumber) * 128 + (k + l)] =
									LCD_CURSOR1_DRAWING_COLOR_U16;
						}
					}

					if (Cursor_t2.isEnabled && Cursor_t2.pos == readCount)
					{
						u8 lineNumber = Cursor_t2.pos % LINES_PER_IMAGE_BUFFER;
						for (
							u8 k = 0; k < 128;
							k +=
								DASHED_LINE_DRAWN_SEGMENT_LEN +
								DASHED_LINE_BLANK_SEGMENT_LEN
						)
						{
							for (u8 l = 0; l < 3 && k < 128; l++)
								pixArr[j][(lineNumber) * 128 + (k + l)] =
									LCD_CURSOR2_DRAWING_COLOR_U16;
						}
					}

					readCount++;
				}

				/*	draw voltage cursors (if any)	*/
				if (Cursor_v1.isEnabled)
				{
					for (
						u8 i = 0; i < LINES_PER_IMAGE_BUFFER;
						i +=
							DASHED_LINE_DRAWN_SEGMENT_LEN +
							DASHED_LINE_BLANK_SEGMENT_LEN
					)
					{
						for (u8 k = 0; k < 3; k++)
						{
							pixArr[j][(i + k) * 128 + Cursor_v1.pos] =
								LCD_CURSOR1_DRAWING_COLOR_U16;
						}
					}
				}

				if (Cursor_v2.isEnabled)
				{
					for (
						u8 i = 0; i < LINES_PER_IMAGE_BUFFER;
						i +=
							DASHED_LINE_DRAWN_SEGMENT_LEN +
							DASHED_LINE_BLANK_SEGMENT_LEN
					)
					{
						for (u8 k = 0; k < 3; k++)
						{
							pixArr[j][(i + k) * 128 + Cursor_v2.pos] =
								LCD_CURSOR2_DRAWING_COLOR_U16;
						}
					}
				}

				/*	Send them using DMA (Internally waits for DMA TC)	*/
				TFT2_voidSendPixels(
					&Global_LCD, (u16*)pixArr[j],
					LINES_PER_IMAGE_BUFFER * 128ul);

				if (readCount == NUMBER_OF_SAMPLES)
					goto endCurrentFrame;
			}
		}
		endCurrentFrame:;
	}
}

void OSC_voidGetInfoStr(char* str)
{
	char hzPre;
	char voltPre;
	u32 vPPInt, vPPFrac;

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

	/*	peak to peak voltage (in uV)	*/
	u64 vPP =
		Global_PeakToPeakValueInCurrentFrame * Global_CurrentMicroVoltsPerPix;

	if (freqmHz < 1000)
		hzPre = 'm';
	else if (freqmHz < 1000000)
	{
		freqmHz /= 1000;
		hzPre = ' ';
	}
	else if (freqmHz < 1000000000)
	{
		freqmHz /= 1000000;
		hzPre = 'k';
	}
	else if (freqmHz < 1000000000000)
	{
		freqmHz /= 1000000000;
		hzPre = 'M';
	}
	else
	{
		freqmHz = 0;
		hzPre = ' ';
	}

	if (vPP < 1000)
	{
		voltPre = 'u';
		vPPInt = vPP;
		vPPFrac = 0;
	}
	else if (vPP < 1000000)
	{
		voltPre = 'm';
		vPPInt = vPP / 1000;
		vPPFrac = vPP % 1000;
	}
	else
	{
		voltPre = ' ';
		vPPInt = vPP / 1000000;
		vPPFrac = vPP % 1000000;
	}

	/*	fraction is maximumly of 3 digits	*/
	while (vPPFrac > 100)
		vPPFrac /= 10;

	sprintf(
		(char*)str, "F = %u%cHz\nVpp = %u.%u%cV",
		(u32)freqmHz, hzPre, vPPInt, vPPFrac, voltPre);
}

void OSC_voidDrawInfoOnPixArray(u16* pixArr)
{
	/*	the string that info is stored at before drawing	*/
	char str[128];

	OSC_voidGetInfoStr(str);

	Txt_voidCpyStrToStaticPixArrNormalOrientation(
			(u8*)str, colorRed.code565, colorBlack.code565, 1,
			Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
			0, 0, LINES_PER_IMAGE_BUFFER, 128, pixArr);
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
	/*	get signal frequency	*/
	u64 freqmHz = 0;
	/*
	 * if CC1IF was not raised before reading CCR1, then no transition have
	 * not happened. i.e.: freq = 0
	 */
	if (TIM_GET_STATUS_FLAG(FREQ_MEASURE_TIMER_UNIT_NUMBER, TIM_Status_CC1))
		freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

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

	/*	set global parameters	*/
	Global_CurrentMicroVoltsPerPix =
		(oscCh1AdcChannels[i].maxVPPinMilliVolts * 1000ul) / 128;

	Global_CurrentUsedAdcChannelIndex = i;

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}
