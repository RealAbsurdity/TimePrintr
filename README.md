# TimePrintr
A clock based on a Thermal Receipt Printer. Finally you will have a written record of time as it passes!
This is a work in progress, the code here utilizes the arduino libraries, the Adafruit Thermal Printer Library and the Adafruit RTC library.

https://github.com/adafruit/Adafruit-Thermal-Printer-Library

https://github.com/adafruit/RTClib

A simple clock based on the Adafruit RTC, Adafruit Thermal Recepit Printer Guts and an Arduino UNO. 

This softare uses the Adafruit Thermal Printer Library and the Adafruit RTC library.

Hardware Description:
Setup for the printer is based on SofwareSerial and uses pins 9 and 10 for RX and TX respectively.
Pin 11 is wired to a momentary contact switch and ground initialized to use internal pullup.

The printer will print the time every five minutes and when the button is pushed. A long push of the
button will also print the current date. The time is set on the RTC module according to the system time 
when the code is compiled. There is currently no method to set the time manually.
