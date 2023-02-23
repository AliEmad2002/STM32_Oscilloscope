# STM32_Oscilloscope
STM32f103 based digital oscilloscope. 1MSPS. Dual channel (Y-t) and (Y-X) modes

# Main features:
* Single and dual channel y-t mode.
* Single and dual channel x-y mode.
* 1 MSPS.
* Maximum of 8V peak to peak.
* Auto voltage and time zoom setting.
* Vertical offset.
* Horizontal offset. (x-y mode only)
* Two cursors per each of the axis.
* Zooming in and out. (Changing volts and seconds per division)
* User configuration saving on flash memory.

# Features to be added (by March2022):
* Math expression in x-y mode using "MathExpressionParser" project: https://github.com/AliEmad2002/MathExpressionParser.
* SD card screen shot.
* Transient capture.
* Interleaved sampling. (For showing extra fast periodic signals. Explained in issues: https://github.com/AliEmad2002/STM32_Oscilloscope/issues/12)

# Example usage:
* Auto zoom setting, offset, votage and time per div setting:

https://user-images.githubusercontent.com/99054912/218281196-0afc6911-5c1d-4a83-83c8-a81065aafef6.mp4
                                             
* Cursors:

https://user-images.githubusercontent.com/99054912/218280302-0e1fd5a7-b59d-487b-b7cb-dc946272d8b8.mp4

# Disclaimer:
* My main focus in this project was embedded SW and HW. Hence, the electronic circuits used are the very basic ones for the job.
* Any ways, most of these are not needed if the MCU to use has PGA (programmable gain amplifier), and Vref pin. For example STM32L476xx.
* Thus, better sampling is obtained using advanced external circuitry, or using MCU like the one mentioned.

# Issues:
* Further documentation at "issues": (check closed ones too)
https://github.com/AliEmad2002/STM32_Oscilloscope/issues

# Other repos used:
* https://github.com/AliEmad2002/STM32F401x_10x_COTS
