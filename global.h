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

#if BOARD == BOARD_VFD_MODULAR_CLOCK

struct PinMap
{
    // HV518 / HV5812
    static const uint8_t data = 11;
    static const uint8_t clock = 13;
    static const uint8_t latch = A0;
    static const uint8_t blank = 6;
    
    // Nixie anodes (not supported on VFD Modular clock)
    static const uint8_t nixie_a1 = -1;
    static const uint8_t nixie_a2 = -1;
    static const uint8_t nixie_a3 = -1;
    static const uint8_t nixie_a4 = -1;
    
    // Input buttons
    static const uint8_t button1 = -1;
    static const uint8_t button2 = -1;
    //static const uint8_t button1 = 4;
    //static const uint8_t button2 = -1;
    
    // other
    static const uint8_t alarm_switch = 2;
    static const uint8_t sig0 = 3;
    static const uint8_t sig1 = 4;
    static const uint8_t sig2 = 5;
    static const uint8_t piezo = 10;
};

#elif BOARD == BOARD_VFD_DELUXE

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
    
    // Input buttons / alarm sitch
    static const uint8_t button1 = 7;
    static const uint8_t button2 = 9;
    static const uint8_t alarm_switch = 12;
    
    // Shield signature pins
    static const uint8_t sig0 = 5;
    static const uint8_t sig1 = A4;
    static const uint8_t sig2 = A3;

    // RTC SQW interrupt
    static const uint8_t sqw = 8; // PB4 / PCINT4
    
    // ofhter
    static const uint8_t piezo = 11;
};

#endif

