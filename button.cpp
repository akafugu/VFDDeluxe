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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"

#define BUTTON_PORT PORTD
#define BUTTON_DDR  DDRD
#define BUTTON_PIN  PIND

#define BUTTON1_BIT  PORTD4

void initialize_button()
{
  // Set button as inputs and enable pullup
  BUTTON_DDR &= ~(_BV(BUTTON1_BIT));
  BUTTON_PORT |= _BV(BUTTON1_BIT);
}

bool is_button_pressed(void)
{
	if (BUTTON_PIN & _BV(BUTTON1_BIT))
		return true;
	return false;
}

uint8_t saved_keystatus = 0x00;
uint8_t keydown_keys = 0x00;
uint8_t keyup_keys = 0x00;
uint8_t keyrepeat_keys = 0x00;

uint16_t keyboard_counter[1] = {0};
uint8_t button_bit[1] = { _BV(BUTTON1_BIT) };

//#define REPEAT_SPEED	2000
#define REPEAT_SPEED	20

void button_timer(void)
{
	uint8_t keystatus = ~(BUTTON_PIN)&(_BV(BUTTON1_BIT));
	keydown_keys |= (uint8_t)(keystatus & ~(saved_keystatus));
	keyup_keys   |= (uint8_t)(~(keystatus) & saved_keystatus);
	saved_keystatus = keystatus;
	
	if(~(keydown_keys)&button_bit[0])
		; // Do nothing, no keyrepeat is needed
	else if(keyup_keys&button_bit[0])
		keyboard_counter[0] = 0;
	else {
		if(keyboard_counter[0] >= REPEAT_SPEED) {
			keyrepeat_keys |= button_bit[0];
			keyboard_counter[0] = 0;
		}
		keyboard_counter[0]++;
	}
}

void get_button_state(struct BUTTON_STATE* buttons)
{
	buttons->b2_keydown = keydown_keys&_BV(BUTTON1_BIT);
	buttons->b2_keyup = keyup_keys&_BV(BUTTON1_BIT);
	buttons->b2_repeat = keyrepeat_keys&_BV(BUTTON1_BIT);
	
	// Reset if we got keyup
	if(keyup_keys&_BV(BUTTON1_BIT))
	{
		keydown_keys   &= ~(_BV(BUTTON1_BIT));
		keyup_keys     &= ~(_BV(BUTTON1_BIT));
		keyrepeat_keys &= ~(_BV(BUTTON1_BIT));
		keyboard_counter[0] = 0;
	}

    buttons->none_held = ~(saved_keystatus)&(_BV(BUTTON1_BIT));
}
