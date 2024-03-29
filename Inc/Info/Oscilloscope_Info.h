/*
 * Oscilloscope_Info.h
 *
 *  Created on: Feb 8, 2023
 *      Author: Ali Emad Ali
 */

#ifndef APP_INFO_OSCILLOSCOPE_INFO_H_
#define APP_INFO_OSCILLOSCOPE_INFO_H_

#define NUMBER_OF_INFO	19

#define START_INDEX_OF_CH2_INFO		7
#define START_INDEX_OF_OTHER_INFO	(START_INDEX_OF_CH2_INFO * 2)

typedef struct{
	s64 (*getValInNanoCallback)(void);
	char* name;
	char* unit;
	b8 enabled;
}OSC_Info_t;

extern void OSC_voidSetDisplayBoundariesForSignalArea(void);

/**	Drawing functions	**/
/*
 * Draws enabled info on their selected places on the display.
 * (Draws first 4 info's only, as there's only 4 places available)
 */
void OSC_voidDrawInfo(void);

/*
 * given a value in nano, it get the suitable floating point number
 * (does no actually use float for less over head when there's no FPU),
 * and the suitable unit prefix.
 */
void OSC_voidGetNumberPrintableVersion(
	s64 valInNano, s32* valInteger, u32* valFraction, char* unitPrefix);

void OSC_voidGetInfoStringToPrint(char* str, u8 infoIndex);

void OSC_voidSSetTFTBoundariesToInfo(u8 infoIndex);

/**	Hide, show functions	**/
/*	Ch1	*/
void OSC_voidShowCh1Info(void);

/*	Ch2	*/
void OSC_voidShowCh2Info(void);

/**	Enable, disable functions	**/
/*	disables all enabled info's	*/
void OSC_voidDisableAllInfo(void);

/*	enables info of certain index in infoArr	*/
void OSC_voidEnableInfo(u8 i);

/*	disables info of certain index in infoArr	*/
void OSC_voidDisableInfo(u8 i);

/*
 * gets indexes of the first four enabled info's, grouped in 32-bit single
 * variable.
 */
u32 OSC_u32GetEnabledInfo(void);

/**	callbacks	**/
/*	freq1	*/
s64 OSC_s64GetFreq1Info(void);

/*	freq2	*/
s64 OSC_s64GetFreq2Info(void);

/*	vpp1	*/
s64 OSC_s64GetVpp1Info(void);

/*	vpp2	*/
s64 OSC_s64GetVpp2Info(void);

/*	vMin1	*/
s64 OSC_s64GetVmin1Info(void);

/*	vMin2	*/
s64 OSC_s64GetVmin2Info(void);

/*	vMax1	*/
s64 OSC_s64GetVmax1Info(void);

/*	vMax2	*/
s64 OSC_s64GetVmax2Info(void);

/*	vAvg1	*/
s64 OSC_s64GetVavg1Info(void);

/*	vAvg2	*/
s64 OSC_s64GetVavg2Info(void);

/*	vDiv1	*/
s64 OSC_s64GetVdiv1Info(void);

/*	vDiv2	*/
s64 OSC_s64GetVdiv2Info(void);

/*	tOn1	*/
s64 OSC_s64GetTon1Info(void);

/*	tOn2	*/
s64 OSC_s64GetTon2Info(void);

/*	tDiv	*/
s64 OSC_s64GetTdivInfo(void);

/*	t1	*/
s64 OSC_s64GetT1Info(void);

/*	t2	*/
s64 OSC_s64GetT2Info(void);


#endif /* APP_INFO_OSCILLOSCOPE_INFO_H_ */
