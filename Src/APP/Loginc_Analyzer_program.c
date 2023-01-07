/*
 * Loginc_Analyzer_program.c
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

/*	MCAL	*/
#include "DMA_interface.h"
#include "NVIC_interface.h"
#include "RCC_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "STK_interface.h"
#include "ADC_private.h"
#include "ADC_interface.h"

/*	HAL	*/
#include "TFT_interface_V1.h"
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Loginc_Analyzer_config.h"
#include "Loginc_Analyzer_interface.h"


/*	static objects	*/
static TFT2_t LCD;

/*	Defines based on configuration file	*/
#define ADC_1_CHANNEL		(ANALOG_INPUT_1_PIN % 16)
#define ADC_2_CHANNEL		(ANALOG_INPUT_2_PIN % 16)

/*
 * Inits all (MCAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file.
 */
void OSC_voidInitMCAL(void)
{
	/**************************************************************************
	 * RCC init:
	 *************************************************************************/
	RCC_voidSysClockInit();

	/*	GPIO/AFIO	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPA);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPB);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_AFIO);

	/*	ADC	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC1);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC2);
	RCC_voidSetAdcPrescaler(RCC_ADC_Prescaler_PCLK2_by6);

	/**************************************************************************
	 * SysTick init: (used for time-stamping)
	 *************************************************************************/
	STK_voidInit();
	STK_voidStartTickMeasure(STK_TickMeasureType_OverflowCount);
	STK_voidEnableSysTick();

	/**************************************************************************
	 * GPIO init:
	 *************************************************************************/
	GPIO_voidSetPinGpoPushPull(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);
	GPIO_voidSetPinMode(
		ANALOG_INPUT_1_PIN / 16, ANALOG_INPUT_1_PIN % 16,
		GPIO_Mode_Input_Analog);
	GPIO_voidSetPinMode(
		ANALOG_INPUT_2_PIN / 16, ANALOG_INPUT_2_PIN % 16,
		GPIO_Mode_Input_Analog);
	GPIO_voidSetPinInputPullDown(BUTTON_1_PIN / 16, BUTTON_1_PIN % 16);

	/**************************************************************************
	 * ADC init:
	 *************************************************************************/
	// enable continuous mode:
	ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_1);
	// set channel sample time:
	ADC_voidSetSampleTime(
		ADC_UnitNumber_1, ADC_1_CHANNEL, ADC_SAMPLE_TIME);
	// write channel in regular sequence:
	ADC_voidSetSequenceRegular(
		ADC_UnitNumber_1, ADC_RegularSequenceNumber_1, ADC_1_CHANNEL);
	// set regular sequence len to 1, as only one channel is to be converted:
	ADC_voidSetSequenceLenRegular(ADC_UnitNumber_1, 1);
	// set regular channels trigger to SWSTART:
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_SWSTART);
	// enable conversion on external event:
	ADC_voidEnableExternalTriggerRegular(ADC_UnitNumber_1);
	// power on:
	ADC_voidEnablePower(ADC_UnitNumber_1);
	// calibrate:
	ADC_voidStartCalibration(ADC_UnitNumber_1);
	ADC_voidWaitCalibration(ADC_UnitNumber_1);
	// trigger start of conversion:
	ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);

	/**************************************************************************
	 * NVIC init:
	 *************************************************************************/

}

/*
 * Inits all (HAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file, and static objects defined in "Loginc_Analyzer_program.c".
 */
void OSC_voidInitHAL(void)
{
	/**************************************************************************
	 * LCD init:
	 *************************************************************************/
	TFT2_voidInit(
		&LCD, LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, LCD_RST_PIN, LCD_A0_PIN,
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	/*	set maximum brightness by default	*/
	TFT2_voidSetBrightness(&LCD, POW_TWO(16) - 1);

	TFT2_SET_X_BOUNDARIES(&LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&LCD, 0, 159);
	u8 foo = 127;

	TFT2_WRITE_CMD(&LCD, TFT_CMD_MEM_WRITE);

	TFT2_ENTER_DATA_MODE(&LCD);

	TFT2_voidFillDMA(&LCD, &foo, 128 * 160);

	//Delay_voidBlockingDelayMs(2000);
}

/*
 * runs main super loop (no OS version)
 */
void OSC_voidRunMainSuperLoop(void)
{
	register u8 tftScrollCounter = 0;
	register u8 lastRead = 0;
	register u8 adcRead;
	register u8 largest, smallest;

	TFT2_voidWaitCurrentDataTransfer(&LCD);
DMA_voidSelectPriority(DMA_UnitNumber_1, LCD.dmaCh, DMA_Priority_VeryHigh);
	TFT2_SET_X_BOUNDARIES(&LCD, 0, 127);
	u8 foo1 = 255;
	u8 foo2 = 0;
	while(1)
	{
		TFT2_voidWaitCurrentDataTransfer(&LCD);

		/*	scroll TFT display	*/
		TFT2_voidScroll(&LCD, tftScrollCounter);

		/*	read ADC	*/
		adcRead = 127 -
			(u8)(((u32)ADC_u16GetDataRegular(ADC_UnitNumber_1)) * 127u / 4095u);

		if (adcRead > lastRead)
		{
			largest = adcRead;
			smallest = lastRead;
		}
		else
		{
			largest = lastRead;
			smallest = adcRead;
		}

		TFT2_SET_Y_BOUNDARIES(&LCD, tftScrollCounter, tftScrollCounter);

		TFT2_WRITE_CMD(&LCD, TFT_CMD_MEM_WRITE);

		TFT2_ENTER_DATA_MODE(&LCD);
		volatile u64 tStart = STK_u64GetElapsedTicks();
		TFT2_voidFillDMA(&LCD, &foo2, smallest);

		TFT2_voidFillDMA(
			&LCD, &foo1, largest - smallest);

		TFT2_voidFillDMA(
			&LCD, &foo2, 128 - largest);
		volatile u64 tEnd = STK_u64GetElapsedTicks();
		/*	iteration control	*/
		tftScrollCounter++;
		if (tftScrollCounter == 161)
			tftScrollCounter = 0;
		lastRead = adcRead;
		Delay_voidBlockingDelayMs(50);


		trace_printf("%u ticks, ", (u32)(tEnd - tStart));
		trace_printf("%u us\n",
			(u32)(1000000 * (tEnd - tStart) / RCC_u32GetBusClk(RCC_Bus_AHB)));
	}
}








































