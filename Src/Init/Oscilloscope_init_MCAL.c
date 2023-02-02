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
#include "Img_interface.h"
#include "Colors.h"

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
extern void OSC_voidAutoCalibrate(void);
extern void OSC_voidMenuButtonCallback(void);
extern void OSC_voidStartFillingSampleBufferOnSignalRisingEdge(void);

extern volatile u16 Global_SampleBuffer[NUMBER_OF_SAMPLES];

void OSC_InitRCC(void)
{
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
	RCC_voidSetAdcPrescaler(RCC_ADC_Prescaler_PCLK2_by4);

	/*	TIM	*/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB1, RCC_PERIPHERAL_TIM3);
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
	/*	Indicator LED pin	*/
	GPIO_voidSetPinGpoPushPull(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);

	/**	Init digital inputs	**/
	/*	auto button pin	*/
	GPIO_voidSetPinInputPullDown(
		BUTTON_AUTO_ENTER_PIN / 16, BUTTON_AUTO_ENTER_PIN % 16);

	/*	pause / resume button pin	*/
	GPIO_voidSetPinInputPullDown(
		BUTTON_PAUSE_RESUME_PIN / 16, BUTTON_PAUSE_RESUME_PIN % 16);

	/*	menu button pin	*/
	GPIO_voidSetPinInputPullDown(
		BUTTON_CURSOR_MENU_PIN / 16, BUTTON_CURSOR_MENU_PIN % 16);

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
	EXTI_voidMapLine(BUTTON_AUTO_ENTER_PIN % 16, BUTTON_AUTO_ENTER_PIN / 16);

	EXTI_voidSetTriggeringEdge(
		BUTTON_AUTO_ENTER_PIN % 16, EXTI_Trigger_risingEdge);

	EXTI_voidSetCallBack(
		BUTTON_AUTO_ENTER_PIN % 16, OSC_voidAutoCalibrate);

	EXTI_voidEnableLine(BUTTON_AUTO_ENTER_PIN % 16);

	EXTI_voidEnableLineInterrupt(BUTTON_AUTO_ENTER_PIN % 16);

	/*	pause / resume button	*/
	EXTI_voidMapLine(
		BUTTON_PAUSE_RESUME_PIN % 16, BUTTON_PAUSE_RESUME_PIN / 16);

	EXTI_voidSetTriggeringEdge(
		BUTTON_PAUSE_RESUME_PIN % 16, EXTI_Trigger_risingEdge);

	EXTI_voidSetCallBack(BUTTON_PAUSE_RESUME_PIN % 16, OSC_voidTrigPauseResume);

	EXTI_voidEnableLine(BUTTON_PAUSE_RESUME_PIN % 16);

	EXTI_voidEnableLineInterrupt(BUTTON_PAUSE_RESUME_PIN % 16);

	/*	menu button	*/
	EXTI_voidMapLine(BUTTON_CURSOR_MENU_PIN % 16, BUTTON_CURSOR_MENU_PIN / 16);

	EXTI_voidSetTriggeringEdge(
		BUTTON_CURSOR_MENU_PIN % 16, EXTI_Trigger_risingEdge);

	EXTI_voidSetCallBack(
		BUTTON_CURSOR_MENU_PIN % 16, OSC_voidMenuButtonCallback);

	EXTI_voidEnableLine(BUTTON_CURSOR_MENU_PIN % 16);

	EXTI_voidEnableLineInterrupt(BUTTON_CURSOR_MENU_PIN % 16);

	/*	safe thread	*/
	EXTI_voidSetCallBack(
		SAFE_THREAD_EXTI_LINE,
		NULL);

	EXTI_voidEnableLine(SAFE_THREAD_EXTI_LINE);

	EXTI_voidEnableLineInterrupt(SAFE_THREAD_EXTI_LINE);
}

