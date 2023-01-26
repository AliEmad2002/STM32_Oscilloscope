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
#include "Oscilloscope_init_Global.h"

/*******************************************************************************
 * Extern global variables (from private.c file):
 ******************************************************************************/
extern b8 Global_LCDIsUnderUsage;
extern b8 Global_IsPixArrReady;;
extern u8 Global_PeakToPeakValueInCurrentFrame;
extern u8 Global_LargestVlaueInCurrentFrame;
extern u8 Global_SmallestVlaueInCurrentFrame;
extern u8 Global_NumberOfsentQuartersSinceLastInfoUpdate;
extern b8 Global_Enter;
extern OSC_RunningState_t Global_RunningState;
extern b8 Global_Paused;

/*******************************************************************************
 * Function declaration:
 ******************************************************************************/
void OSC_voidInitGlobal(void)
{
	Global_LCDIsUnderUsage = false;

	Global_PeakToPeakValueInCurrentFrame = 0;
	Global_LargestVlaueInCurrentFrame = 0;
	Global_SmallestVlaueInCurrentFrame = 0;

	Global_NumberOfsentQuartersSinceLastInfoUpdate = 0;

	Global_Enter = false;

	Global_IsPixArrReady = false;

	Global_RunningState = OSC_RunningState_Preparing1stQuarter;

	Global_Paused = false;
}









