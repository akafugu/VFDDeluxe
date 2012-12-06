/*
 * VFD Deluxe
 * (C) 2011-12 Akafugu Corporation
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

// GLOBAL include file: Should be the first file to include in all source files

#include <Arduino.h>

#include "features.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include <stdbool.h>
#include <stdlib.h>

#include "direct_pin_read.h"


// fixme: pin mapping should be in separate file?

struct PinMap
{
    // HV518 / HV5812
    static const uint8_t data = A0;
    static const uint8_t clock = A2;
    static const uint8_t latch = A1;
    //static const uint8_t blank = 9; // v1    
    static const uint8_t blank = 6;
    
    // Nixie anodes
    static const uint8_t nixie_a1 = A3;
    static const uint8_t nixie_a2 = 17;
    static const uint8_t nixie_a3 = A4;
    static const uint8_t nixie_a4 = 16;
    
    // Input buttons
    static const uint8_t button1 = 7;
    static const uint8_t button2 = 9;
    //static const uint8_t button1 = 4;
    //static const uint8_t button2 = -1;
};
