/*
 * Loginc_Analyzer_program.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"

/*	MCAL	*/
#include "RCC_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"

/*	HAL	*/

/*	SELF	*/
#include "Loginc_Analyzer_config.h"
#include "Loginc_Analyzer_interface.h"


/*
 * Inits all (MCAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file.
 */
void LA_voidInitMCAL(void)
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

	/**************************************************************************
	 * GPIO init:
	 *************************************************************************/
	GPIO_voidSetPinGpoPushPull(LCD_A0_PIN / 16, LCD_A0_PIN % 16);
	GPIO_voidSetPinGpoPushPull(LCD_RST_PIN / 16, LCD_RST_PIN % 16);
	GPIO_voidSetPinGpoPushPull(LED_INDICATOR_PIN / 16, LED_INDICATOR_PIN % 16);

	/**************************************************************************
	 * SPI init:
	 *************************************************************************/
	SPI_voidInit(
		LCD_SPI_UNIT_NUMBER, SPI_Directional_Mode_Uni, SPI_DataFrameFormat_8bit,
		SPI_FrameDirection_MSB_First, SPI_Prescaler_2, SPI_Mode_Master,
		SPI_ClockPolarity_0Idle, SPI_ClockPhase_CaptureFirst);

	SPI_voidInitPins(LCD_SPI_UNIT_NUMBER, LCD_SPI_AFIO_MAP, 0, 0, 1);

	SPI_voidEnableUnit(SPI_UnitNumber_1);

	/**************************************************************************
	 * TIM init:
	 *************************************************************************/
	TIM_u64InitPWM(
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_FREQ_HZ);

	TIM_voidInitOutputPin(
		LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER,
		LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL,
		LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP);

	TIM_voidEnableCounter(LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER);
}
















































