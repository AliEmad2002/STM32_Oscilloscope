/*
 * Oscilloscope_init_MCAL.c
 *
 *  Created on: Jan 18, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Debug_active.h"

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

/*
 * HAL
 * (for 'Oscilloscope_Private.h' dependency
 */
#include "TFT_interface_V2.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_init_MCAL.h"

/**	extern buttons callback functions	*/
extern void OSC_voidTrigPauseResume(void);
extern void OSC_voidEnterMenuMode(void);
extern void OSC_voidAutoCalibVoltAndTimePerDiv(void);


void OSC_InitRCC(void)
{
	/*	clock configurations are in 'RCC_config.h'	*/
	RCC_voidSysClockInit();

	/*	GPIO/AFIO	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPA);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPB);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPC);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_AFIO);

	/*	EXTI	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_EXTI);

	/*	ADC	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC1);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_ADC2);
	RCC_voidSetAdcPrescaler(RCC_ADC_Prescaler_PCLK2_by6);
}

void OSC_InitSysTick(void)
{
	STK_voidInit();
	STK_voidStartTickMeasure(STK_TickMeasureType_OverflowCount);
	STK_voidEnableSysTick();

}

void OSC_InitGPIO(void)
{
	/**	Init digital outputs	**/
	/*	indecator LED pin	*/
	GPIO_voidSetPinGpoPushPull(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);

	/**	Init digital inputs	**/
	/*	auto button pin	*/
	GPIO_voidSetPinInputPullDown(BUTTON_AUTO_PIN / 16, BUTTON_AUTO_PIN % 16);

	/*	menu button pin	*/
	GPIO_voidSetPinInputPullDown(BUTTON_MENU_PIN / 16, BUTTON_MENU_PIN % 16);

	/*	pause / resume button pin	*/
	GPIO_voidSetPinInputPullDown(
		BUTTON_PAUSE_RESUME_PIN / 16, BUTTON_PAUSE_RESUME_PIN % 16);

	/**	Init analog inputs	**/
	/*	init all (oscilloscope channel 1)'s ADC channels	*/
	for (u8 i = 0; i < CHANNEL_1_NUMBER_OF_LEVELS; i++)
	{
		GPIO_Pin_t pin =
			ADC_u8GetPortAndPinOfChannel(oscCh1AdcChannels[i].adcChannelNumber);

		GPIO_voidSetPinMode(pin / 16, pin % 16, GPIO_Mode_Input_Analog);

		#if DEBUG_ON
		trace_printf(
			"osc_ch1 is connected to port: %d, pin: %d\n", pin / 16, pin % 16);
		#endif
	}
}

void OSC_InitEXTI(void)
{
	/*	auto button	*/
	EXTI_voidMapLine(BUTTON_AUTO_PIN % 16, BUTTON_AUTO_PIN / 16);

	EXTI_voidSetTriggeringEdge(BUTTON_AUTO_PIN % 16, EXTI_Trigger_risingEdge);

	EXTI_voidSetCallBack(
		BUTTON_AUTO_PIN % 16, OSC_voidAutoCalibVoltAndTimePerDiv);

	EXTI_voidEnableLine(BUTTON_AUTO_PIN % 16);

	EXTI_voidEnableLineInterrupt(BUTTON_AUTO_PIN % 16);

	/*	pause / resume button	*/
	EXTI_voidMapLine(
		BUTTON_PAUSE_RESUME_PIN % 16, BUTTON_PAUSE_RESUME_PIN / 16);

	EXTI_voidSetTriggeringEdge(
		BUTTON_PAUSE_RESUME_PIN % 16, EXTI_Trigger_risingEdge);

	EXTI_voidSetCallBack(BUTTON_AUTO_PIN % 16, OSC_voidTrigPauseResume);

	EXTI_voidEnableLine(BUTTON_PAUSE_RESUME_PIN % 16);

	EXTI_voidEnableLineInterrupt(BUTTON_PAUSE_RESUME_PIN % 16);

	/*	menu button	*/
	EXTI_voidMapLine(BUTTON_MENU_PIN % 16, BUTTON_MENU_PIN / 16);

	EXTI_voidSetTriggeringEdge(BUTTON_MENU_PIN % 16, EXTI_Trigger_risingEdge);

	EXTI_voidSetCallBack(
			BUTTON_MENU_PIN % 16, OSC_voidEnterMenuMode);

	EXTI_voidEnableLine(BUTTON_MENU_PIN % 16);

	EXTI_voidEnableLineInterrupt(BUTTON_MENU_PIN % 16);
}


