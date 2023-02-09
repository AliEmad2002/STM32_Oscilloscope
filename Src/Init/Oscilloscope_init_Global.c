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
extern volatile u8 Global_Ch1PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch1MinValueInCurrentFrame;
extern volatile u8 Global_Ch1MaxValueInCurrentFrame;
extern volatile u8 Global_Ch2PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch2MinValueInCurrentFrame;
extern volatile u8 Global_Ch2MaxValueInCurrentFrame;
extern volatile b8 Global_Paused;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_IsMenuOpen;
extern volatile b8 Global_IsCh1Enabled;
extern volatile b8 Global_IsCh2Enabled;
extern volatile u8 Global_LastRead1;
extern volatile u8 Global_LastRead2;
extern volatile u8 Global_Smaller1;
extern volatile u8 Global_Larger1;
extern volatile u8 Global_Smaller2;
extern volatile u8 Global_Larger2;
extern volatile s32 Global_Offset1MicroVolts;
extern volatile s32 Global_Offset2MicroVolts;
extern volatile OSC_RunningMode_t Global_CurrentRunningMode;
extern volatile b8 Global_Ch1LastReadWasInRange;
extern volatile b8 Global_Ch2LastReadWasInRange;
extern volatile u8 Global_NotInUseImgBufferIndex;

extern volatile OSC_Cursor_t Cursor_v1;
extern volatile OSC_Cursor_t Cursor_v2;
extern volatile OSC_Cursor_t Cursor_t1;
extern volatile OSC_Cursor_t Cursor_t2;

/*******************************************************************************
 * Function declaration:
 ******************************************************************************/
void OSC_voidInitGlobal(void)
{
	Global_Ch1PeakToPeakValueInCurrentFrame = 0;
	Global_Ch1MinValueInCurrentFrame = 0;
	Global_Ch1MaxValueInCurrentFrame = 0;
	Global_Ch2PeakToPeakValueInCurrentFrame = 0;
	Global_Ch2MinValueInCurrentFrame = 0;
	Global_Ch2MaxValueInCurrentFrame = 0;

	Global_Paused = false;

	//Global_UpDownTarget = OSC_Up_Down_Target_ChangeVoltageDiv;
	//Global_UpDownTarget = OSC_Up_Down_Target_ChangeTimeCursor2Position;
	Global_UpDownTarget = OSC_Up_Down_Target_ChangeTimeDiv;

	Global_IsMenuOpen = false;

	Global_IsCh1Enabled = true;
	Global_IsCh2Enabled = true;

	Global_LastRead1 = 0;
	Global_LastRead2 = 0;
	Global_Smaller1 = 0;
	Global_Larger1 = 0;
	Global_Smaller2 = 0;
	Global_Larger2 = 0;
	Global_Offset1MicroVolts = 0;
	Global_Offset2MicroVolts = 0;

	Global_Ch1LastReadWasInRange = false;
	Global_Ch2LastReadWasInRange = false;

	Global_NotInUseImgBufferIndex = 0;

	Global_CurrentRunningMode = OSC_RunningMode_Normal;

	Cursor_v1 = (OSC_Cursor_t){0, false};
	Cursor_v2 = (OSC_Cursor_t){0, false};
	Cursor_t1 = (OSC_Cursor_t){0, false};
	Cursor_t2 = (OSC_Cursor_t){0, false};
}









