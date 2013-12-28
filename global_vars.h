/*
 * Globals for VFD Modular Clock
 * (C) 2011-2012 Akafugu Corporation
 * (C) 2012 William B Phelps
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
 
#ifndef GLOBALS_H_
#define GLOBALS_H_

#ifdef __FLASH
#define FLASH __flash
#else
#define FLASH
#endif

// date format modes
typedef enum {
  FORMAT_YMD = 0,
  FORMAT_DMY,
  FORMAT_MDY,
} date_format_t;

// alarm type
typedef enum {
  ALARM_NORMAL = 0,
  ALARM_PROGRESSIVE,
} alarm_type_t;

struct __globals
{
	uint8_t EEcheck1;
	uint8_t clock_24h;
	uint8_t show_temp;
	uint8_t show_humid;
	uint8_t show_press;
	uint8_t show_dots;
	uint8_t brightness;
	uint8_t volume;
	uint8_t dateyear;
	uint8_t datemonth;
	uint8_t dateday;
	uint8_t alarmtype;
	uint8_t snooze_enabled;
	uint8_t flw_enabled;
#ifdef HAVE_GPS 
	uint8_t gps_enabled;
	int8_t TZ_hour; // could be negative
	int8_t TZ_minute;
// debugging counters 
	uint8_t gps_cks_errors;  // gps checksum error counter
	uint8_t gps_parse_errors;  // gps parse error counter
	uint8_t gps_time_errors;  // gps time error counter
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
	uint8_t DST_mode;  // DST off, on, auto?
	int8_t DST_offset;  // DST offset in Hours
#endif
#ifdef HAVE_AUTO_DST  // DST rules
	uint8_t DST_Rules[9];
#endif
#ifdef HAVE_AUTO_DATE
	date_format_t date_format;
	uint8_t AutoDate;
#endif
#ifdef HAVE_AUTO_DIM
	uint8_t AutoDim;
	uint8_t AutoDimHour1;
	uint8_t AutoDimLevel1;
	uint8_t AutoDimHour2;
	uint8_t AutoDimLevel2;
	uint8_t AutoDimHour3;
	uint8_t AutoDimLevel3;
#endif
	uint8_t EEcheck2;
};

extern uint8_t g_has_flw; // set to true if there is a four letter word EEPROM attached
extern uint8_t g_gps_updating;  // for signalling GPS update on some displays
extern uint8_t g_DST_updated;  // DST update flag = allow update only once per day
extern uint8_t g_has_dots; // can current shield show dot (decimal points)

void globals_init(void);
void save_globals(void);
extern struct __globals globals; // can't put this here...

#endif

