/*
 * Oscilloscope_interface.h
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
 */

#ifndef OSCILLOSCOPE_INTERFACE_H_
#define OSCILLOSCOPE_INTERFACE_H_


/*******************************************************************************
 * Init functions:
 ******************************************************************************/
/*
 * Inits all (MCAL) hardware resources configured in "Oscilloscope_configh.h"
 * file.
 */
extern void OSC_voidInitMCAL(void);

/*
 * Inits all (HAL) hardware resources configured in "Oscilloscope_configh.h"
 * file, and static objects defined in "Oscilloscope_program.c".
 */
extern void OSC_voidInitHAL(void);

extern void OSC_voidInitApp(void);

/*******************************************************************************
 * Mode switching:
 ******************************************************************************/
/*	enters menu mode	*/
void OSC_voidEnterMenuMode(void);

/*	Enters math mode (beta)	*/
void OSC_voidEnterMathMode(void);

/*	pause / resume (state is actually triggered each call)	*/
void OSC_voidTrigPauseResume(void);

/*******************************************************************************
 * Main thrad functions:
 ******************************************************************************/
/*	main super loop (no OS version)	*/
void OSC_voidMainSuperLoop(void);

/*	prepares info image that is to be displayed on the info section	*/
void OSC_voidPrepareInfoPixArray(void);

/*
 * auto calibrates volts per div and time per div to properly
 * display the signal
 */
void OSC_voidAutoCalibVoltAndTimePerDiv(void);

/*******************************************************************************
 * ISR callbacks:
 ******************************************************************************/
/*
 * this function is called whenever DMA finishes a transfer to TFT, it starts
 * the next "OSC_LineDrawingState_t" operation, and clears DMA completion flags.
 *
 */
void OSC_voidDMATransferCompleteCallback(void);

/*
 * this function is called to start executing what's mentioned in
 * "OSC_LineDrawingState_1" description.
 *
 * It is executed periodically as what user had configured time axis.
 *
 * minimum call rate of this function is important, as executing it in a high
 * rate would overlap drawing of multiple lines!
 * minimum call rate when F_SYS = 72MHz and using default TFT settings is about
 * 90~100 us.
 */
void OSC_voidTimToStartDrawingNextLineCallback(void);

/*
 * This function is periodically called/triggered by a configured timer unit.
 * It draws signal info on the screen.
 * This function:
 *  - Prepares info image.
 * 	- Pulls TFT semaphore till released.
 * 	- Takes it.
 *	- Pauses DMA transfer complete interrupt.
 *	- Sets info image boundaries on TFT.
 *	- Sends info image by DMA.
 *	- Waits for transfer complete.	  --|    all three are implemented in:
 *	- Clears DMA transfer complete flag.|==>"TFT2_voidWaitCurrentDataTransfer()"
 *	- Disables DMA.                   --|
 *	- Enables/resumes DMA transfer complete interrupt.
 *	- Releases TFT semaphore.
 *
 */
void OSC_voidTimToStartDrawingInfoCallback(void);

#endif /* OSCILLOSCOPE_INTERFACE_H_ */
