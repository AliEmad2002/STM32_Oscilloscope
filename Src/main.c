/*
 * main.c
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

#include "Oscilloscope_Private.h"
#include "Oscilloscope_interface.h"


int main(void)
{
	RCC_voidSysClockInit();

	OSC_voidInitGlobal();

	OSC_voidInitMCAL();

	OSC_voidInitHAL();

	OSC_voidInitAPP();

	OSC_voidMainSuperLoop();

	while(1)
	{

	}

	return 0;
}

