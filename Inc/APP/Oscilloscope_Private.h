/*
 * Oscilloscope_Private.h
 *
 *  Created on: Jan 18, 2023
 *      Author: Ali Emad Ali
 *
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_
#define INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_

/*	enum that describes the different states of line drawing	*/
typedef enum{
	OSC_LineDrawingState_1,	// means that the last started operation is:
							// scroll, read ADC, set new drawing boundaries
							// and draw black segment from 0 to
							// 'OSC_smallest' - 1.

	OSC_LineDrawingState_2,	// means that the last started operation is:
							// Drawing red segment from 'OSC_smallest' to
							// 'OSC_largeest'.

	OSC_LineDrawingState_3,	// means that the last started operation is:
							// Drawing red segment from 'OSC_largeest' + 1 to
							// 127.
}OSC_LineDrawingState_t;

/*	static objects	*/
static TFT2_t LCD;

static u8 OSC_smallest = 0;		// these two variables represent smallest and
static u8 OSC_largest = 0;		// largest points in the current line's active
								// (red) segment.

static OSC_LineDrawingState_t drawingState = OSC_LineDrawingState_3;

static u8 tftScrollCounter = 0;

/*	binary semaphore for TFT interfacing line	*/
static b8 tftIsUnderUsage = false;

static NVIC_Interrupt_t tftDmaInterruptNumber = 0;

/*
 * Peak to peak value in a single frame.
 * Is calculated as the difference between the largest and smallest values in
 * current frame.
 * "current frame" starts when "tftScrollCounter" equals zero, and ends when it
 * equals "tftScrollCounterMax".
 * Unit is: [TFT screen pixels].
 */
static u8 peakToPeakValueInCurrentFrame = 0;
static u8 largestVlaueInCurrentFrame = 0;
static u8 smallestVlaueInCurrentFrame = 0;

static NVIC_Interrupt_t timTrigLineDrawingInterrupt = 0;

static u16 infoPixArr[30][128] = {0};
static b8 isInfoPixArrPrepared = false;

/*	Defines based on configuration file	*/
#define ADC_1_CHANNEL		(ANALOG_INPUT_1_PIN % 16)
#define ADC_2_CHANNEL		(ANALOG_INPUT_2_PIN % 16)

/*	constant values based on experimental tests	*/
const u64 lineDrawingRatemHzMax = 10000000;


#endif /* INCLUDE_APP_OSCILLOSCOPE_PRIVATE_H_ */
