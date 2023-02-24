/*
 * Oscilloscope_Sampling.h
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_SAMPLING_H_
#define INCLUDE_APP_OSCILLOSCOPE_SAMPLING_H_

u8 OSC_u8GetTimSyncTimerUnit(void);

void OSC_voidWaitForSignalRisingEdge(void);

void OSC_voidPreFillSampleBuffer(void);

void OSC_voidStartFillingSampleBuffer(void);

void OSC_voidWaitSampleBufferFill(void);

/*
 * when t_pix  < t_conv, interpolate / predict samples in between real samples.
 */
void OSC_voidInterpolate(void);

void OSC_voidTakeNewSamples(void);



#endif /* INCLUDE_APP_OSCILLOSCOPE_SAMPLING_H_ */
