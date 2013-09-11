VFDDeluxe Firmware
==================

![Akafugu Modular VFD Clock](/akafugu/VFD-Modular-Clock/raw/master/vfd.jpg)

Arduino-based firmware for the [Akafugu Modular VFD Clock mk2](http://www.akafugu.jp/posts/products/vfd-modular-clock/).

The VFD Modular Clock is a clock based on old-fashioned VFD Display Tubes.

VFD is short for Vacuum Flourescent Display. A VFD display is typically green or blue, and emits a bright light with high contrast. VFD Displays are often found in car radios.

A VFD Display tube looks like an old Vacuum Tube, the predecessor to the transistor. The inside of the tube contains segments that can be lit up to form numbers and letters. Most tubes contain segments for one digit, and several must be stacked together to make a complete display.

The clock itself is modular, it comes with a base board, which is powered by an ATMega32U4 microcontroller and contains a high-voltage VFD driver that is used to light up the display shield that sits on the top board.

For use with mk2 (version 2) of the base board only. For version 1 boards, please use
the original VFD Modular Clock firmware instead:
[https://github.com/akafugu/VFD-Modular-Clock](https://github.com/akafugu/VFD-Modular-Clock)

Firmware
--------

The VFD Modular Clock is based on the ATMega32U4 microcontroller. The firmware is written for the Arduino and covers all basic clock functionality such as setting time and alarm, brightness and 24h/12h time.
