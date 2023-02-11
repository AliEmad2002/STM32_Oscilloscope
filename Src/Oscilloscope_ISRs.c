/*
 * Oscilloscope_ISRs.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Debug_active.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Delay_interface.h"

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
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Div.h"
#include "Oscilloscope_Offset.h"
#include "Oscilloscope_Brightness.h"
#include "Oscilloscope_AutoCalib.h"
#include "Oscilloscope_ISRs.h"

void OSC_voidTrigPauseResume(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
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
		ROTARY_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
	{
		return;
	}

	switch (Global_UpDownTarget)
	{
	case OSC_Up_Down_Target_ChangeCh1VoltageDiv:
		OSC_voidIncrementCh1VoltageDiv();
		break;

	case OSC_Up_Down_Target_ChangeCh2VoltageDiv:
		OSC_voidIncrementCh2VoltageDiv();
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

	case OSC_Up_Down_Target_ChangeCh1Offset:
		OSC_voidIncrementCh1Offset();
		break;

	case OSC_Up_Down_Target_ChangeCh2Offset:
		OSC_voidIncrementCh2Offset();
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
		ROTARY_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
	{
		return;
	}

	switch (Global_UpDownTarget)
	{
	case OSC_Up_Down_Target_ChangeCh1VoltageDiv:
		OSC_voidDecrementCh1VoltageDiv();
		break;

	case OSC_Up_Down_Target_ChangeCh2VoltageDiv:
		OSC_voidDecrementCh2VoltageDiv();
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

	case OSC_Up_Down_Target_ChangeCh1Offset:
		OSC_voidDecrementCh1Offset();
		break;

	case OSC_Up_Down_Target_ChangeCh2Offset:
		OSC_voidDecrementCh2Offset();
		break;

	case OSC_Up_Down_Target_ChangeBrightness:
		OSC_voidDecrementBrightness();
		break;
	}

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}

void OSC_voidMenuButtonCallback(void)
{
//	/*	check debouncing first	*/
//	static u64 lastPressTime = 0;
//
//	if (
//		STK_u64GetElapsedTicks() - lastPressTime <
//		BUTTON_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
//	{
//		return;
//	}

	/*	set opened menu flag	*/
	Global_IsMenuOpen = true;

//	/*	debouncing timestamp	*/
//	lastPressTime = STK_u64GetElapsedTicks();
}

void OSC_voidAutoEnterMenuButtonCallback(void)
{
	static u64 lastPressTime = 0;

	if (
		STK_u64GetElapsedTicks() - lastPressTime <
		BUTTON_DEBOUNCING_TIME_MS * STK_TICKS_PER_MS)
	{
		return;
	}

	/*
	 * wait 500ms, if button was not yet released enter menu,
	 * otherwise do auto calibration
	 */
	Delay_voidBlockingDelayMs(BUTTON_DEBOUNCING_TIME_MS * 2);

	if (
		GPIO_DIGITAL_READ(
			BUTTON_AUTO_ENTER_MENU_PIN / 16, BUTTON_AUTO_ENTER_MENU_PIN % 16)
	)
	{
		/*	wait for button release	*/
		while (
			GPIO_DIGITAL_READ(
				BUTTON_AUTO_ENTER_MENU_PIN / 16,
				BUTTON_AUTO_ENTER_MENU_PIN % 16)
		);

		/*	debouncing	*/
		Delay_voidBlockingDelayMs(BUTTON_DEBOUNCING_TIME_MS);

		/*	execute menu callback	*/
		OSC_voidMenuButtonCallback();
	}

	else
	{
		OSC_voidAutoCalibrate();
	}

	/*	debouncing timestamp	*/
	lastPressTime = STK_u64GetElapsedTicks();
}
