/*
 * Loginc_Analyzer_config.h
 *
 *  Created on: Nov 25, 2022
 *      Author: Ali Emad Ali
 */

#ifndef LOGINC_ANALYZER_CONFIG_H_
#define LOGINC_ANALYZER_CONFIG_H_

/*	LCD	*/
#define LCD_A0_PIN									GPIO_Pin_A0
#define LCD_RST_PIN									GPIO_Pin_A1

#define LCD_SPI_UNIT_NUMBER							SPI_UnitNumber_1
#define LCD_SPI_AFIO_MAP							0

#define LCD_BRIGHTNESS_CONTROL_TIMER_UNIT_NUMBER	4
#define LCD_BRIGHTNESS_CONTROL_TIMER_CHANNEL		TIM_Channel_4
#define LCD_BRIGHTNESS_CONTROL_TIMER_AFIO_MAP		0
#define LCD_BRIGHTNESS_CONTROL_TIMER_FREQ_HZ		100

/*	Running indication LED	*/
#define LED_INDICATOR_PIN							GPIO_Pin_B11

/*	Control buttons	*/
#define BUTTON_1_PIN								GPIO_Pin_A2

/*
 * Analog channels.
 */
#define ANALOG_INPUT_1_PIN							GPIO_Pin_A3
#define ANALOG_INPUT_2_PIN							GPIO_Pin_A4
// must be an element of the enum: 'ADC_SampleTime_t'
#define ADC_SAMPLE_TIME								ADC_SampleTime_1_5


#endif /* LOGINC_ANALYZER_CONFIG_H_ */































