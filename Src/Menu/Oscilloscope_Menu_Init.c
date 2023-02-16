/*
 * Oscilloscope_Menu_Init.c
 *
 *  Created on: Feb 11, 2023
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
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Info.h"
#include "Oscilloscope_Info_init.h"
#include "Oscilloscope_Math.h"
#include "Oscilloscope_Menu_Private.h"
#include "Oscilloscope_Menu_Init.h"

extern volatile Menu_t Global_MainMenu;
extern volatile OSC_Info_t Global_InfoArr[NUMBER_OF_INFO];

extern void OSC_voidSelectNormalMode(void);
extern void OSC_voidSelectMathMode(void);

Menu_t* OSC_PtrInitChangeVoltageDivMenu(void)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[2];

	/**	define elements	**/
	static const char str0[] = " Ch1";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_Callback;
	elementArr[0].childPtr = OSC_voidSelectChangeCh1VoltageDivAsUpDownTraget;

	static const char str1[] = " Ch2";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_Callback;
	elementArr[1].childPtr = OSC_voidSelectChangeCh2VoltageDivAsUpDownTraget;

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 2,
		.elementArr = elementArr
	};

	return &menu;
}

Menu_t* OSC_PtrInitChangeDivMenu(void)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[2];

	/**	define elements	**/
	static const char str0[] = " Voltage";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_SubMenu;
	elementArr[0].childPtr = OSC_PtrInitChangeVoltageDivMenu();

	static const char str1[] = " Time";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_Callback;
	elementArr[1].childPtr = OSC_voidSelectChangeTimeDivAsUpDownTraget;

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 2,
		.elementArr = elementArr
	};

	return &menu;
}

Menu_t* OSC_PtrInitOffsetMenu(void)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[2];

	/**	define elements	**/
	static const char str0[] = " Ch1";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_Callback;
	elementArr[0].childPtr = OSC_voidSelectChangeCh1Offset;

	static const char str1[] = " Ch2";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_Callback;
	elementArr[1].childPtr = OSC_voidSelectChangeCh2Offset;

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 2,
		.elementArr = elementArr
	};

	return &menu;
}

Menu_t* OSC_PtrInitCusrorSelectMenu(void)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[4];

	/**	define elements	**/
	static const char str0[] = " Select v1";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_Callback;
	elementArr[0].childPtr = OSC_voidSelectChangeV1Position;

	static const char str1[] = " Select v2";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_Callback;
	elementArr[1].childPtr = OSC_voidSelectChangeV2Position;

	static const char str2[] = " Select t1";
	elementArr[2].str = (char*)str2;
	elementArr[2].type = Menu_ElementType_Callback;
	elementArr[2].childPtr = OSC_voidSelectChangeT1Position;

	static const char str3[] = " Select t2";
	elementArr[3].str = (char*)str3;
	elementArr[3].type = Menu_ElementType_Callback;
	elementArr[3].childPtr = OSC_voidSelectChangeT2Position;

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 4,
		.elementArr = elementArr
	};

	return &menu;
}

Check_List_t* OSC_PtrInitCursorEnableDisableCkeckList(char* returnStr)
{
	/**	Define elementArr	**/
	static Check_List_Element_t elementArr[5];

	static const char str0[] = "v1";
	elementArr[0].str = (char*)str0;
	elementArr[0].isChecked = (b8*)&(Cursor_v1.isEnabled);
	elementArr[0].isHidden = false;

	static const char str1[] = "v2";
	elementArr[1].str = (char*)str1;
	elementArr[1].isChecked = (b8*)&(Cursor_v2.isEnabled);
	elementArr[1].isHidden = false;

	static const char str2[] = "t1";
	elementArr[2].str = (char*)str2;
	elementArr[2].isChecked = (b8*)&(Cursor_t1.isEnabled);
	elementArr[2].isHidden = false;

	static const char str3[] = "t2";
	elementArr[3].str = (char*)str3;
	elementArr[3].isChecked = (b8*)&(Cursor_t2.isEnabled);
	elementArr[3].isHidden = false;

	elementArr[4].str = returnStr;
	elementArr[4].isHidden = false;

	/**	Define check-list	**/
	static Check_List_t checkList;
	checkList.currentSelectedElement = 0;
	checkList.elementArr = elementArr;
	checkList.numberOfElements = 5;

	return &checkList;
}

Menu_t* OSC_PtrInitCursorMenu(char* returnStr)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[2];

	/**	define elements	**/
	static const char str0[] = " Enable/Disable Cursor";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_CheckList;
	elementArr[0].childPtr = OSC_PtrInitCursorEnableDisableCkeckList(returnStr);

	static const char str1[] = " Select Cursor";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_SubMenu;
	elementArr[1].childPtr = OSC_PtrInitCusrorSelectMenu();

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 2,
		.elementArr = elementArr
	};

	return &menu;
}

