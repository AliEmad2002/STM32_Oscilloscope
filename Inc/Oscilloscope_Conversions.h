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
#define GET_CH1_V_IN_PIXELS(v_adc)                                            \
(	                                                                   	      \
	(5 * 1000000 + Global_Offset1MicroVolts) /                                \
	(s32)Global_CurrentCh1MicroVoltsPerPix -						                  \
	(10000000 * (s64)(v_adc)) / Global_CurrentCh1MicroVoltsPerPix / 4096 + SIGNAL_LINE_LENGTH / 2 	                                                      \
)

#define GET_CH2_V_IN_PIXELS(v_adc)                                            \
(	                                                                   	      \
	(5 * 1000000 + Global_Offset2MicroVolts) /                                \
	(s32)Global_CurrentCh2MicroVoltsPerPix -						                  \
	(10000000 * (s64)(v_adc)) / Global_CurrentCh2MicroVoltsPerPix / 4096 + SIGNAL_LINE_LENGTH / 2 	                                                      \
)

#define GET_V_IN_MICRO_VOLTS(v_adc)                 \
(	                                                \
	5 * 1000000 - (10000000 * (s64)(v_adc)) / 4096	\
)


#endif /* INCLUDE_APP_OSCILLOSCOPE_CONVERSIONS_H_ */
