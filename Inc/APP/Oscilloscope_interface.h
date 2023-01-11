/*
 * Oscilloscope_interface.h
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
 */

#ifndef OSCILLOSCOPE_INTERFACE_H_
#define OSCILLOSCOPE_INTERFACE_H_


/*
 * Inits all (MCAL) hardware resources configured in "Oscilloscope_configh.h"
 * file.
 */
void OSC_voidInitMCAL(void);

/*
 * Inits all (HAL) hardware resources configured in "Oscilloscope_configh.h"
 * file, and static objects defined in "Oscilloscope_program.c".
 */
void OSC_voidInitHAL(void);

/*	main super loop (no OS version)	*/
void OSC_voidMainSuperLoop(void);


#endif /* OSCILLOSCOPE_INTERFACE_H_ */
