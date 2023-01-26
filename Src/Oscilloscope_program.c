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
extern TFT2_t Global_LCD;
extern u16 Global_QuarterOfTheDisplay[40][128];
extern b8 Global_LCDIsUnderUsage;
extern NVIC_Interrupt_t Global_LCDDmaInterruptNumber;
extern NVIC_Interrupt_t Global_RefreshQuarterOfTheDisplayTimerInterruptNumber;
extern u8 Global_PeakToPeakValueInCurrentFrame;
extern u8 Global_LargestVlaueInCurrentFrame;
extern u8 Global_SmallestVlaueInCurrentFrame;
extern u8 Global_NumberOfsentQuartersSinceLastInfoUpdate;
extern u8 Global_NumberOfsentQuartersRequieredForInfoUpdate;
extern b8 Global_Enter;
extern u32 Global_CurrentMicroVoltsPerPix;
extern u32 Global_CurrentMicroSecondsPerPix;
extern u8 Global_CurrentUsedAdcChannelIndex;
extern OSC_RunningState_t Global_RunningState;
extern b8 Global_Paused;

/*******************************************************************************
 * Mode switching:
 ******************************************************************************/
void OSC_voidEnterNormalMode(void)
{
	OSC_voidResume();
}

void OSC_voidEnterMathMode(void)
{

}

void OSC_voidPause(void)
{
	/*	stop display refresh trigger counter	*/
	TIM_voidDisableCounter(LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

	/*	update flag	*/
	Global_Paused = true;
}

void OSC_voidResume(void)
{
	/*	start display refresh trigger counter	*/
	TIM_voidEnableCounter(LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

	/*	update flag	*/
	Global_Paused = false;
}

void OSC_voidTrigPauseResume(void)
{
	if (!Global_Paused)
	{
		OSC_voidPause();
	}

	else
	{
		OSC_voidResume();
	}
}

/*******************************************************************************
 * Main thread functions:
 ******************************************************************************/
void OSC_voidMainSuperLoop(void)
{
	while (1)
	{
		if (Global_Paused)
			continue;

		else if (runningState == OSC_RunningState_NormalMode)
		{
			if (isInfoPixArrPrepared == false)
				OSC_voidPrepareInfoPixArray();
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
	for (u8 i = 0; i < CHANNEL_1_NUMBER_OF_LEVELS; i++)
	{
		ADC_ChannelNumber_t adcCh = oscCh1AdcChannels[i].adcChannelNumber;

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
		/*	otherwise, use this gain, and break out of the loop	*/
		else
		{
			Global_CurrentMicroVoltsPerPix =
				(oscCh1AdcChannels[i].maxVPPinMilliVolts * 1000ul) / 128;

			Global_CurrentUsedAdcChannelIndex = i;

			break;
		}
	}
}

/*******************************************************************************
 * ISR callbacks:
 ******************************************************************************/





































