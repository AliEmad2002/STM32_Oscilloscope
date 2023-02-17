/*
 * Oscilloscope_Conversions.h
 *
 *  Created on: Feb 11, 2023
 *      Author: Ali Emad Ali
 */

#ifndef INCLUDE_APP_OSCILLOSCOPE_CONVERSIONS_H_
#define INCLUDE_APP_OSCILLOSCOPE_CONVERSIONS_H_


/*
* Eqn.:
* "v_ch" is the very main input that user can handle.
* "v_opamp" is op-amp's output voltage.
* "v_adc" is the converted 12-bit value by internal ADC.
*
* v_ch 	= 5 - v_opamp / 0.33			[volts]
* 		= 5 - 10 * v_adc / 4096			[volts]
*
* v_pix = v_ch * pixels_per_volt + SIGNAL_LINE_LENGTH / 2
*
* 		 = 	5 * pixels_per_volt -
* 		 	(10 * pixels_per_volt * v_adc) / 4096 +
* 		 	64							[pixels]
*/
#define GET_CH1_V_IN_MICRO_VOLTS(v_adc)             \
(	                                                \
	5e6 - (10e6 * (s64)(v_adc)) / 4096				\
)

#define GET_CH1_V_IN_PIXELS(v_adc)                                          \
(	                                                                   	    \
	(5e6 + Global_Offset1MicroVolts) /										\
	(s32)Global_CurrentCh1MicroVoltsPerPix -						        \
	(10e6 * (s64)(v_adc)) / Global_CurrentCh1MicroVoltsPerPix / 4096 +  	\
	SIGNAL_LINE_LENGTH / 2 	                                                \
)

#define GET_Y_PIX_MATH_MODE(yParsed)               \
(                                                  \
	((yParsed) + Global_Offset1MicroVolts) /       \
	(s32)Global_CurrentCh1MicroVoltsPerPix + 64    \
)

#define GET_CH2_V_IN_MICRO_VOLTS(v_adc)             \
(	                                                \
	(33e5 * (s64)(v_adc)) / 4096					\
)

#define GET_CH2_V_IN_PIXELS(v_adc)                                   \
(	                                                                 \
	(Global_Offset2MicroVolts + GET_CH2_V_IN_MICRO_VOLTS((v_adc))) / \
	(s32)Global_CurrentCh2MicroVoltsPerPix + SIGNAL_LINE_LENGTH / 2  \
)

#define GET_X_PIX_MATH_MODE(xParsed)               \
(                                                  \
	((xParsed) + Global_Offset2MicroVolts) /       \
	(s32)Global_CurrentCh2MicroVoltsPerPix + 80    \
)


#endif /* INCLUDE_APP_OSCILLOSCOPE_CONVERSIONS_H_ */
