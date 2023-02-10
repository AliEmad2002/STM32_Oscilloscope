/*
 * Oscilloscope_Check_list.c
 *
 *  Created on: Feb 10, 2023
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
#include "Oscilloscope_Info.h"

extern OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

extern volatile TFT2_t Global_LCD;
extern volatile u16* Global_ImgBufferArr[2];
extern volatile u8 Global_NotInUseImgBufferIndex;
extern volatile char Global_Str[128];

extern volatile Rotary_Encoder_t OSC_RotaryEncoder;

Check_List_Element_t Global_infoCheckListElementArr[NUMBER_OF_INFO + 1];

Check_List_t Global_infoCheckList;

void OSC_voidInitInfoCheckList(void)
{
	/*	link "Global_InfoArr" to "Global_infoCheckListElementArr"	*/
	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
	{
		Global_infoCheckListElementArr[i].str = Global_InfoArr[i].name;

		Global_infoCheckListElementArr[i].isChecked =
			&(Global_InfoArr[i].enabled);

		Global_infoCheckListElementArr[i].isHidden = false;
	}

	/*	add "return" element	*/
	static char returnStr[] = "Return";
	Global_infoCheckListElementArr[NUMBER_OF_INFO].str = returnStr;
	Global_infoCheckListElementArr[NUMBER_OF_INFO].isHidden = false;

	/*	link "Global_infoCheckListElementArr" to "Global_infoCheckList"	*/
	Global_infoCheckList.elementArr = Global_infoCheckListElementArr;

	Global_infoCheckList.numberOfElements = NUMBER_OF_INFO + 1;

	/*	normally start with zero	*/
	Global_infoCheckList.currentSelectedElement = 0;
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
	TFT2_voidFillDMA(&Global_LCD, &LCD_BACKGROUND_COLOR_U8, 128 * 160);

	/**	draw check-list lines	**/
	for (u8 i = 0; i < checkListPtr->numberOfElements; i++)
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
				Global_Str, "%c %s", CHECKED_SYMBOL_ASCII_CODE,
				checkListPtr->elementArr[i].str);
		}
		else
		{
			sprintf(
				Global_Str, "%c %s", UNCHECKED_SYMBOL_ASCII_CODE,
				checkListPtr->elementArr[i].str);
		}

		/*	wait for previous transfer completion	*/
		TFT2_voidWaitCurrentDataTransfer(&Global_LCD);

		/*	print txt on img buffer that is not in use (free of DMA usage)	*/
		Txt_voidPrintStrOnPixArrRightOrientation(
			Global_Str, fontColor, bgColor, 0, 0,
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
			&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex], 160 * 8);
	}

	/*	wait for transfer completion	*/
	TFT2_voidWaitCurrentDataTransfer(&Global_LCD);
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
			if (checkListPtr->currentSelectedElement == NUMBER_OF_INFO)
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











