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

#include "global.h"

#ifdef HAVE_NIXIE_SUPPORT

#include "display.h"

pin_direct_t nixie_a1_pin;
pin_direct_t nixie_a2_pin;
pin_direct_t nixie_a3_pin;
pin_direct_t nixie_a4_pin;

volatile char ndata[6]; // Digit data for nixies

extern enum shield_t shield;
extern uint8_t display_on;

void write_vfd_8bit(uint8_t data);

// 1/2 duty cycle 6-digit nixie shield (10x3 channels + 2 dot channels)
void init_nixie_6digit()
{
  digitalWrite(PinMap::blank, LOW);

  // Nixie anodes (digits)
  pinMode(PinMap::nixie_a1, OUTPUT);
  pinMode(PinMap::nixie_a3, OUTPUT);

  nixie_a1_pin.pin = PinMap::nixie_a1;
  nixie_a1_pin.reg = PIN_TO_OUTPUT_REG(PinMap::nixie_a1);
  nixie_a1_pin.bitmask = PIN_TO_BITMASK(PinMap::nixie_a1);
  nixie_a3_pin.pin = PinMap::nixie_a3;
  nixie_a3_pin.reg = PIN_TO_OUTPUT_REG(PinMap::nixie_a3);
  nixie_a3_pin.bitmask = PIN_TO_BITMASK(PinMap::nixie_a3);
}

void nixie_clear_display(void)
{
    DIRECT_PIN_LOW(nixie_a1_pin.reg, nixie_a1_pin.bitmask);
    DIRECT_PIN_LOW(nixie_a3_pin.reg, nixie_a3_pin.bitmask);
}

void write_nixie(uint8_t value1, uint8_t value2, uint8_t value3);
void write_nixie_dots();
void write_nixie_6digit(uint8_t digit, uint8_t value1, uint8_t value2, uint8_t value3, bool dots = false);

void write_nixie_6digit(uint8_t digit, uint8_t value1, uint8_t value2, uint8_t value3, bool dots)
{
    nixie_clear_display();

    if (dots)
      write_nixie_dots();
    else
      write_nixie(value1, value2, value3);

    switch (digit) {
    case 0:
        DIRECT_PIN_HIGH(nixie_a1_pin.reg, nixie_a1_pin.bitmask);
        break; 
    case 1:
        DIRECT_PIN_HIGH(nixie_a3_pin.reg, nixie_a3_pin.bitmask);
        break; 
    }
}

uint8_t multiplex_counter_nixie;

void display_multiplex_in14()
{
  if (multiplex_counter_nixie == 0)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 1 && multiplex_counter_nixie <= 10)
    display_on ? write_nixie_6digit(0, ndata[5], ndata[3], ndata[1]) : nixie_clear_display();
  else if (multiplex_counter_nixie == 11)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 12 && multiplex_counter_nixie <= 21)
    display_on ? write_nixie_6digit(1, ndata[4], ndata[2], ndata[0]) : nixie_clear_display();
  
  // fixme: this is to support the nixie's dots.
  // fixme: dots are on the left side of digits 3 and 5 on IN-14, so parameter 1 should be 0
  else if (multiplex_counter_nixie == 22)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 23 && multiplex_counter_nixie <= 27)
    display_on ? write_nixie_6digit(1, 0, 0, 0, true) : nixie_clear_display();

  multiplex_counter_nixie++;

  if (multiplex_counter_nixie == 28) multiplex_counter_nixie = 0;
}

#ifdef IN14_FIX

volatile char xdata[6]; // Digit data for nixies

uint8_t remap_in14(uint8_t val)
{
    // fix for incorrectly wired first version of IN14 shield
    //ndata[i] = remap_in14(ndata[i]);
    
    if (val == 0)
        return 1;
    if (val == 1)
        return 0;
    if (val == 2)
        return 9;
    if (val == 3)
        return 8;
    if (val == 4)
        return 7;
    if (val == 5)
        return 6;
    if (val == 6)
        return 5;
    if (val == 7)
        return 4;
    if (val == 8)
        return 3;
    if (val == 9)
        return 2;
    if (val == 10)
        return 10;
    
    return val;
}
#endif


void nixie_print(uint8_t hh, uint8_t mm, uint8_t ss)
{
#ifdef IN14_FIX
/*
    xdata[0] = ss % 10;
    xdata[1] = ss % 10;
    xdata[2] = ss % 10;
    xdata[3] = ss % 10;
    xdata[4] = ss % 10;
    xdata[5] = ss % 10;
    */

    xdata[4] = hh % 10;
    hh /= 10;
    xdata[5] = hh % 10;

    xdata[2] = mm % 10;
    mm /= 10;
    xdata[3] = mm % 10;

    xdata[0] = ss % 10;
    ss /= 10;
    xdata[1] = ss % 10;
    
    // fix for incorrectly wired first version of IN14 shield
    for (uint8_t i = 0; i <= 5; i++)
        ndata[i] = remap_in14(xdata[i]);
#else
/*
    ndata[0] = ss % 10;
    ndata[1] = ss % 10;
    ndata[2] = ss % 10;
    ndata[3] = ss % 10;
    ndata[4] = ss % 10;
    ndata[5] = ss % 10;
    */

    ndata[4] = hh % 10;
    hh /= 10;
    ndata[5] = hh % 10;

    ndata[2] = mm % 10;
    mm /= 10;
    ndata[3] = mm % 10;

    ndata[0] = ss % 10;
    ss /= 10;
    ndata[1] = ss % 10;
#endif
}

void nixie_print_compact(uint8_t hh, uint8_t mm, uint8_t ss)
{
#ifdef IN14_FIX
    Xdata[5] = 10;

    Xdata[3] = hh % 10;
    hh /= 10;
    Xdata[4] = hh % 10;

    Xdata[1] = mm % 10;
    mm /= 10;
    Xdata[2] = mm % 10;

    Xdata[0] = 10;
    
    // fix for incorrectly wired first version of IN14 shield
    for (uint8_t i = 0; i <= 5; i++)
        ndata[i] = remap_in14(xdata[i]);
#else
    ndata[5] = 10;

    ndata[3] = hh % 10;
    hh /= 10;
    ndata[4] = hh % 10;

    ndata[1] = mm % 10;
    mm /= 10;
    ndata[2] = mm % 10;

    ndata[0] = 10;
#endif

}

void nixie_clear_data()
{
    ndata[0] = ndata[1] = ndata[2] = ndata[3] = ndata[4] = ndata[5] = 10;  
}

#else

void nixie_print(uint8_t hh, uint8_t mm, uint8_t ss) {}
void nixie_print_compact(uint8_t hh, uint8_t mm, uint8_t ss) {}
void nixie_clear_data() {}

#endif // HAVE_NIXIE_SUPPORT
