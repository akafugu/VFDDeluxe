/*
 * Globals for VFD Modular Clock
 * (C) 2011-2013 Akafugu Corporation
 * (C) 2011-2013 William B Phelps
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
//#define FEATURE_AUTO_DIM  // moved to Makefile

#include "global.h"

#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "global_vars.h"

#define EE_CHECK 31 // change this value if you change EE addresses
#define EE_globals 0 // eeprom address

__globals globals = {
	EE_CHECK,
	true, // clock_24h
	false, // show temp
	false, // show humid
	false, // show press
	true, // show dots
	8, // brightness
	0, // volume
	13, 1, 1, // year, month, day
	ALARM_NORMAL, // alarm type
	false, // snooze enabled
	false, // flw enabled
#ifdef HAVE_GPS
	false, // gps enabled
	0, 0, // TZ hour, minute
	0, 0, 0, // gps debug counters
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
	0, 0, // DST mode, offset
#endif
#ifdef HAVE_AUTO_DST  // DST rules
#ifdef DST_NSW
	10,1,1,2,3,1,1,2,1,
#else
	3,1,2,2,11,1,1,2,1,
#endif
#endif
#ifdef HAVE_AUTO_DATE
	FORMAT_YMD, // date format
	false, // auto date
#endif
#ifdef HAVE_AUTO_DIM
	false, // auto dim
	7, 8, // auto dim1 hour, level
	19, 5, // auto dim2 hour, level
	22, 2, // auto dim3 hour, level
#endif
	EE_CHECK,
};

uint8_t g_gps_updating;  // for signalling GPS update on some displays
uint8_t g_DST_updated;  // DST update flag = allow update only once per day
uint8_t g_has_dots; // can current shield show dot (decimal points)

// workaround: Arduino avr-gcc toolchain is missing eeprom_update_byte
#define eeprom_update_byte eeprom_write_byte

void init_globals()
{
	for (unsigned int t=0; t<sizeof(globals); t++) {
		eeprom_write_byte((uint8_t *)EE_globals + t, *((char*)&globals + t));
	}
}

void save_globals()
{
	for (unsigned int t=0; t<sizeof(globals); t++) {
		uint8_t b1 = eeprom_read_byte((uint8_t *)EE_globals + t);
		if (b1 != *((char *) &globals + t))
			eeprom_write_byte((uint8_t *)EE_globals + t, *((char*)&globals + t));
	}
}

void globals_init(void) 
{
	uint8_t ee_check1 = eeprom_read_byte((uint8_t *)EE_globals + (&globals.EEcheck1-&globals.EEcheck1));
	uint8_t ee_check2 = eeprom_read_byte((uint8_t *)EE_globals + (&globals.EEcheck2-&globals.EEcheck1));
	if ((ee_check1!=EE_CHECK) || (ee_check2!=EE_CHECK)) {
		init_globals();
	}
	for (unsigned int t=0; t<sizeof(globals); t++)
		*((char*)&globals + t) = eeprom_read_byte((uint8_t *)EE_globals + t);
}