void OSC_InitADC(void)
{
	/*	enable continuous mode	*/
	//ADC_voidEnableContinuousConversionMode(ADC_UnitNumber_1);

	/*	set ADC channels sample time	*/
	for (u8 i = 0; i < CHANNEL_1_NUMBER_OF_LEVELS; i++)
	{
		ADC_ChannelNumber_t ch = oscCh1AdcChannels[i].adcChannelNumber;

		ADC_voidSetSampleTime(
			ADC_UnitNumber_1, ch, ADC_SAMPLE_TIME);
	}

	/*
	 * set AWD to all channels mode (any ways only one channel is being
	 * converted at a time
	 */
	ADC_voidSetAWDMode(ADC_UnitNumber_1, ADC_WatchdogMode_AllChannels);

	/*	set analog watchdog threshold values	*/
	ADC_voidSetAWDHighThreshold(ADC_UnitNumber_1, ADC_THRESHOLD_MAX);
	ADC_voidSetAWDLowThreshold(ADC_UnitNumber_1, ADC_THRESHOLD_MIN);

	/*	enable AWD	*/
	ADC_voidEnableAWDRegularCh(ADC_UnitNumber_1);

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

	/*	set regular channels trigger to TIM2CC2	*/
	ADC_voidSetExternalEventRegular(
		ADC_UnitNumber_1, ADC_ExternalEventRegular_TIM3TRGO);

	/*	enable conversion triggering on external event	*/
	ADC_voidEnableExternalTriggerRegular(ADC_UnitNumber_1);

	/*	Enable DMA request at end of conversion	*/
	ADC_voidEnableDMA(ADC_UnitNumber_1);

	/*	power on	*/
	ADC_voidEnablePower(ADC_UnitNumber_1);

	/*	calibrate	*/
	ADC_voidStartCalibration(ADC_UnitNumber_1);

	ADC_voidWaitCalibration(ADC_UnitNumber_1);

	/*	trigger start of conversion	*/
	//ADC_voidStartSWConversionRegular(ADC_UnitNumber_1);
}

void OSC_InitTIM(void)
{
	/*	init frequency measurement timer	*/
	u64 maxMeasurableFreqmHz;
	/*	start frequency measurement	*/
	TIM_voidInitFreqAndDutyMeasurement(
		FREQ_MEASURE_TIMER_UNIT_NUMBER,
		FREQ_MEASURE_TIMER_UNIT_AFIO_MAP,
		FREQ_MEASURE_MIN_FREQ_MILLI_HZ,
		&maxMeasurableFreqmHz,
		NULL, NULL);

	#if DEBUG_ON
	trace_printf(
		"maximum measurable frequency is: %dHz\n",
		(u32)(maxMeasurableFreqmHz / 1000));
	#endif

	/**	init ADC sampling timer	**/
	/*	disable slave mode (use clk_int as clk source)	*/
	TIM_voidSetSlaveMode(3, TIM_SlaveMode_Disabled);

	/*	set it up-counting	*/
	TIM_voidSetCounterDirection(3, TIM_CountDirection_Up);

	/*	set update request source to OVF/UNF only	*/
	TIM_voidSetUpdateSource(3, TIM_UpdateSource_OVF_UNF);

	/*	init trigger output on update event	*/
	TIM_voidSetMasterModeSelection(3, TIM_MasterMode_Update);

	/*	set freq	*/
	TIM_u64SetFrequency(3, INITIAL_SAMPLING_FREQUENCY);

	/*	Enable counter	*/
	//TIM_voidEnableCounter(3);

	/*	init PWM	*/
	/*TIM_u64InitPWM(
		ADC_SAMPLING_TRIGGER_TIMER_UNIT_NUMBER,
		ADC_SAMPLING_TRIGGER_TIMER_CHANNEL,
		TIM_OutputCompareMode_PWM1, INITIAL_SAMPLING_FREQUENCY);

	TIM_voidSetDutyCycle(
		ADC_SAMPLING_TRIGGER_TIMER_UNIT_NUMBER,
		ADC_SAMPLING_TRIGGER_TIMER_CHANNEL,
		POW_TWO(15));*/

	/*	Enable counter	*/
	//TIM_voidEnableCounter(ADC_SAMPLING_TRIGGER_TIMER_UNIT_NUMBER);
}

