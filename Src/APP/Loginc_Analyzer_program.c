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
#include "NVIC_interface.h"
#include "RCC_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "STK_interface.h"
#include "ADC_interface.h"

/*	HAL	*/
#include "TFT_interface.h"

/*	SELF	*/
#include "Loginc_Analyzer_config.h"
#include "Loginc_Analyzer_interface.h"


#include "SPI_private.h"

/*	static objects	*/
static TFT_t LCD;
static Frame_t frame;
static u16 ch1Val = 0;
static u16 ch2Val = 0;

/*	Defines based on configuration file	*/
#define ADC_1_CHANNEL		(ANALOG_INPUT_1_PIN % 16)
#define ADC_2_CHANNEL		(ANALOG_INPUT_2_PIN % 16)

/*	EOC callback	*/
void OSC_voidEOCCallback(void);

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

	/*	SPI	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_SPI1);

	/*	TIM	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB1, RCC_PERIPHERAL_TIM4);

	/*	ADC	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC1);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC2);
	RCC_voidSetAdcPrescaler(RCC_ADC_Prescaler_PCLK2_by6);

	/**************************************************************************
	 * GPIO init:
	 *************************************************************************/
	GPIO_voidSetPinGpoPushPull(LCD_A0_PIN / 16, LCD_A0_PIN % 16);
	GPIO_voidSetPinGpoPushPull(LCD_RST_PIN / 16, LCD_RST_PIN % 16);
	GPIO_voidSetPinGpoPushPull(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);
	GPIO_voidSetPinMode(
		ANALOG_INPUT_1_PIN / 16, ANALOG_INPUT_1_PIN % 16,
		GPIO_Mode_Input_Analog);
	GPIO_voidSetPinMode(
		ANALOG_INPUT_2_PIN / 16, ANALOG_INPUT_2_PIN % 16,
		GPIO_Mode_Input_Analog);
	GPIO_voidSetPinInputPullDown(BUTTON_1_PIN / 16, BUTTON_1_PIN % 16);

	/**************************************************************************
	 * SPI init:
	 *************************************************************************/
	SPI_voidInit(
		LCD_SPI_UNIT_NUMBER, SPI_Directional_Mode_Uni, SPI_DataFrameFormat_8bit,
		SPI_FrameDirection_MSB_First, SPI_Prescaler_2, SPI_Mode_Master,
		SPI_ClockPolarity_0Idle, SPI_ClockPhase_CaptureFirst);

	SPI_voidInitPins(LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, 0, 0, 1);

	SPI_ENABLE_PERIPHERAL(LCD_SPI_UNIT_NUMBER);

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
	// set EOC interrupt callback:
	ADC_voidSetInterruptCallback(OSC_voidEOCCallback);
	// enable EOC interrupt:
	ADC_voidEnableInterrupt(ADC_UnitNumber_1, ADC_Interrupt_EOC);
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
	NVIC_voidEnableInterrupt(NVIC_Interrupt_ADC1_2);
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
	TFT_voidInitBrightnessControl(
		&LCD, LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_FREQ_HZ,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	/*	set maximum brightness by default	*/
	TFT_SET_BRIGHTNESS(&LCD, POW_TWO(16) - 1);

	TFT_voidInit(&LCD, LCD_SPI_UNIT_NUMBER, LCD_RST_PIN, LCD_A0_PIN);

	/**************************************************************************
	 * frame init:
	 *************************************************************************/
	IMG_voidinitFrame(&frame, colorBlack);
	TFT_voidDrawFrame(&LCD, &frame);

	/*IMG_CURRENT_RECT(frame).color = colorRed;
	IMG_CURRENT_RECT(frame).pointStart = (Point_t){10, 10};
	IMG_CURRENT_RECT(frame).pointEnd = (Point_t){20, 20};
	frame.rectCount++;*/

	TFT_voidDrawFrame(&LCD, &frame);

	TFT_voidInitScroll(&LCD, 0, 162, 0);
}

/*
 * runs main super loop (no OS version)
 */
void OSC_voidRunMainSuperLoop(void)
{
	u8 tftScrollCounter = 0;
	Point_t p1 = {0, 0};
	Point_t p2 = {128, 0};
	u8 i;
	//u8 whereIsReadPointInLine[160] = {0};
	STK_voidInit();
	STK_voidStartTickMeasure(STK_TickMeasureType_OverflowCount);
	STK_voidEnableSysTick();
	while(1)
	{
		volatile u64 tStart = STK_u64GetElapsedTicks();
		/*	scroll TFT display	*/
		TFT_voidScroll(&LCD, tftScrollCounter);

		/*	draw reading on last displayed line	*/
		TFT_SET_BOUNDARIES(&LCD, p1, p2);
		TFT_WRITE_CMD(&LCD, 0x2C);
		SPI_SET_FRAME_FORMAT_16_BIT(LCD.spiUnit);
		GPIO_SET_PIN_HIGH(LCD.A0Port, LCD.A0Pin);
		i = 0;
		//	draw background color until 'adcRead':
		for (; i < ch1Val; i++)
			SPI_TRANSMIT(LCD.spiUnit, frame.backgroundColor.code565);
		// draw a single point with different color:
		SPI_TRANSMIT(LCD.spiUnit, colorRed.code565);
		//	draw background color until end of line:
		for (; i < 127; i++)
			SPI_TRANSMIT(LCD.spiUnit, frame.backgroundColor.code565);

		tftScrollCounter++;
		if (tftScrollCounter == 161)
			tftScrollCounter = 0;
		p1.y = tftScrollCounter;
		p2.y = tftScrollCounter;
		//Delay_voidBlockingDelayMs(50);
		volatile u64 tEnd = STK_u64GetElapsedTicks();
		trace_printf("%u ticks, ", (u32)(tEnd - tStart));
		trace_printf("%u us\n",
			(u32)(8000000 * (tEnd - tStart) / RCC_u32GetBusClk(RCC_Bus_AHB)));
	}
}



void OSC_voidEOCCallback(void)
{
	ch1Val = 127 -
		(u8)(((u32)ADC_u16GetDataRegular(ADC_UnitNumber_1)) * 127u / 4095u);
}








































