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

#ifndef DISPLAY_NIXIE_H_
#define DISPLAY_NIXIE_H_

#include "features.h"
#include <stdbool.h>
#include <avr/io.h>

void init_nixie_6digit();
void init_nixie_hybrid();

void display_multiplex_in14();
void display_multiplex_hybrid();

void nixie_print(uint8_t hh, uint8_t mm, uint8_t ss);
void nixie_print_compact(uint8_t hh, uint8_t mm, uint8_t ss);
void nixie_clear_data();

#endif // DISPLAY_NIXIE_H

