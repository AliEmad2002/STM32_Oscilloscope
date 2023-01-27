/*
 * Oscilloscope_init_MCAL.h
 *
 *  Created on: Jan 18, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_INIT_MCAL_H_
#define INCLUDE_APP_OSCILLOSCOPE_INIT_MCAL_H_


void OSC_InitRCC(void);

void OSC_InitSysTick(void);

void OSC_InitGPIO(void);

void OSC_InitEXTI(void);

void OSC_InitADC(void);

void OSC_InitTIM(void);

void OSC_InitDMA(void);

void OSC_InitSCB(void);

void OSC_InitNVIC(void);

void OSC_voidInitMCAL(void);

#endif /* INCLUDE_APP_OSCILLOSCOPE_INIT_MCAL_H_ */
