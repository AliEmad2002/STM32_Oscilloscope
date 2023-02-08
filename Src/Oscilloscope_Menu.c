/*
 * Oscilloscope_Menu.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Menu_config.h"
#include "Menu_interface.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Txt_interface.h"
#include "Delay_interface.h"
#include "diag/trace.h"

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
#include "Rotary_Encoder_Interface.h"

/*	SELF	*/
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Menu_Private.h"
#include "Oscilloscope_Menu_Interface.h"

extern volatile TFT2_t Global_LCD;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_IsMenuOpen;

extern volatile Rotary_Encoder_t OSC_RotaryEncoder;

extern void OSC_voidSetDisplayBoundariesForSignalArea(void);


/*	pixel array of a single line	*/
u16 menuLinePixArr[8][128];

void OSC_voidUpdateMenuOnDisplay(Menu_t* menu)
{
	/**	fill the screen with background color	**/
	/*	set boundaries (full screen)	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

	/*	start data write operation	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	draw background	*/
	TFT2_voidFillDMA(&Global_LCD, &LCD_BACKGROUND_COLOR_U8, 128 * 160);

	/**	draw menu lines	**/
	/*
	 * loop on menu elements,
	 */
	for (u8 i = 0; i < menu->numberOfElements; i++)
	{
		u16 bgColor, fontColor;

		/*	if the selected menu element is to be drawn, invert colors	*/
		if (i == menu->currentSelected)
		{
			bgColor = LCD_MAIN_DRAWING_COLOR_U16;
			fontColor = LCD_BACKGROUND_COLOR_U16;
		}
		/*	otherwise, draw menu element normally	*/
		else
		{
			bgColor = LCD_BACKGROUND_COLOR_U16;
			fontColor = LCD_MAIN_DRAWING_COLOR_U16;
		}

		/*	wait for previous transfer completion	*/
		TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

		// TODO: orientation!
		Txt_voidCpyStrToStaticPixArrNormalOrientation(
			menu->elementArr[i].str, fontColor, bgColor, 1,
			Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
			0, 0, 8, 128, menuLinePixArr);

		/*	set boundaries 	*/
		TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
		TFT2_SET_Y_BOUNDARIES(&Global_LCD, 8 * i, 8 * (i + 1));

		/*	start data write operation	*/
		TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
		TFT2_ENTER_DATA_MODE(&Global_LCD);

		/*	send	*/
		TFT2_voidSendPixels(&Global_LCD, (u16*)menuLinePixArr, 128 * 8);
	}

	/*	wait for transfer completion	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);
}

void OSC_voidOpenMainMenu(void)
{
	/*	disable rotary encoder callbacks	*/
	OSC_RotaryEncoder.countUpCallbackEnabled = false;
	OSC_RotaryEncoder.countDownCallbackEnabled = false;

	/*	disable auto, pause and menu buttons EXTI lines	*/
	EXTI_voidDisableLine(BUTTON_AUTO_ENTER_MENU_PIN % 16);
	EXTI_voidDisableLine(BUTTON_PAUSE_RESUME_PIN % 16);

	OSC_voidOpenMenu(&mainMenu);

	/*	set screen boundaries for full signal image area	*/
	OSC_voidSetDisplayBoundariesForSignalArea();

	/*	clear opened menu flag	*/
	Global_IsMenuOpen = false;

	/*	enable back rotary encoder callbacks	*/
	OSC_RotaryEncoder.countUpCallbackEnabled = true;
	OSC_RotaryEncoder.countDownCallbackEnabled = true;

	/*	enable back auto, pause and menu buttons EXTI lines	*/
	EXTI_voidEnableLine(BUTTON_AUTO_ENTER_MENU_PIN % 16);
	EXTI_voidEnableLine(BUTTON_PAUSE_RESUME_PIN % 16);
}

void OSC_voidOpenMenu(Menu_t* menu)
{
	/*	reset selection	*/
	menu->currentSelected = 0;

	/*	show menu on the screen	*/
	OSC_voidUpdateMenuOnDisplay(menu);

	s32 lastRotaryCount = OSC_RotaryEncoder.count;

	while(1)
	{
		/*	if user moves up	*/
		if (OSC_RotaryEncoder.count > lastRotaryCount)
		{
			/*	select next element in menu	*/
			Menu_voidSelectNextElement(menu);
			/*	update display	*/
			OSC_voidUpdateMenuOnDisplay(menu);
			/*	debouncing delay	*/
			//Delay_voidBlockingDelayMs(150);
			/*	update "lastRotaryCount"	*/
			lastRotaryCount = OSC_RotaryEncoder.count;
		}

		/*	if user moves down	*/
		if (OSC_RotaryEncoder.count < lastRotaryCount)
		{
			/*	select previous element in menu	*/
			Menu_voidSelectPreviousElement(menu);
			/*	update display	*/
			OSC_voidUpdateMenuOnDisplay(menu);
			/*	debouncing delay	*/
			//Delay_voidBlockingDelayMs(150);
			/*	update "lastRotaryCount"	*/
			lastRotaryCount = OSC_RotaryEncoder.count;
		}

		/*	if user pressed auto/enter button	*/
		if (
			GPIO_DIGITAL_READ(
				BUTTON_AUTO_ENTER_MENU_PIN / 16,
				BUTTON_AUTO_ENTER_MENU_PIN % 16))
		{
			/*	debouncing delay	*/
			Delay_voidBlockingDelayMs(150);
			/*	execute callback of current selected element	*/
			Menu_voidEnterSelectedElement(menu);
			/*	exit this menu on selected element callback return	*/
			break;
		}
	}
}

/*	callback functions	*/
void OSC_voidSelectChangeCh1VoltageDivAsUpDownTraget(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeCh1VoltageDiv;
}

void OSC_voidSelectChangeCh2VoltageDivAsUpDownTraget(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeCh2VoltageDiv;
}

void OSC_voidSelectChangeTimeDivAsUpDownTraget(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeTimeDiv;
}

void OSC_voidSelectChangeV1Position(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeVoltageCursor1Position;
}

void OSC_voidSelectChangeV2Position(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeVoltageCursor2Position;
}

void OSC_voidSelectChangeT1Position(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeTimeCursor1Position;
}

void OSC_voidSelectChangeT2Position(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeTimeCursor2Position;
}

void OSC_voidSelectChangeCh1Offset(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeCh1Offset;
}

void OSC_voidSelectChangeCh2Offset(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeCh2Offset;
}

void OSC_voidSelectChangeBrightness(void)
{
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeBrightness;
}





