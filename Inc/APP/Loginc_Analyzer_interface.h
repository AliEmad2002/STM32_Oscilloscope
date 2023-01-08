/*
 * Loginc_Analyzer_interface.h
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
 */

#ifndef LOGINC_ANALYZER_INTERFACE_H_
#define LOGINC_ANALYZER_INTERFACE_H_


/*
 * Inits all (MCAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file.
 */
void OSC_voidInitMCAL(void);

/*
 * Inits all (HAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file, and static objects defined in "Loginc_Analyzer_program.c".
 */
void OSC_voidInitHAL(void);

///*
// * runs main super loop (no OS version)
// */
void OSC_voidRunMainSuperLoop(void);


#endif /* LOGINC_ANALYZER_INTERFACE_H_ */
