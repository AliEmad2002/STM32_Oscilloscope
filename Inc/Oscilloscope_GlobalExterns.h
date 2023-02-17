/*
 * Oscilloscope_GlobalExterns.h
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_OSCILLOSCOPE_GLOBALEXTERNS_H_
#define INCLUDE_OSCILLOSCOPE_GLOBALEXTERNS_H_


extern volatile TFT2_t Global_LCD;
extern volatile u16 Global_PixArr[2 * LINES_PER_IMAGE_BUFFER * SIGNAL_LINE_LENGTH];
extern volatile u16* Global_ImgBufferArr[2];
extern volatile u8 Global_Ch1PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch1MinValueInCurrentFrame;
extern volatile u8 Global_Ch1MaxValueInCurrentFrame;
extern volatile u32 Global_Ch1SumOfCurrentFrame;
extern volatile u8 Global_Ch2PeakToPeakValueInCurrentFrame;
extern volatile u8 Global_Ch2MinValueInCurrentFrame;
extern volatile u8 Global_Ch2MaxValueInCurrentFrame;
extern volatile u32 Global_Ch2SumOfCurrentFrame;
extern volatile u32 Global_CurrentCh1MicroVoltsPerPix;
extern volatile u32 Global_CurrentCh2MicroVoltsPerPix;
extern volatile u64 Global_CurrentNanoSecondsPerPix;
extern volatile b8 Global_Paused;
extern volatile u64 Global_LastMeasuredFreq;
extern volatile OSC_Up_Down_Target_t Global_UpDownTarget;
extern volatile b8 Global_IsMenuOpen;
extern volatile u16 Global_SampleBuffer[2 * NUMBER_OF_SAMPLES];
extern volatile char Global_Str[128];
extern volatile b8 Global_IsCh1Enabled;
extern volatile b8 Global_IsCh2Enabled;
extern volatile u8 Global_LastRead1;
extern volatile u8 Global_LastRead2;
extern volatile u8 Global_Smaller1;
extern volatile u8 Global_Larger1;
extern volatile u8 Global_Smaller2;
extern volatile u8 Global_Larger2;
extern volatile s32 Global_Offset1MicroVolts;
extern volatile s32 Global_Offset2MicroVolts;
extern volatile b8 Global_Ch1LastReadWasInRange;
extern volatile b8 Global_Ch2LastReadWasInRange;
extern volatile OSC_RunningMode_t Global_CurrentRunningMode;
extern volatile u8 Global_NotInUseImgBufferIndex;
extern volatile IR_Receiver_t Global_IrReceiver;
extern volatile b8 Global_IsIrNotRead;
extern volatile u32 Global_IrData;

extern volatile MathParser_t Global_xMathParser;
extern volatile MathParser_t Global_yMathParser;

extern volatile OSC_Cursor_t Cursor_v1;
extern volatile OSC_Cursor_t Cursor_v2;
extern volatile OSC_Cursor_t Cursor_t1;
extern volatile OSC_Cursor_t Cursor_t2;


#endif /* INCLUDE_OSCILLOSCOPE_GLOBALEXTERNS_H_ */
