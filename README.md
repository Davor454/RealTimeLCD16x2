# RealTimeLCD16x2
Date and Time displayed on a LCD 2x16 using i2c and stm32F103R nucleo development board
09.06.2026
Added a external coin battery behind the STM board.
Wired the + of the battery connector to the right side
of the SB45 (desoldered the 0 ohm resister on the SB45 before),
and the - to the ground of the USER B1 button(GND is top right pin).
This battery helps keep the time, date and day of week alive while there is no Vdd power to the board.
Once the date, time and day of week is set using the python script timeSender with a USB UART COM device, wired to 
USART1 of the board, the LCD 2x16 will display the time date and day of week even after a power cycle(restart).
I saved the values of the rtc date and day of week into backup registers, and i read them back on every boot, but first I check if
weather this is the First boot using a magic number written on the first boot in DR1 backup register.

TODO: call the python script from the project not from cmd 
TODO: implement alarm with buzzer
TODO: implement rfid to light up green diode on OK and 1 beep, red diode and long beep on NOTOK
