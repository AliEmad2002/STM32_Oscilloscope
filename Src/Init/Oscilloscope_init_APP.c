/*
 * Oscilloscope_init_APP.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "Img_interface.h"
#include "Colors.h"
#include "Delay_interface.h"
#include "Menu_interface.h"
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
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_config.h"
#include "Oscilloscope_Private.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_SavedConfig.h"
#include "Oscilloscope_Info_Init.h"
#include "Oscilloscope_Menu_Interface.h"
#include "Oscilloscope_Menu_Init.h"
#include "Oscilloscope_AutoCalib.h"
#include "Oscilloscope_init_APP.h"

void OSC_voidInitAPP(void)
{
	/*	init UI	*/
	OSC_voidInitInfoArr();

	OSC_voidInitMainMenu();

	/*	copy saved configuration from flash to RAM	*/
	OSC_voidGetSavedConfigFromFlash();

	/*
	 * as calling "OSC_voidAutoCalibrate()" at the first few microseconds of
	 * the MCU program returns without setting any thing, wait before calling.
	 */
	Delay_voidBlockingDelayMs(500);

	/*	Auto calibrate voltage per div, voltage gain and time per div	*/
	OSC_voidAutoCalibrate();

	/*	clear display (fill with black color)	*/
	TFT2_voidClearDisplay((TFT2_t*)&Global_LCD);
}

































