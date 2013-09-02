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

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdbool.h>

// Button port & pins
#define BUTTON_PORT  PORTB
#define BUTTON_DDR   DDRDB
#define BUTTON_PIN   PINB
#define BUTTON_BIT_1 BUTTON_PORT6
#define BUTTON_BIT_2 BUTTON_PORT7

struct BUTTON_STATE
{
	bool b1_keydown : 1;
	bool b1_keyup : 1;
	bool b1_repeat : 1;
	bool b2_keydown : 1;
	bool b2_keyup : 1;
	bool b2_repeat : 1;
	
	bool both_held : 1;
	bool none_held : 1;
};

struct BUTTON_STATE_OLD
{
	bool pressed, released, held;
};

void initialize_button(uint8_t pin1, int8_t pin2);

//bool is_button_pressed(void);

void get_button_state(struct BUTTON_STATE* buttons);

void button_timer(void);

#endif
