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
void LA_voidInitMCAL(void);

/*
 * Inits all (HAL) hardware resources configured in "Loginc_Analyzer_configh.h"
 * file, and static objects defined in "Loginc_Analyzer_program.c".
 */
void LA_voidInitHAL(void);


#endif /* LOGINC_ANALYZER_INTERFACE_H_ */
