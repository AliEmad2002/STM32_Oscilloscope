/*
 * Oscilloscope_Draw.c
 *
 *  Created on: Feb 16, 2023
 *      Author: ali20
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
#include "Error_Handler_interface.h"
#include "Txt_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "My_Math.h"
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
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Conversions.h"
#include "Oscilloscope_Draw.h"


void OSC_voidSetDisplayBoundariesForSignalArea(void)
{
	/*	set screen boundaries for full signal image area	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, SIGNAL_IMG_X_MIN, SIGNAL_IMG_X_MAX);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, NUMBER_OF_SAMPLES - 1);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);
}

void OSC_voidFillDisplayWithBGColor(void)
{
	/*	set screen boundaries for full screen	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 0, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	/*	fill	*/
	TFT2_voidFillDMA((TFT2_t*)&Global_LCD, &LCD_BACKGROUND_COLOR_U8, 128 * 160);

	/*	wait for TC	*/
	TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);
}

void OSC_voidEditStrtingOnDislpay(char* str)
{
	static const char* keyPadToAlphabet[] = {
		/*	basic	*/
					"0",
		"1"		, "2abc"	, "3def",
		"4ghi"	, "5jkl"	, "6mno",
		"7pqrs"	, "8tuv"	, "9wxyz",

		/*	others	*/
		"+", "-", "*", "/", "x", "y"
	};

	/*	clear whole display (fill with BG color)	*/
	OSC_voidFillDisplayWithBGColor();

	/*
	 * set drawing boundaries to first line only. (on which expression is to be
	 * edited)	*/
	TFT2_SET_X_BOUNDARIES(&Global_LCD, 120, 127);
	TFT2_SET_Y_BOUNDARIES(&Global_LCD, 0, 159);

	/*	start data writing mode on screen	*/
	TFT2_WRITE_CMD(&Global_LCD, TFT_CMD_MEM_WRITE);
	TFT2_ENTER_DATA_MODE(&Global_LCD);

	u8 firstEmptyCharIndex = strlen(str);

	/*
	 * index of the character to be used from
	 * "keyPadToAlphabet[IR_pressed_button]"
	 */
	u8 alphaIndex = 0;

	u8 prevIrData = 0;

	u64 prevIrTimeStamp = 0;

	while(1)
	{
		/*	clear first line of display (string area)	*/
		TFT2_voidFillDMA(
			(TFT2_t*)&Global_LCD, &LCD_BACKGROUND_COLOR_U8, 8 * 160);

		/*	print str on image buffer	*/
		Txt_voidPrintStrOnPixArrRightOrientation(
			str, LCD_MAIN_DRAWING_COLOR_U16, LCD_BACKGROUND_COLOR_U16, 0, 0,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex],
			8, 160);

		/*	wait TC	*/
		TFT2_voidWaitCurrentDataTransfer((TFT2_t*)&Global_LCD);

		/*	print image buffer on TFT	*/
		TFT2_voidSendPixels(
			(TFT2_t*)&Global_LCD,
			(u16*)Global_ImgBufferArr[Global_NotInUseImgBufferIndex], 160 * 8);

		/*	wait for new un-read IR data	*/
		while(Global_IsIrNotRead == false);
		Global_IsIrNotRead = false;

		/*	if enter was pressed, return from function immediately	*/
		if (Global_IrData == 17)
			return;

		/*	timestamp reception	*/
		u64 currentIrTimeStamp = STK_u64GetElapsedTicks();

		/*
		 * some IR remote controllers repeat the data when button is kept hold
		 * down. At some of these, this was found to be too fast, which may
		 * lead to multiple virtual inputs from one single press.
		 * Hence, a minimum interval between presses is introduced.
		 */
		if(
				currentIrTimeStamp - prevIrTimeStamp <
				IR_MIN_TIME_BETWEEN_PRESSES_MS * STK_TICKS_PER_MS
		)
			continue;

		/*
		 * If the small time threshold have passed, or current pressed button is
		 * not same as the previous one, increment "firstEmptyCharIndex". But!
		 * Only on a condition that current "firstEmptyCharIndex" is the
		 * terminator. To avoid non-real string end.
		 */
		else if(
				(
					currentIrTimeStamp - prevIrTimeStamp >
					IR_MAX_SMALL_TIME_MS * STK_TICKS_PER_MS ||
					prevIrData != Global_IrData
				)													&&

				str[firstEmptyCharIndex] != '\0'
		)
			firstEmptyCharIndex++;

		/*
		 * if new press value is same as the previous one, and time since
		 * previous reception is small (based on a configured threshold),
		 * change last char in "str" to the next char of the pressed button in
		 * "keyPadToAlphabet".
		 * Update: ignore back button (backspace, i.e.: integer 10).
		 */
		if (
			prevIrData == Global_IrData 				&&
			currentIrTimeStamp - prevIrTimeStamp <=
			IR_MAX_SMALL_TIME_MS * STK_TICKS_PER_MS		&&
			Global_IrData != 16
		)
		{
			str[firstEmptyCharIndex] =
				keyPadToAlphabet[Global_IrData][alphaIndex];

			/*	increment (circularly) the variable: "alphaIndex"	*/
			alphaIndex++;
			if (alphaIndex == strlen(keyPadToAlphabet[Global_IrData]))
				alphaIndex = 0;
		}

		/*
		 * Otherwise, write first char of the pressed button in
		 * "keyPadToAlphabet", increment "firstEmptyCharIndex". And finally,
		 * reset "alphaIndex".
		 */
		else
		{
			/*
			 * if backspace, replace last char with '\0', and decrement
			 * "firstEmptyCharIndex".
			 */
			if (Global_IrData == 16)
			{
				if (firstEmptyCharIndex != 0)
					firstEmptyCharIndex--;

				str[firstEmptyCharIndex] = '\0';
			}

			/*	other wise, add char	*/
			else
			{
				str[firstEmptyCharIndex] =
					keyPadToAlphabet[Global_IrData][0];

				/*	don't forget to shift the terminator	*/
				str[firstEmptyCharIndex + 1] = '\0';
			}

			alphaIndex = 0;
		}

		/*	update "prev"s	*/
		prevIrData = Global_IrData;
		prevIrTimeStamp = currentIrTimeStamp;
	}
}
















