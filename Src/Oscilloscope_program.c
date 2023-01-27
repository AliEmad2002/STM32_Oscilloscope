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
#include "Oscilloscope_interface.h"

/*******************************************************************************
 * Extern global variables (from private.c file):
 ******************************************************************************/
extern volatile TFT2_t Global_LCD;
extern volatile b8 Global_LCDIsUnderUsage;
extern volatile u16 Global_QuarterOfTheDisplay[40][128];
extern volatile b8 Global_IsPixArrReady;
extern volatile NVIC_Interrupt_t Global_LCDDmaInterruptNumber;
extern volatile NVIC_Interrupt_t Global_RefreshQuarterOfTheDisplayTimerInterruptNumber;
extern volatile u8 Global_PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_LargestVlaueInCurrentFrame;
extern volatile u8 Global_SmallestVlaueInCurrentFrame;
extern volatile u8 Global_NumberOfsentQuartersSinceLastInfoUpdate;
extern volatile b8 Global_Enter;
extern volatile u32 Global_CurrentMicroVoltsPerPix;
extern volatile u32 Global_CurrentMicroSecondsPerPix;
extern volatile u8 Global_CurrentUsedAdcChannelIndex;
extern volatile OSC_RunningState_t Global_RunningState;
extern volatile b8 Global_Paused;

/*******************************************************************************
 * Mode switching:
 ******************************************************************************/
void OSC_voidEnterMathMode(void)
{

}

void OSC_voidTrigPauseResume(void)
{
	if (!Global_Paused)
	{
		Global_Paused = true;
	}

	else
	{
		Global_Paused = false;
	}

	Delay_voidBlockingDelayMs(150);
}

/*******************************************************************************
 * Main thread functions:
 ******************************************************************************/
void OSC_voidMainSuperLoop(void)
{
	/*
	 * ADC converted samples are stored here first.
	 * Note: 120 sample == full image frame.
	 */
	volatile u16 adcReadArr[120];

	/*
	 * These are the image buffers.
	 * notice that: (120 / n) must be an integer
	 */
	const u8 n = 20;
	u16 pixArr1[n][128];
	u16 pixArr2[n][128];

	/*	set screen boundaries for full signal image area	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 119);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	voltage in pixels	*/
	u8 lastRead = 0;
	u8 currentRead = 0;

	u8 currentSmaller;
	u8 currentLarger;

	/*	store STK ticks per second here
	 * (to not use the function more than once)
	 */
	volatile u32 stkTicksPerSecond = STK_u32GetTicksPerSecond();

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
		 * calculate STK ticks to wait between each sample and the one next to
		 * it. This has to be done each loop, as user may change
		 * "Global_CurrentMicroSecondsPerPix" in an ISR.
		 * Eqn.:
		 * N_ticks_sample =
		 * 	(microSecondsPerPix * STK_TicksPerSecond) / 1000000
		 */
		volatile u64 ticksToWait =
			((u64)Global_CurrentMicroSecondsPerPix * stkTicksPerSecond) /
			1000000ul;

		/*	take 120 samples (one frame, i.e.: 3/4 of the display) with interval
		 * between each two of them equal to:
		 * "Global_CurrentMicroSecondsPerPix".
		 */
		if (!Global_Paused)
		{
			for (u8 i = 0; i < 120; i++)
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
			/**	pixArr1	**/
			/*	Draw in "pixArr1[]".	*/
			for (u8 i = 0; i < n; i++)
			{
				/*
				 * read ADC converted value
				 * Eqn.: voltage_in_pix = voltage_in_volt * N_pixels_per_volt
				 *  = 10^5 * 33 * Vadc / microVoltsPerPix / 4096
				 */
				currentRead =
					((3300000ul * (u64)adcReadArr[readCount++]) /
					(u64)Global_CurrentMicroVoltsPerPix) >> 12;

				/*
				 * find the smaller and the larger of 'currentRead' and 'lastRead'
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

				/*	wait for DMA 1st, 2nd, 3rd channels transfer complete	*/
				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw background color from zero to just before smaller	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
					&pixArr1[i][0]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
					currentSmaller);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw main color from smaller to larger	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
					&pixArr1[i][currentSmaller]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
					currentLarger - currentSmaller + 1);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw background color from after larger to 127	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
					&pixArr1[i][currentLarger + 1]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
					127 - currentLarger);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);
			}

			/*	Send them using DMA (Internally waits for DMA TC)	*/
			TFT2_voidSendPixels(&Global_LCD, (u16*)pixArr1, n * 128);

			if (readCount == 120)
				break;

			/**	pixArr2	**/
			for (u8 i = 0; i < n; i++)
			{
				/*
				 * read ADC converted value
				 * Eqn.: voltage_in_pix = voltage_in_volt * N_pixels_per_volt
				 *  = 10^5 * 33 * Vadc / microVoltsPerPix / 4096
				 */
				currentRead =
					((3300000ul * (u64)adcReadArr[readCount++]) /
					(u64)Global_CurrentMicroVoltsPerPix) >> 12;

				/*
				 * find the smaller and the larger of 'currentRead' and 'lastRead'
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

				/*	wait for DMA 1st, 2nd, 3rd channels transfer complete	*/
				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

				DMA_voidWaitTillChannelIsFreeAndDisableIt(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw background color from zero to just before smaller	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
					&pixArr2[i][0]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL,
					currentSmaller);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, FIRST_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw main color from smaller to larger	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
					&pixArr2[i][currentSmaller]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL,
					currentLarger - currentSmaller + 1);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, SECOND_LINE_SEGMENT_DMA_CHANNEL);

				/*	draw background color from after larger to 127	*/
				DMA_voidSetMemoryAddress(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
					&pixArr2[i][currentLarger + 1]);

				DMA_voidSetNumberOfData(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL,
					127 - currentLarger);

				DMA_voidEnableChannel(
					DMA_UnitNumber_1, THIRD_LINE_SEGMENT_DMA_CHANNEL);
			}

			/*	Send them using DMA (Internally waits for DMA TC)	*/
			TFT2_voidSendPixels(&Global_LCD, (u16*)pixArr2, n * 128);

			if (readCount == 120)
				break;
		}

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
	u64 freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

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

void OSC_voidDrawInfoOnPixArray(void)
{
	/*	the string that info is stored at before drawing	*/
	char str[128];

	OSC_voidGetInfoStr(str);

	Txt_voidCpyStrToStaticPixArrNormalOrientation(
			str, colorRed.code565, colorBlack.code565, 1,
			Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
			0, 0, 128, 40, Global_QuarterOfTheDisplay);

	/*	raise ready flag	*/
	Global_IsPixArrReady = true;
}

void OSC_voidAutoCalibrate(void)
{
	/** Time calibration	**/
	/*	get signal frequency	*/
	u64 freqmHz = TIM_u64GetFrequencyMeasured(FREQ_MEASURE_TIMER_UNIT_NUMBER);

	/*
	 * set time per pix such that user can see 3 periods of the signal in one
	 * frame.
	 * eqn: time_per_pix = 3 * T / 120
	 * where T is the periodic time of the signal,
	 * 120 is the width of the image.
	 */
	if (freqmHz == 0)
		Global_CurrentMicroSecondsPerPix = 1;
	else
		Global_CurrentMicroSecondsPerPix =
			1000000000ul / freqmHz / 40;

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

	Delay_voidBlockingDelayMs(150);
}

/*******************************************************************************
 * ISR callbacks:
 ******************************************************************************/
