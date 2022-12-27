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

/*	MCAL	*/
#include "RCC_interface.h"
#include "SPI_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "STK_interface.h"

/*	HAL	*/
#include "TFT_interface.h"

/*	SELF	*/
#include "Loginc_Analyzer_config.h"
#include "Loginc_Analyzer_interface.h"


#include "SPI_private.h"

/*	static objects	*/
static TFT_t LCD;
static Frame_t frame;


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

	SPI_ENABLE_PERIPHERAL(LCD_SPI_UNIT_NUMBER);

	/**************************************************************************
	 * ADC init:
	 *************************************************************************/

}

/*
 * Inits all (HAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file, and static objects defined in "Loginc_Analyzer_program.c".
 */
void LA_voidInitHAL(void)
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
	/*
	for(u8 i = 0; i < 100; i++)
	{
		IMG_CURRENT_RECT(frame).color = colorRed;
		IMG_CURRENT_RECT(frame).pointStart = (Point_t){50, 50};
		IMG_CURRENT_RECT(frame).pointEnd = (Point_t){60, 60};
		frame.rectCount++;
	}

	STK_voidInit();
	STK_voidStartTickMeasure(STK_TickMeasureType_OverflowCount);
	STK_voidEnableSysTick();

	while(1)
	{
		u64 tStart = STK_u64GetElapsedTicks();
		TFT_voidDrawFrame(&LCD, &frame);
		u64 tEnd = STK_u64GetElapsedTicks();
		trace_printf("%u ticks, ", (u32)(tEnd - tStart));
		trace_printf("%u ms\n",
			(u32)(8000 * (tEnd - tStart) / RCC_u32GetBusClk(RCC_Bus_AHB)));
		//Delay_voidBlockingDelayMs(50);
	}
	*/

}














































