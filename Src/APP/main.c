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
#include "Oscilloscope_interface.h"

int main(void)
{
	OSC_voidInitMCAL();
	//Delay_voidBlockingDelayMs(10000);
	OSC_voidInitHAL();
	OSC_voidMainSuperLoop();

	while(1)
	{

	}

	return 0;
}

