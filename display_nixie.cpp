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

#include "global.h"
#include "display.h"

pin_direct_t nixie_a1_pin;
pin_direct_t nixie_a2_pin;
pin_direct_t nixie_a3_pin;
pin_direct_t nixie_a4_pin;

volatile char ndata[6]; // Digit data for nixies

extern enum shield_t shield;
extern uint8_t display_on;

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

// 1/4 duty cycle 4-digit vfd/nixie hybrid shield (17 channels for VFD, 10 channels for Nixie)
void init_nixie_hybrid()
{
      digitalWrite(PinMap::blank, LOW);

    // Nixie anodes (digits)
    pinMode(PinMap::nixie_a1, OUTPUT);
    pinMode(PinMap::nixie_a2, OUTPUT);
    pinMode(PinMap::nixie_a3, OUTPUT);
    pinMode(PinMap::nixie_a4, OUTPUT);

    nixie_a1_pin.pin = PinMap::nixie_a1;
    nixie_a1_pin.reg = PIN_TO_OUTPUT_REG(PinMap::nixie_a1);
    nixie_a1_pin.bitmask = PIN_TO_BITMASK(PinMap::nixie_a1);
    nixie_a2_pin.pin = PinMap::nixie_a2;
    nixie_a2_pin.reg = PIN_TO_OUTPUT_REG(PinMap::nixie_a2);
    nixie_a2_pin.bitmask = PIN_TO_BITMASK(PinMap::nixie_a2);
    nixie_a3_pin.pin = PinMap::nixie_a3;
    nixie_a3_pin.reg = PIN_TO_OUTPUT_REG(PinMap::nixie_a3);
    nixie_a3_pin.bitmask = PIN_TO_BITMASK(PinMap::nixie_a3);
    nixie_a4_pin.pin = PinMap::nixie_a4;
    nixie_a4_pin.reg = PIN_TO_OUTPUT_REG(PinMap::nixie_a4);
    nixie_a4_pin.bitmask = PIN_TO_BITMASK(PinMap::nixie_a4);
}

void nixie_clear_display(void)
{
    if (shield == SHIELD_IN14) {
        DIRECT_PIN_LOW(nixie_a1_pin.reg, nixie_a1_pin.bitmask);
        DIRECT_PIN_LOW(nixie_a2_pin.reg, nixie_a2_pin.bitmask);
        DIRECT_PIN_LOW(nixie_a3_pin.reg, nixie_a3_pin.bitmask);
        DIRECT_PIN_LOW(nixie_a4_pin.reg, nixie_a4_pin.bitmask);    
    }
    else {
        DIRECT_PIN_LOW(nixie_a1_pin.reg, nixie_a1_pin.bitmask);
        DIRECT_PIN_LOW(nixie_a3_pin.reg, nixie_a3_pin.bitmask);
    }
}

void write_nixie_hybrid(uint8_t digit, uint8_t value)
{
    nixie_clear_display();

    /*
    if (g_blink_on) {
        if (g_blank == 4) { clear_display(); return; }
        else if (g_blank == 1 && (digit == 0 || digit == 1)) { clear_display(); return; }
        else if (g_blank == 2 && (digit == 2 || digit == 3)) { clear_display(); return; }
        else if (g_blank == 3 && (digit == 4 || digit == 5)) { clear_display(); return; }
    }

    if (g_antipoison && g_randomize_on) {
        value = rnd() % 10;
    }
    */

    if (value == 10) return;

    //fixme: write to shift register here

    switch (digit) {
    case 0:
        DIRECT_PIN_HIGH(nixie_a1_pin.reg, nixie_a1_pin.bitmask);
        break; 
    case 1:
        DIRECT_PIN_HIGH(nixie_a2_pin.reg, nixie_a2_pin.bitmask);
        break; 
    case 2:
        DIRECT_PIN_HIGH(nixie_a3_pin.reg, nixie_a3_pin.bitmask);
        break; 
    case 3:
        DIRECT_PIN_HIGH(nixie_a4_pin.reg, nixie_a4_pin.bitmask);
        break; 
    }
}

void write_nixie_6digit(uint8_t digit, uint8_t value1, uint8_t value2, uint8_t value3)
{
    nixie_clear_display();

    /*
    if (g_blink_on) {
        if (g_blank == 4) { clear_display(); return; }
        else if (g_blank == 1 && (digit == 0 || digit == 1)) { clear_display(); return; }
        else if (g_blank == 2 && (digit == 2 || digit == 3)) { clear_display(); return; }
        else if (g_blank == 3 && (digit == 4 || digit == 5)) { clear_display(); return; }
    }

    if (g_antipoison && g_randomize_on) {
        value1 = rnd() % 10;
        value2 = rnd() % 10;
    }
    */

    if (value1 == 10 || value2 == 10) return;

    // write to shift register here
    //set_number(value1, value2);

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

void display_multiplex_hybrid()
{
  if (multiplex_counter_nixie == 0)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 1 && multiplex_counter_nixie <= 10)
    display_on ? write_nixie_hybrid(0, ndata[5]) : nixie_clear_display();
  else if (multiplex_counter_nixie == 11)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 12 && multiplex_counter_nixie <= 21)
    display_on ? write_nixie_hybrid(1, ndata[4]) : nixie_clear_display();
  else if (multiplex_counter_nixie == 22)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 23 && multiplex_counter_nixie <= 32)
    display_on ? write_nixie_hybrid(2, ndata[3]) : nixie_clear_display();
  else if (multiplex_counter_nixie == 33)
    nixie_clear_display();
  else if (multiplex_counter_nixie >= 34 && multiplex_counter_nixie <= 43)
    display_on ? write_nixie_hybrid(3, ndata[2]) : nixie_clear_display();

  multiplex_counter_nixie++;

  if (multiplex_counter_nixie == 44) multiplex_counter_nixie = 0;
}


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

  multiplex_counter_nixie++;

  if (multiplex_counter_nixie == 22) multiplex_counter_nixie = 0;
}

void nixie_print(uint8_t hh, uint8_t mm, uint8_t ss)
{
    ndata[1] = hh % 10;
    hh /= 10;
    ndata[0] = hh % 10;

    ndata[3] = mm % 10;
    mm /= 10;
    ndata[2] = mm % 10;

    ndata[5] = mm % 10;
    mm /= 10;
    ndata[4] = mm % 10;
}

