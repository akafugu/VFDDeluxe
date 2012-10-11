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

#ifndef BUTTON_H_
#define BUTTON_H_

#include "features.h"
#include <stdbool.h>

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

void initialize_button();

bool is_button_pressed(void);

void get_button_state(struct BUTTON_STATE* buttons);

void button_timer(void);

#endif
