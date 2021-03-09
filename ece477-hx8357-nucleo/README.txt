Tested with the STM32L053R8T7 on a Nucleo-64 board.

To get the driver code working: (STM32L053x-series)

 - The SPI pins must be set to a maximum output speed of "medium"
	- Failure to do so means the pins won't meet the setup and hold times for the HX8357D.

 - Use hardware NSS. 
	- NSS pulse is optional, but for STM32L053x-series chips, they don't have it

 - Set the SPI prescaler so that the maximum transfer rate is <= 8 MBits/sec.
	- Unfortunately, no combination of prescaler and GPIO output speeds 
		allowed for any higher transfer rates in my testing...