/*
 * Oscilloscope_program.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
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
#include "Oscilloscope_init_MCAL.h"
#include "Oscilloscope_init_HAL.h"
#include "Oscilloscope_init_Global.h"
#include "Oscilloscope_Menu_Interface.h"
#include "Oscilloscope_Conversions.h"
#include "Oscilloscope_ISRs.h"
#include "Oscilloscope_Sampling.h"
#include "Oscilloscope_Draw.h"
#include "Oscilloscope_Info.h"
#include "Oscilloscope_Normal.h"
#include "Oscilloscope_Math.h"
#include "Oscilloscope_interface.h"


void OSC_voidMainSuperLoop(void)
{
	/*	last time info was drawn. (timestamp)	*/
	u64 lastInfoDrawTime = 0;

	/*	periodic time to draw info (in STK ticks)	*/
	volatile u64 infoDrawPeriod =
		INFO_DRAWING_PERIODIC_TIME_MS * STK_TICKS_PER_MS;

	u64 lastFrameTimeStamp = 0;

	while (1)
	{
		/*	wait for frame time to come (according to configured FPS)	*/
		WAIT_NEXT_FRAME_AND_UPDATE_TIMESTAMP(lastFrameTimeStamp);

		/*
		 * if user opened menu, enter it.
		 * (menu internally clears flag and resets display boundaries on exit)
		 */
		if (Global_IsMenuOpen)
		{
			OSC_voidOpenMainMenu();
		}

		/*	only if in normal mode	*/
		if (Global_CurrentRunningMode == OSC_RunningMode_Normal)
		{
			/*	if info drawing time has passed, draw info	*/
			if (STK_u64GetElapsedTicks() - lastInfoDrawTime > infoDrawPeriod)
			{
				/*	draw info on screen	*/
				OSC_voidDrawInfo();
				/*	update timestamp	*/
				lastInfoDrawTime = STK_u64GetElapsedTicks();
			}
		}

		/*	only if device is not paused, take new samples	*/
		if (!Global_Paused)
		{
			OSC_voidTakeNewSamples();
		}

		/*	draw a frame based on the current running mode	*/
		switch(Global_CurrentRunningMode)
		{
		case OSC_RunningMode_Normal:
			OSC_voidDrawNormalModeFrame();
			break;

		case OSC_RunningMode_Math:
			OSC_voidDrawXYModeFrame();
			break;
		}
	}
}

void OSC_voidSelectNormalMode(void)
{
	Global_CurrentRunningMode = OSC_RunningMode_Normal;
}

void OSC_voidSelectMathMode(void)
{
	Global_CurrentRunningMode = OSC_RunningMode_Math;
}
