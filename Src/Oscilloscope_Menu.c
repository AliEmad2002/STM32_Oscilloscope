///*
// * Oscilloscope_Menu.c
// *
// *  Created on: Jan 19, 2023
// *      Author: Ali Emad Ali
// */
//
///*	LIB	*/
//#include "Std_Types.h"
//#include "Bit_Math.h"
//#include "Menu_config.h"
//#include "Menu_interface.h"
//#include "Img_interface.h"
//#include "Colors.h"
//#include "Txt_interface.h"
//#include "diag/trace.h"
//
///*	MCAL	*/
//#include "RCC_interface.h"
//#include "SCB_interface.h"
//#include "NVIC_interface.h"
//#include "STK_interface.h"
//#include "DMA_interface.h"
//#include "SPI_interface.h"
//#include "TIM_interface.h"
//#include "GPIO_interface.h"
//#include "EXTI_interface.h"
//#include "ADC_interface.h"
//
///*	HAL	*/
//#include "TFT_interface_V2.h"
//
///*	SELF	*/
//#include "Oscilloscope_config.h"
//#include "Oscilloscope_Menu.h"
//
//extern void OSC_voidPause(void);
//extern void OSC_voidResume(void);
//
//extern TFT2_t OSC_LCD;
//extern b8 enter;
//
//void OSC_voidRefreshMenu(void)
//{
//	/**	fill the screen with background color	**/
//	/*	set boundaries (full screen)	*/
//	TFT2_SET_X_BOUNDARIES(&OSC_LCD, 0, 127);
//	TFT2_SET_Y_BOUNDARIES(&OSC_LCD, 0, 159);
//
//	/*	start data write operation	*/
//	TFT2_WRITE_CMD(&OSC_LCD, TFT_CMD_MEM_WRITE);
//	TFT2_ENTER_DATA_MODE(&OSC_LCD);
//
//	/*	send	*/
//	TFT2_voidFillDMA(&OSC_LCD, &LCD_BACKGROUND_COLOR_U8, 128 * 160);
//
//	/**	draw menu lines	**/
//	/*	pixel array of a single line	*/
//	u16 menuLinePixArr[8][128];
//
//	/*
//	 * loop on menu elements,
//	 */
//	for (u8 i = 0; i < NUMBER_OF_OSC_MENU_ELEMENTS; i++)
//	{
//		u16 bgColor, fontColor;
//
//		/*	if the selected menu element is to be drawn, invert colors	*/
//		if (i == mainMenu.currentSelected)
//		{
//			bgColor = LCD_MAIN_DRAWING_COLOR_U16;
//			fontColor = LCD_BACKGROUND_COLOR_U16;
//		}
//		/*	otherwise, draw menu element normally	*/
//		else
//		{
//			bgColor = LCD_BACKGROUND_COLOR_U16;
//			fontColor = LCD_MAIN_DRAWING_COLOR_U16;
//		}
//
//		/*	wait for previous transfer completion	*/
//		TFT2_voidWaitCurrentDataTransfer(&OSC_LCD);
//
//		// TODO: orientation!
//		Txt_voidCpyStrToStaticPixArrNormalOrientation(
//			mainMenu.elementArr[i].str, fontColor, bgColor, 1,
//			Txt_HorizontalMirroring_Disabled, Txt_VerticalMirroring_Disabled,
//			0, 0, 8, 128, menuLinePixArr);
//
//		/*	set boundaries 	*/
//		TFT2_SET_X_BOUNDARIES(&OSC_LCD, 0, 127);
//		TFT2_SET_Y_BOUNDARIES(&OSC_LCD, 8 * i, 8 * (i + 1));
//
//		/*	start data write operation	*/
//		TFT2_WRITE_CMD(&OSC_LCD, TFT_CMD_MEM_WRITE);
//		TFT2_ENTER_DATA_MODE(&OSC_LCD);
//
//		/*	send	*/
//		TFT2_voidSendPixels(&OSC_LCD, menuLinePixArr, 128 * 8);
//	}
//
//	/*	wait for last transfer completion	*/
//	TFT2_voidWaitCurrentDataTransfer(&OSC_LCD);
//}
//
//void OSC_voidOpenMenu(void)
//{
//	/*	pause before opening menu	*/
//	OSC_voidPause();
//
//	/*	reset selection	*/
//	mainMenu.currentSelected = 1;
//
//	/*	show menu on the screen	*/
//	OSC_voidRefreshMenu();
//
//	u8 lastSelected = mainMenu.currentSelected;
//
//	while(1)
//	{
//		/*
//		 * wait for a key press. either up, down, or pause / resume (enter)
//		 */
//		if (lastSelected != mainMenu.currentSelected)
//		{
//			OSC_voidRefreshMenu();
//			lastSelected = mainMenu.currentSelected;
//		}
//
//		if (enter == true)
//		{
//			enter = false;
//			mainMenu.elementArr[mainMenu.currentSelected].callback();
//			break;
//		}
//	}
//
//	/*	resume	*/
//	OSC_voidResume();
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