void OSC_InitADC(void)
{
	/*	enable continuous mode	*/
	ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_1);

	/*	set ADC channels sample time	*/
	for (u8 i = 0; i < CHANNEL_1_NUMBER_OF_LEVELS; i++)
	{
		ADC_ChannelNumber_t ch = oscCh1AdcChannels[i].adcChannelNumber;

		ADC_voidSetSampleTime(
			ADC_UnitNumber_1, ch, ADC_SAMPLE_TIME);
	}

	/**
	 * Because only one ADC channel is to be used during signal plotting (this
	 * channel is determined depending on volts per div settings), initially
	 * use the very first ADC channel in the ADC channel array.
	 */
	ADC_ChannelNumber_t ch1st = oscCh1AdcChannels[0].adcChannelNumber;

	/*	write channel in regular sequence	*/
	ADC_voidSetSequenceRegular(
		ADC_UnitNumber_1, ADC_RegularSequenceNumber_1, ch1st);

	/*
	 * set regular sequence len to 1. (as only one channel is to be converted)
	 */
	ADC_voidSetSequenceLenRegular(ADC_UnitNumber_1, 1);

	/*	set regular channels trigger to SWSTART	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_SWSTART);

	/*	enable conversion triggering on external event	*/
	ADC_voidEnableExternalTriggerRegular(ADC_UnitNumber_1);

	/*	power on	*/
	ADC_voidEnablePower(ADC_UnitNumber_1);

	/*	calibrate	*/
	ADC_voidStartCalibration(ADC_UnitNumber_1);

	ADC_voidWaitCalibration(ADC_UnitNumber_1);

	/*	trigger start of conversion	*/
	ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);
}

void OSC_InitTIM(void)
{
	/*	start frequency measurement	*/
	TIM_voidInitFreqAndDutyMeasurement(
		FREQ_MEASURE_TIMER_UNIT_NUMBER,
		FREQ_MEASURE_TIMER_UNIT_AFIO_MAP,
		FREQ_MEASURE_MIN_FREQ_MILLI_HZ);
}

void OSC_InitSCB(void)
{
	/**
	 * TODO: Once this function is called, PRIGROUP bits are written with the
	 * key, the machine suffers a hard fault.
	 * Some search on the Internet says that it's structural fault in
	 * blue-pills. Check that ASAP!
	 *
	 * due to this problem, program is handled with single level of prioring
	 * and subgrouping.
	 */

	/*	configure number of NVIC groups and sub groups	*/
	//SCB_voidSetPriorityGroupsAndSubGroupsNumber(SCB_PRIGROUP_group16_sub0);
}

void OSC_InitNVIC(void)
{
	timTrigLineDrawingInterrupt =
			TIM_u8GetUpdateEventInterruptNumber(
				LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);

	NVIC_Interrupt_t timTrigInfoDrawingInterrupt =
			TIM_u8GetUpdateEventInterruptNumber(
				LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);

	tftDmaInterruptNumber = DMA_u8GetInterruptVectorIndex(
		DMA_UnitNumber_1,
		(LCD_SPI_UNIT_NUMBER == SPI_UnitNumber_1 ?
			DMA_ChannelNumber_3 : DMA_ChannelNumber_5));

	NVIC_voidEnableInterrupt(timTrigLineDrawingInterrupt);
	NVIC_voidEnableInterrupt(timTrigInfoDrawingInterrupt);

	/**
	 * see comments in "OSC_InitSCB()"
	 */
	/*NVIC_voidSetInterruptPriority(
		tftDmaInterruptNumber, 0, 0);

	NVIC_voidSetInterruptPriority(
		timTrigLineDrawingInterrupt, 1, 0);

	NVIC_voidSetInterruptPriority(
		timTrigInfoDrawingInterrupt, 1, 0);*/
}

void OSC_voidInitMCAL(void)
{
	OSC_InitRCC();

	OSC_InitSysTick();

	OSC_InitGPIO();

	OSC_InitEXTI();

	OSC_InitADC();

	OSC_InitTIM();

	OSC_InitSCB();

	OSC_InitNVIC();
}