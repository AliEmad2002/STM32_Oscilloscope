/*
 * Oscilloscope_Menu.h
 *
 *  Created on: Jan 19, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_MENU_H_
#define INCLUDE_APP_OSCILLOSCOPE_MENU_H_


void OSC_voidSelectChangeVoltageDivAsUpDownTraget(void);
void OSC_voidSelectChangeTimeDivAsUpDownTraget(void);
void OSC_voidSelectChangeV1Position(void);
void OSC_voidSelectChangeV2Position(void);
void OSC_voidSelectChangeT1Position(void);
void OSC_voidSelectChangeT2Position(void);

/*	enable / add cursor	*/
extern void OSC_voidEnableCursorV1(void);
extern void OSC_voidEnableCursorV2(void);
extern void OSC_voidEnableCursorT1(void);
extern void OSC_voidEnableCursorT2(void);

/*	disable / remove cursor	*/
extern void OSC_voidDisableCursorV1(void);
extern void OSC_voidDisableCursorV2(void);
extern void OSC_voidDisableCursorT1(void);
extern void OSC_voidDisableCursorT2(void);

/*	increment cursor position (if enabled)	*/
extern void OSC_voidIncrementCursorV1(void);
extern void OSC_voidIncrementCursorV2(void);
extern void OSC_voidIncrementCursorT1(void);
extern void OSC_voidIncrementCursorT2(void);

/*	decrement cursor position (if enabled)	*/
extern void OSC_voidDecrementCursorV1(void);
extern void OSC_voidDecrementCursorV2(void);
extern void OSC_voidDecrementCursorT1(void);
extern void OSC_voidDecrementCursorT2(void);


static Menu_t changeDivMenu = {
	.currentSelected = 0,

	.numberOfElements = 2,

	.elementArr[0] = {
		" Voltage", Menu_ElementType_Callback,
		OSC_voidSelectChangeVoltageDivAsUpDownTraget
	},

	.elementArr[1] = {
		" Time", Menu_ElementType_Callback,
		OSC_voidSelectChangeTimeDivAsUpDownTraget
	}
};

static Menu_t cursorAddMenu = {
	.currentSelected = 0,

	.numberOfElements = 4,

	.elementArr[0] = {
		" Add v1", Menu_ElementType_Callback, OSC_voidEnableCursorV1
	},

	.elementArr[1] = {
		" Add v2", Menu_ElementType_Callback, OSC_voidEnableCursorV2
	},

	.elementArr[2] = {
		" Add t1", Menu_ElementType_Callback, OSC_voidEnableCursorT1
	},

	.elementArr[3] = {
		" Add t2", Menu_ElementType_Callback, OSC_voidEnableCursorT2
	},
};

static Menu_t cursorRemoveMenu = {
	.currentSelected = 0,

	.numberOfElements = 4,

	.elementArr[0] = {
		" Remove v1", Menu_ElementType_Callback, OSC_voidDisableCursorV1
	},

	.elementArr[1] = {
		" Remove v2", Menu_ElementType_Callback, OSC_voidDisableCursorV2
	},

	.elementArr[2] = {
		" Remove t1", Menu_ElementType_Callback, OSC_voidDisableCursorT1
	},

	.elementArr[3] = {
		" Remove t2", Menu_ElementType_Callback, OSC_voidDisableCursorT2
	},
};

static Menu_t cursorSelectMenu = {
	.currentSelected = 0,

	.numberOfElements = 4,

	.elementArr[0] = {
		" Select v1", Menu_ElementType_Callback, OSC_voidSelectChangeV1Position
	},

	.elementArr[1] = {
		" Select v2", Menu_ElementType_Callback, OSC_voidSelectChangeV2Position
	},

	.elementArr[2] = {
		" Select t1", Menu_ElementType_Callback, OSC_voidSelectChangeT1Position
	},

	.elementArr[3] = {
		" Select t2", Menu_ElementType_Callback, OSC_voidSelectChangeT2Position
	},
};

static Menu_t cursorMenu = {
	.currentSelected = 0,

	.numberOfElements = 3,

	.elementArr[0] = {
		" Add", Menu_ElementType_SubMenu, &cursorAddMenu
	},

	.elementArr[1] = {
		" Remove", Menu_ElementType_SubMenu, &cursorRemoveMenu
	},

	.elementArr[2] = {
		" Select", Menu_ElementType_SubMenu, &cursorSelectMenu
	}
};

static Menu_t mainMenu = {
	.currentSelected = 0,

	.numberOfElements = 2,

	.elementArr[0] = {
		" Change division", Menu_ElementType_SubMenu, &changeDivMenu
	},

	.elementArr[1] = {
		" Cursor", Menu_ElementType_SubMenu, &cursorMenu
	}
};

void OSC_voidUpdateMenuOnDisplay(Menu_t* menu);

/*	opens menu (shows it on screen and watches the up / down buttons 	*/
void OSC_voidOpenMenu(Menu_t* menu);

#endif /* INCLUDE_APP_OSCILLOSCOPE_MENU_H_ */
