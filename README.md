# STM32_Oscilloscope
STM32f103 based digital oscilloscope. 1MSPS. Dual channel (Y-t) and (Y-X) modes

# Main features:
* Single and dual channel y-t mode.
* Single and dual channel x-y mode.
* 1 MSPS.
* Maximum of 8V peak to peak.
* Auto voltage and time zoom setting.
* Interleaved sampling. (For showing extra fast periodic signals. Explained in issues)
* Vertical offset.
* Horizontal offset. (x-y mode only)
* Two cursors per each of the axis.
* Zooming in and out. (changing volts and seconds per division)

# Features to be added (by March2022):
* Math expression in x-y mode using "MathExpressionParser" project: https://github.com/AliEmad2002/MathExpressionParser.
* SD card screen shot.
* Transient capture.

# Discalimer:
* My main focus in this project was embedded SW and HW. Hence, the electronic circuits are the very basic ones for the job.
* Any ways, these are replacable if the MCU to use has PGA (programmable gain amplifier), and Vref pin. For example STM32L476xx.
* Thus, better sampling is obtained using advaced external circuitry, or using MCU like the one mentioned.

# Other repos used:
* https://github.com/AliEmad2002/STM32F401x_10x_COTS
