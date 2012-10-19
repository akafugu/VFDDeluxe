/*
 * Four Letter Word Generator
 * (C) 2012 Akafugu Corporation
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

#ifndef FLW_H__
#define FLW_H__

#include <inttypes.h>
#include <stdbool.h>

void seed_random(uint32_t seed);
bool has_eeprom(void);
unsigned long get_word(unsigned long offset, char* word);

#endif
