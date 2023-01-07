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

#include "RCC_interface.h"
#include "TIM_interface.h"

/*	APP	*/
#include "Loginc_Analyzer_interface.h"



int main(void)
{
	OSC_voidInitMCAL();
	OSC_voidInitHAL();
	OSC_voidRunMainSuperLoop();

	while(1)
	{

	}

	return 0;
}

