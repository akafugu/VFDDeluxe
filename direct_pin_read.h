/*
 * VFD Deluxe - Firmware for VFD Modular Clock mk2
 * (C) 2011-13 Akafugu Corporation
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 */

// Based on code in this library http://www.pjrc.com/teensy/td_libs_Encoder.html
// Modified by Akafugu for VFDDeluxe Firmware Support

#ifndef DIRECT_PIN_READ_H_
#define DIRECT_PIN_READ_H_

#if defined(__AVR__)

#define IO_REG_TYPE uint8_t
#define PIN_TO_INPUT_REG(pin)         (portInputRegister(digitalPinToPort(pin)))
#define PIN_TO_OUTPUT_REG(pin)        (portOutputRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)           (digitalPinToBitMask(pin))
#define DIRECT_PIN_READ(base, mask)   (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_PIN_HIGH(base, mask)   ((*(base)) |= (mask))
#define DIRECT_PIN_LOW(base, mask)    ((*(base)) &= ~(mask))

#elif defined(__PIC32MX__)

#define IO_REG_TYPE uint32_t
#define PIN_TO_BASEREG(pin)             (portModeRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define DIRECT_PIN_READ(base, mask)	(((*(base+4)) & (mask)) ? 1 : 0)
#define RAW_PIN_READ(base, mask)        ((*(base)) & (mask))

#else

#error Unsupported architecture

#endif

struct pin_direct_t
{
    uint8_t pin;
    volatile IO_REG_TYPE* reg;
    IO_REG_TYPE bitmask;
};

#endif // DIRECT_PIN_READ_H_

