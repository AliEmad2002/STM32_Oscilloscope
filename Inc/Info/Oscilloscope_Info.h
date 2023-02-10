/*
 * Oscilloscope_Info.h
 *
 *  Created on: Feb 8, 2023
 *      Author: Ali Emad Ali
 */

#ifndef APP_INFO_OSCILLOSCOPE_INFO_H_
#define APP_INFO_OSCILLOSCOPE_INFO_H_

#define MAX_NAME_LEN	6
#define MAX_UNIT_LEN	3

#define NUMBER_OF_INFO	15

typedef struct{
	s64 (*getValInNanoCallback)(void);
	char name[MAX_NAME_LEN];
	char unit[MAX_UNIT_LEN];
	b8 enabled;
}OSC_Info_t;

/*	Inits info array	*/
void OSC_voidInitInfo(void);

/*	enables info	*/
void OSC_voidEnableInfo(u8 infoIndex);

/*	disables info	*/
void OSC_voidDisableInfo(u8 infoIndex);

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
	u64 valInNano, u32* valInteger, u32* valFraction, char* unitPrefix);

void OSC_voidGetInfoStringToPrint(char* str, u8 infoIndex);

void OSC_voidSSetTFTBoundariesToInfo(u8 infoIndex);




#endif /* APP_INFO_OSCILLOSCOPE_INFO_H_ */