void OSC_InitDMA(void)
{
	/**
	 *	Init the 3 channels used in filling a single line in the display.
	 *	(mem to mem operations)
	 **/
	/*	enable DMA1 clock (if not enabled)	*/
	DMA_voidEnableRCCClock(DMA_UnitNumber_1);

	const DMA_ChannelNumber_t dmaChannels[] = {
		FIRST_LINE_SEGMENT_DMA_CHANNEL,
		SECOND_LINE_SEGMENT_DMA_CHANNEL,
		THIRD_LINE_SEGMENT_DMA_CHANNEL
	};

	for (u8 i = 0; i < 3; i++)
	{
		/*	Source size is 16-bit	*/
		DMA_voidSelectPeripheralSize(
			DMA_UnitNumber_1, dmaChannels[i], DMA_Size_16bits);

		/*	Destination size is 16-bit	*/
		DMA_voidSelectMemorySize(
			DMA_UnitNumber_1, dmaChannels[i], DMA_Size_16bits);

		/*
		 * set direction to read peripheral
		 * (i.e.: the memory location of color data)
		 */
		DMA_voidSelectDataTransferDirection(
			DMA_UnitNumber_1, dmaChannels[i], DMA_Direction_ReadPeripheral);

		/*	Enable memory increment	*/
		DMA_voidEnableMemoryIncrement(DMA_UnitNumber_1, dmaChannels[i]);

		/*	Enable mem to mem mode	*/
		DMA_voidEnableMemToMemMode(DMA_UnitNumber_1, dmaChannels[i]);
	}

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

	/**	init ADC end of conversion DMA transfer to sample buffer	**/
	/*	Source size is 16-bit	*/
	DMA_voidSelectPeripheralSize(
		DMA_UnitNumber_1, ADC_DMA_CHANNEL, DMA_Size_16bits);

	/*	Destination size is 16-bit	*/
	DMA_voidSelectMemorySize(
		DMA_UnitNumber_1, ADC_DMA_CHANNEL, DMA_Size_16bits);

	/*	set direction to read peripheral	*/
	DMA_voidSelectDataTransferDirection(
		DMA_UnitNumber_1, ADC_DMA_CHANNEL, DMA_Direction_ReadPeripheral);

	/*	Enable memory increment	*/
	DMA_voidEnableMemoryIncrement(DMA_UnitNumber_1, ADC_DMA_CHANNEL);

	/*	set destination address	*/
	DMA_voidSetMemoryAddress(
		DMA_UnitNumber_1, ADC_DMA_CHANNEL, &Global_SampleBuffer[0]);

	/*	set number of data	*/
	DMA_voidSetNumberOfData(
		DMA_UnitNumber_1, ADC_DMA_CHANNEL, NUMBER_OF_SAMPLES);

	/*	enable auto reload mode (circular mode)	*/
	DMA_voidEnableCircularMode(DMA_UnitNumber_1, ADC_DMA_CHANNEL);
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
//	timTrigLineDrawingInterrupt =
//			TIM_u8GetUpdateEventInterruptNumber(
//				LCD_REFRESH_TRIGGER_TIMER_UNIT_NUMBER);
//
//	NVIC_Interrupt_t timTrigInfoDrawingInterrupt =
//			TIM_u8GetUpdateEventInterruptNumber(
//				LCD_INFO_DRAWING_TRIGGER_TIMER_UNIT_NUMBER);
//
//	tftDmaInterruptNumber = DMA_u8GetInterruptVectorIndex(
//		DMA_UnitNumber_1,
//		(LCD_SPI_UNIT_NUMBER == SPI_UnitNumber_1 ?
//			DMA_ChannelNumber_3 : DMA_ChannelNumber_5));
//
//	NVIC_voidEnableInterrupt(timTrigLineDrawingInterrupt);
//	NVIC_voidEnableInterrupt(timTrigInfoDrawingInterrupt);
//
//	/**
//	 * see comments in "OSC_InitSCB()"
//	 */
//	/*NVIC_voidSetInterruptPriority(
//		tftDmaInterruptNumber, 0, 0);
//
//	NVIC_voidSetInterruptPriority(
//		timTrigLineDrawingInterrupt, 1, 0);
//
//	NVIC_voidSetInterruptPriority(
//		timTrigInfoDrawingInterrupt, 1, 0);*/
}

void OSC_voidInitMCAL(void)
{
	OSC_InitRCC();

	OSC_InitSysTick();

	OSC_InitGPIO();

	OSC_InitTIM();

	OSC_InitEXTI();

	OSC_InitDMA();

	OSC_InitADC();

	OSC_InitSCB();

	OSC_InitNVIC();
}
