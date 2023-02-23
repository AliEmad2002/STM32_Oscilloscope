/*
 * Oscilloscope_SavedConfig.c
 *
 *  Created on: Feb 23, 2023
 *      Author: ali20
 */

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include "My_Math.h"
#include "LinkedList.h"
#include "MathParser.h"

/*	MCAL	*/
#include "RCC_interface.h"
#include "STK_interface.h"
#include "ADC_interface.h"
#include "TIM_interface.h"
#include "GPIO_interface.h"
#include "DMA_interface.h"
#include "SPI_interface.h"
#include "FPEC_interface.h"

/*	HAL	*/
#include "TFT_interface_V2.h"
#include "IR_interface.h"

/*	SELF	*/
#include "Oscilloscope_Private.h"
#include "Oscilloscope_config.h"
#include "Oscilloscope_Cursor.h"
#include "Oscilloscope_GlobalExterns.h"
#include "Oscilloscope_Info.h"
#include "Oscilloscope_SavedConfig.h"

/*******************************************************************************
 * Addresses of saved data in flash
 ******************************************************************************/
#define CH1_MICRO_VOLTS_PER_PIX_F_ADDRESS			(FPEC_WORD_ADDRESS(62, 0))
#define CH2_MICRO_VOLTS_PER_PIX_F_ADDRESS			(FPEC_WORD_ADDRESS(62, 1))

#define L_NANO_SECONDS_PER_PIX_F_ADDRESS			(FPEC_WORD_ADDRESS(62, 2))
#define H_NANO_SECONDS_PER_PIX_F_ADDRESS			(FPEC_WORD_ADDRESS(62, 3))

#define UP_DOWN_TARGET_F_ADDRESS					(FPEC_WORD_ADDRESS(62, 4))

#define IS_CH1_ENABLED_F_ADDRESS					(FPEC_WORD_ADDRESS(62, 5))
#define IS_CH2_ENABLED_F_ADDRESS					(FPEC_WORD_ADDRESS(62, 6))

#define OFFSET1_MICRO_VOLTS_F_ADDRESS				(FPEC_WORD_ADDRESS(62, 7))
#define OFFSET2_MICRO_VOLTS_F_ADDRESS				(FPEC_WORD_ADDRESS(62, 8))

/*	Four indexes of the four first enabled info's are stored in this word	*/
#define SELECTED_INFO_ARR_F_ADDRESS						FPEC_WORD_ADDRESS(62, 9)

void OSC_voidGetSavedConfigFromFlash(void)
{
	/*	unlock flash memory	*/
	FPEC_voidUnlock();

	/*	copy configuration from flash to RAM	*/
	Global_CurrentCh1MicroVoltsPerPix =
		FPEC_u32ReadWord(CH1_MICRO_VOLTS_PER_PIX_F_ADDRESS);

	Global_CurrentCh2MicroVoltsPerPix =
		FPEC_u32ReadWord(CH2_MICRO_VOLTS_PER_PIX_F_ADDRESS);


	u64 currentNanoSecondsPerPixLow =
		FPEC_u32ReadWord(L_NANO_SECONDS_PER_PIX_F_ADDRESS);

	u64 currentNanoSecondsPerPixHigh =
		FPEC_u32ReadWord(H_NANO_SECONDS_PER_PIX_F_ADDRESS);

	Global_CurrentNanoSecondsPerPix =
		currentNanoSecondsPerPixLow | (currentNanoSecondsPerPixHigh << 32);


	Global_UpDownTarget = FPEC_u32ReadWord(UP_DOWN_TARGET_F_ADDRESS);


	Global_IsCh1Enabled = FPEC_u32ReadWord(IS_CH1_ENABLED_F_ADDRESS);

	Global_IsCh2Enabled = FPEC_u32ReadWord(IS_CH2_ENABLED_F_ADDRESS);


	Global_Offset1MicroVolts = FPEC_u32ReadWord(OFFSET1_MICRO_VOLTS_F_ADDRESS);

	Global_Offset2MicroVolts = FPEC_u32ReadWord(OFFSET2_MICRO_VOLTS_F_ADDRESS);


	u32 selectedInfoArrCompressed =
		FPEC_u32ReadWord(SELECTED_INFO_ARR_F_ADDRESS);

	u8 selectedInfo0 = selectedInfoArrCompressed & 0xFF;
	u8 selectedInfo1 = selectedInfoArrCompressed & (0xFF << 8);
	u8 selectedInfo2 = selectedInfoArrCompressed & (0xFF << 16);
	u8 selectedInfo3 = selectedInfoArrCompressed & (0xFF << 24);

	OSC_voidDisableAllInfo();

	OSC_voidEnableInfo(selectedInfo0);
	OSC_voidEnableInfo(selectedInfo1);
	OSC_voidEnableInfo(selectedInfo2);
	OSC_voidEnableInfo(selectedInfo3);

	/*	lock flash memory	*/
	FPEC_voidLock();
}

void OSC_voidWriteCurrentConfigOnFlash(void)
{
	/*	unlock flash memory	*/
	FPEC_voidUnlock();

	/*	erase configuration page in flash	*/
	FPEC_voidEnablePageEraseMode();

	FPEC_voidErasePage(62);

	FPEC_voidDisablePageEraseMode();

	/*	copy configuration from RAM to flash	*/
	FPEC_voidEnableProgrammingMode();

	FPEC_voidProgramWord(
		CH1_MICRO_VOLTS_PER_PIX_F_ADDRESS, Global_CurrentCh1MicroVoltsPerPix);

	FPEC_voidProgramWord(
		CH2_MICRO_VOLTS_PER_PIX_F_ADDRESS, Global_CurrentCh2MicroVoltsPerPix);


	FPEC_voidProgramWord(
		L_NANO_SECONDS_PER_PIX_F_ADDRESS,
		Global_CurrentNanoSecondsPerPix & 0xFFFFFFFF);

	FPEC_voidProgramWord(
		H_NANO_SECONDS_PER_PIX_F_ADDRESS,
		Global_CurrentNanoSecondsPerPix >> 32);


	FPEC_voidProgramWord(UP_DOWN_TARGET_F_ADDRESS, Global_UpDownTarget);


	FPEC_voidProgramWord(IS_CH1_ENABLED_F_ADDRESS, Global_IsCh1Enabled);

	FPEC_voidProgramWord(IS_CH2_ENABLED_F_ADDRESS, Global_IsCh2Enabled);


	FPEC_voidProgramWord(OFFSET1_MICRO_VOLTS_F_ADDRESS, Global_Offset1MicroVolts);

	FPEC_voidProgramWord(OFFSET2_MICRO_VOLTS_F_ADDRESS, Global_Offset2MicroVolts);


	FPEC_voidProgramWord(SELECTED_INFO_ARR_F_ADDRESS, OSC_u32GetEnabledInfo());


	FPEC_voidDisableProgrammingMode();

	/*	lock flash memory	*/
	FPEC_voidLock();
}