Check_List_t* OSC_PtrInitChannelEnableDisableCkeckList(char* returnStr)
{
	/**	Define elementArr	**/
	static Check_List_Element_t elementArr[3];

	static const char str0[] = "Ch1";
	elementArr[0].str = (char*)str0;
	elementArr[0].isChecked = (b8*)&Global_IsCh1Enabled;
	elementArr[0].isHidden = false;

	static const char str1[] = "Ch2";
	elementArr[1].str = (char*)str1;
	elementArr[1].isChecked = (b8*)&Global_IsCh2Enabled;
	elementArr[1].isHidden = false;

	elementArr[2].str = returnStr;
	elementArr[2].isHidden = false;

	/**	Define check-list	**/
	static Check_List_t checkList;
	checkList.currentSelectedElement = 0;
	checkList.elementArr = elementArr;
	checkList.numberOfElements = 3;

	return &checkList;
}

Check_List_t* OSC_PtrInitInfoEnableDisableCkeckList(char* returnStr)
{
	static Check_List_Element_t elementArr[NUMBER_OF_INFO + 1];

	static Check_List_t checkList;

	/*	link "infoArr" to "elementArr"	*/
	for (u8 i = 0; i < NUMBER_OF_INFO; i++)
	{
		elementArr[i].str = Global_InfoArr[i].name;
		elementArr[i].isChecked = (b8*)&(Global_InfoArr[i].enabled);
		elementArr[i].isHidden = false;
	}

	/*	add "return" element	*/
	elementArr[NUMBER_OF_INFO].str = returnStr;
	elementArr[NUMBER_OF_INFO].isHidden = false;

	/*	link "elementArr" to "checkList"	*/
	checkList.elementArr = elementArr;
	checkList.numberOfElements = NUMBER_OF_INFO + 1;
	checkList.currentSelectedElement = 0;

	return &checkList;
}

Menu_t* OSC_PtrInitMathExpressionMenu(void)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[2];

	/**	define elements	**/
	static const char str0[] = " Edit X-axis expression";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_Callback;
	elementArr[0].childPtr = OSC_voidEditXAxisMathExpression;

	static const char str1[] = " Edit Y-axis expression";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_Callback;
	elementArr[1].childPtr = OSC_voidEditYAxisMathExpression;

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 2,
		.elementArr = elementArr
	};

	return &menu;
}

Menu_t* OSC_PtrInitChangeModeMenu(void)
{
	/**	define elementArr	**/
	static Menu_Element_t elementArr[2];

	/**	define elements	**/
	static const char str0[] = " Normal mode";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_Callback;
	elementArr[0].childPtr = OSC_voidSelectNormalMode;

	static const char str1[] = " Math mode";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_Callback;
	elementArr[1].childPtr = OSC_voidSelectMathMode;

	/**	define menu	**/
	static Menu_t menu = {
		.currentSelected = 0,
		.numberOfElements = 2,
		.elementArr = elementArr
	};

	return &menu;
}

void OSC_voidInitMainMenu(void)
{
	static const char returnStr[] = "Return";

	/**	define elementArr	**/
	static Menu_Element_t elementArr[8];

	/**	define elements	**/
	static const char str0[] = " Change division";
	elementArr[0].str = (char*)str0;
	elementArr[0].type = Menu_ElementType_SubMenu;
	elementArr[0].childPtr = OSC_PtrInitChangeDivMenu();

	static const char str1[] = " Cursor";
	elementArr[1].str = (char*)str1;
	elementArr[1].type = Menu_ElementType_SubMenu;
	elementArr[1].childPtr = OSC_PtrInitCursorMenu((char*)returnStr);

	static const char str2[] = " Enable/Disable Channel";
	elementArr[2].str = (char*)str2;
	elementArr[2].type = Menu_ElementType_CheckList;
	elementArr[2].childPtr =
		OSC_PtrInitChannelEnableDisableCkeckList((char*)returnStr);

	static const char str3[] = " Change offset";
	elementArr[3].str = (char*)str3;
	elementArr[3].type = Menu_ElementType_SubMenu;
	elementArr[3].childPtr = OSC_PtrInitOffsetMenu();

	static const char str4[] = " Enable/Disable Info";
	elementArr[4].str = (char*)str4;
	elementArr[4].type = Menu_ElementType_CheckList;
	elementArr[4].childPtr =
		OSC_PtrInitInfoEnableDisableCkeckList((char*)returnStr);

	static const char str5[] = " Select running mode";
	elementArr[5].str = (char*)str5;
	elementArr[5].type = Menu_ElementType_SubMenu;
	elementArr[5].childPtr = OSC_PtrInitChangeModeMenu();

	static const char str6[] = " Edit math expression";
	elementArr[6].str = (char*)str6;
	elementArr[6].type = Menu_ElementType_SubMenu;
	elementArr[6].childPtr = OSC_PtrInitMathExpressionMenu();

	static const char str7[] = " Change brightness";
	elementArr[7].str = (char*)str7;
	elementArr[7].type = Menu_ElementType_Callback;
	elementArr[7].childPtr = OSC_voidSelectChangeBrightness;

	/**	assign to main menu	**/
	Global_MainMenu.currentSelected = 0;
	Global_MainMenu.numberOfElements = 8;
	Global_MainMenu.elementArr = elementArr;
}

