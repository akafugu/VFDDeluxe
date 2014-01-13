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

//#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "global_vars.h"
#include "display.h"

#define EE_CHECK 43 // change this value if you change EE addresses
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
#ifdef HAVE_FLW
	false, // flw enabled
#endif
#ifdef HAVE_GPS
	false, // gps enabled
	0, 0, // TZ hour, minute
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
	0, 0, // DST mode, offset
#endif
#ifdef HAVE_AUTO_DST  // DST rules
#ifdef DST_NSW
	10,1,1,2, 4,1,1,3, 1,
#else
	3,1,2,2, 11,1,1,2, 1,
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
#ifdef HAVE_RTC_SQW
	true, // sqw enabled
#endif
	EE_CHECK,
};

//uint8_t alarm_hour, alarm_min, alarm_sec;
//uint8_t hour, min, sec;

uint8_t g_gps_updating = false;  // for signalling GPS update on some displays
uint8_t g_DST_updated = false;  // DST update flag = allow update only once per day
uint8_t g_has_dots = false; // can current shield show dot (decimal points)
#ifdef HAVE_GPS
uint8_t g_gps_cks_errors = 0;  // gps checksum error counter
uint8_t g_gps_parse_errors = 0;  // gps parse error counter
uint8_t g_gps_time_errors = 0;  // gps time error counter
#endif

void save_globals()
{
uint8_t c1 = 0; // # of bytes written
//  tone(PinMap::piezo, 3000, 20);  // tick
	for (unsigned int p=0; p<sizeof(globals); p++) {
		uint8_t b1 = eeprom_read_byte((uint8_t *)EE_globals + p);
		uint8_t b2 = *((uint8_t *) &globals + p);
		if (b1 != b2) {
			eeprom_write_byte((uint8_t *)EE_globals + p, *((uint8_t*)&globals + p));
			c1++;
		}
	}
	if (c1) {
		tone(PinMap::piezo, 1760, 50);  // short beep
	}
}

void globals_init(void) 
{
	uint8_t ee_check1 = eeprom_read_byte((uint8_t *)EE_globals + (&globals.EEcheck1-&globals.EEcheck1));
	uint8_t ee_check2 = eeprom_read_byte((uint8_t *)EE_globals + (&globals.EEcheck2-&globals.EEcheck1));
	if ((ee_check1!=EE_CHECK) || (ee_check2!=EE_CHECK)) { // has EE been initialized?
		for (unsigned int p=0; p<sizeof(globals); p++) { // copy globals structure to EE memory
			eeprom_write_byte((uint8_t *)EE_globals + p, *((uint8_t*)&globals + p));
		}
	}
	else { // read globals from EE
		for (unsigned int p=0; p<sizeof(globals); p++) // read gloabls from EE
			*((uint8_t*)&globals + p) = eeprom_read_byte((uint8_t *)EE_globals + p);
	}
}
