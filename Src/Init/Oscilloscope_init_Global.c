/*
 * Oscilloscope_init_Global.c
 *
 *  Created on: Jan 26, 2023
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
#include "Oscilloscope_init_Global.h"

/*******************************************************************************
 * Extern global variables (from private.c file):
 ******************************************************************************/
extern u8 Global_PeakToPeakValueInCurrentFrame;
extern u8 Global_LargestVlaueInCurrentFrame;
extern u8 Global_SmallestVlaueInCurrentFrame;
extern b8 Global_Paused;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_IsMenuOpen;

extern volatile OSC_Cursor_t Cursor_v1;
extern volatile OSC_Cursor_t Cursor_v2;
extern volatile OSC_Cursor_t Cursor_t1;
extern volatile OSC_Cursor_t Cursor_t2;

/*******************************************************************************
 * Function declaration:
 ******************************************************************************/
void OSC_voidInitGlobal(void)
{
	Global_PeakToPeakValueInCurrentFrame = 0;
	Global_LargestVlaueInCurrentFrame = 0;
	Global_SmallestVlaueInCurrentFrame = 0;

	Global_Paused = false;

	Global_UpDownTarget = OSC_Up_Down_Target_ChangeVoltageDiv;
	//Global_UpDownTarget = OSC_Up_Down_Target_ChangeTimeCursor2Position;

	Global_IsMenuOpen = false;

	Cursor_v1 = (OSC_Cursor_t){50, true};
	Cursor_v2 = (OSC_Cursor_t){0, false};
	Cursor_t1 = (OSC_Cursor_t){0, false};
	Cursor_t2 = (OSC_Cursor_t){11, true};
}









