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
#include "Check_List_interface.h"
#include <stdio.h>
#include "LinkedList.h"
#include "MathParser.h"

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
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Draw.h"
#include "Oscilloscope_Info.h"
#include "Oscilloscope_Info_Init.h"
#include "Oscilloscope_Menu_Private.h"
#include "Oscilloscope_Menu_Interface.h"
#include "Oscilloscope_Menu_Init.h"

/*	main menu	*/
volatile Menu_t Global_MainMenu;

extern volatile Rotary_Encoder_t OSC_RotaryEncoder;

void OSC_voidOpenMainMenu(void)
{
	/*	disable rotary encoder callbacks	*/
	OSC_RotaryEncoder.countUpCallbackEnabled = false;
	OSC_RotaryEncoder.countDownCallbackEnabled = false;

	/*	disable auto, pause and menu buttons EXTI lines	*/
	EXTI_voidDisableLine(BUTTON_AUTO_ENTER_MENU_PIN % 16);
	EXTI_voidDisableLine(BUTTON_PAUSE_RESUME_PIN % 16);

	OSC_voidOpenMenu((Menu_t*)&Global_MainMenu);

	/**	fill the screen with background color	**/
	OSC_voidFillDisplayWithBGColor();

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

void OSC_voidUpdateMenuOnDisplay(Menu_t* menu)
{
	/**	fill the screen with background color	**/
	OSC_voidFillDisplayWithBGColor();

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
		TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);

		/*	print txt on img buffer that is not in use (free of DMA usage)	*/
		Txt_voidPrintStrOnPixArrRightOrientation(
			menu->elementArr[i].str, fontColor, bgColor, 0, 0,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex],
			8, 160);

		/*	set boundaries 	*/
		TFT2_SET_X_BOUNDARIES(&Global_LCD, 128 - 8 * (i + 1), 127 - 8 * i);
		TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

		/*	start data write operation	*/
		TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
		TFT2_ENTER_DATA_MODE(&Global_LCD);

		/*	send	*/
		TFT2_voidSendPixels(
			(TFT2_t*)&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex], 160 * 8);
	}

	/*	wait for transfer completion	*/
	TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);
}

void OSC_voidOpenCheckList(Check_List_t* checkListPtr)
{
	/*	reset selection	*/


	/*	show check-list on the screen	*/
	OSC_voidUpdateCheckListOnDisplay(checkListPtr);

	s32 lastRotaryCount = OSC_RotaryEncoder.count;

	while(1)
	{
		/*	if user moves up	*/
		if (OSC_RotaryEncoder.count > lastRotaryCount)
		{
			/*	select next element in check-list	*/
			Check_List_voidSelecttNextElement(checkListPtr);
			/*	update display	*/
			OSC_voidUpdateCheckListOnDisplay(checkListPtr);
			/*	update "lastRotaryCount"	*/
			lastRotaryCount = OSC_RotaryEncoder.count;
		}

		/*	if user moves down	*/
		if (OSC_RotaryEncoder.count < lastRotaryCount)
		{
			/*	select previous element in check-list	*/
			Check_List_voidSelecttPreviousElement(checkListPtr);
			/*	update display	*/
			OSC_voidUpdateCheckListOnDisplay(checkListPtr);
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

			/*	break if user selects "return"	*/
			if (
				checkListPtr->currentSelectedElement ==
				checkListPtr->numberOfElements - 1
			)
				break;

			/*
			 * otherwise, toggle checked flag of current selected element,
			 * and update display.
			 */
			CHECK_LIST_TOGGLE_CHECK(
				checkListPtr, checkListPtr->currentSelectedElement);

			OSC_voidUpdateCheckListOnDisplay(checkListPtr);
		}
	}
}

void OSC_voidUpdateCheckListOnDisplay(Check_List_t* checkListPtr)
{
	/**	fill the screen with background color	**/
	/*	set boundaries (full screen)	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

	/*	start data write operation	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	draw background	*/
	TFT2_voidFillDMA((TFT2_t*)&Global_LCD, &LCD_BACKGROUND_COLOR_U8, 128 * 160);

	/**	draw check-list lines	**/
	u8 iStart;
	u8 iMax;
	if(checkListPtr->numberOfElements > 15)
	{
		iStart = checkListPtr->currentSelectedElement;
		iMax = iStart + 15;

		if (iMax > checkListPtr->numberOfElements)
			iMax = checkListPtr->numberOfElements;
	}

	else
	{
		iStart = 0;
		iMax = checkListPtr->numberOfElements;
	}

	for (u8 i = iStart; i < iMax; i++)
	{
		/*	if 'i'th element is hidden, continue	*/
		if (CHECK_LIST_IS_ELEMENT_HIDDEN(checkListPtr, i))
			continue;

		u16 bgColor, fontColor;

		/*
		 * if the selected check-list element is to be drawn, invert colors.
		 */
		if (i == checkListPtr->currentSelectedElement)
		{
			bgColor = LCD_MAIN_DRAWING_COLOR_U16;
			fontColor = LCD_BACKGROUND_COLOR_U16;
		}
		/*	otherwise, draw menu element normally.	*/
		else
		{
			bgColor = LCD_BACKGROUND_COLOR_U16;
			fontColor = LCD_MAIN_DRAWING_COLOR_U16;
		}

		/*	add check, not checked symbol	*/
		if (*(checkListPtr->elementArr[i].isChecked))
		{
			sprintf(
				(char*)Global_Str, "%c %s", CHECKED_SYMBOL_ASCII_CODE,
				checkListPtr->elementArr[i].str);
		}
		else
		{
			sprintf(
				(char*)Global_Str, "%c %s", UNCHECKED_SYMBOL_ASCII_CODE,
				checkListPtr->elementArr[i].str);
		}

		/*	wait for previous transfer completion	*/
		TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);

		/*	print txt on img buffer that is not in use (free of DMA usage)	*/
		Txt_voidPrintStrOnPixArrRightOrientation(
			(char*)Global_Str, fontColor, bgColor, 0, 0,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex],
			8, 160);

		/*	set boundaries 	*/
		TFT2_SET_X_BOUNDARIES(&Global_LCD, 128 - 8 * ((i - iStart) + 1), 127 - 8 * (i - iStart));
		TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

		/*	start data write operation	*/
		TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
		TFT2_ENTER_DATA_MODE(&Global_LCD);

		/*	send	*/
		TFT2_voidSendPixels(
			(TFT2_t*)&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex], 160 * 8);
	}

	/*	wait for transfer completion	*/
	TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);
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





