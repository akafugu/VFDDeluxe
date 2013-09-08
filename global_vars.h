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

extern uint8_t b_24h_clock;
extern uint8_t b_show_temp;
extern uint8_t b_show_dots;
extern uint8_t b_brightness;
extern uint8_t b_volume;

extern uint8_t b_dateyear;
extern uint8_t b_datemonth;
extern uint8_t b_dateday;
extern uint8_t b_alarmtype;

#ifdef HAVE_FLW
extern uint8_t b_flw_enabled;
#endif
#ifdef HAVE_GPS 
extern uint8_t b_gps_enabled;
extern uint8_t b_TZ_hour;
extern uint8_t b_TZ_minute;
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
extern uint8_t b_DST_mode;  // DST off, on, auto?
extern uint8_t b_DST_offset;  // DST offset in Hours
extern uint8_t b_DST_updated;  // DST update flag = allow update only once per day
#endif
#ifdef HAVE_AUTO_DATE
extern uint8_t b_date_format;
extern uint8_t b_AutoDate;
#endif
#ifdef HAVE_AUTO_DIM
extern uint8_t b_AutoDim;
extern uint8_t b_AutoDimHour;
extern uint8_t b_AutoDimLevel;
extern uint8_t b_AutoBrtHour;
extern uint8_t b_AutoBrtLevel;
#endif
#ifdef HAVE_AUTO_DST
extern uint8_t b_DST_Rule0;
extern uint8_t b_DST_Rule1;
extern uint8_t b_DST_Rule2;
extern uint8_t b_DST_Rule3;
extern uint8_t b_DST_Rule4;
extern uint8_t b_DST_Rule5;
extern uint8_t b_DST_Rule6;
extern uint8_t b_DST_Rule7;
extern uint8_t b_DST_Rule8;
#endif

extern int8_t g_24h_clock;
extern int8_t g_show_temp;
extern int8_t g_show_dots;
extern int8_t g_brightness;
extern int8_t g_volume;

extern int8_t g_dateyear;
extern int8_t g_datemonth;
extern int8_t g_dateday;
extern uint8_t g_alarmtype;

extern uint8_t g_has_flw; // set to true if there is a four letter word EEPROM attached
extern int8_t g_flw_enabled;

#ifdef HAVE_GPS 
extern int8_t g_gps_enabled;
extern int8_t g_TZ_hour;
extern int8_t g_TZ_minute;
// debugging counters 
extern int8_t g_gps_cks_errors;  // gps checksum error counter
extern int8_t g_gps_parse_errors;  // gps parse error counter
extern int8_t g_gps_time_errors;  // gps time error counter
#endif
extern int8_t g_gps_updating;  // for signalling GPS update on some displays

#if defined HAVE_GPS || defined HAVE_AUTO_DST
extern int8_t g_DST_mode;  // DST off, on, auto?
extern int8_t g_DST_offset;  // DST offset in Hours
extern int8_t g_DST_updated;  // DST update flag = allow update only once per day
#endif
#ifdef HAVE_AUTO_DST  // DST rules
//DST_Rules dst_rules = {{3,1,2,2},{11,1,1,2},1};   // initial values from US DST rules as of 2011
extern int8_t g_DST_Rules[9];
#endif
#ifdef HAVE_AUTO_DATE
extern date_format_t g_date_format;
extern int8_t g_AutoDate;
#endif
#ifdef HAVE_AUTO_DIM
extern int8_t g_AutoDim;
extern int8_t g_AutoDimHour;
extern int8_t g_AutoDimLevel;
extern int8_t g_AutoBrtHour;
extern int8_t g_AutoBrtLevel;
#endif
extern uint8_t g_has_dots; // can current shield show dot (decimal points)

void globals_init(void);

#endif

