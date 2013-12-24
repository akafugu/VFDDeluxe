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

// Settings saved to eeprom
#define EE_CHECK 44 // change this value if you change EE addresses
#define EE_check_byte 0 
#define EE_24h_clock 1
#define EE_show_humid 2
#define EE_show_press 3
#define EE_show_temp 4
#define EE_show_dots 5
#define EE_brightness 6
#define EE_volume 7
#define EE_dateyear 8
#define EE_datemonth 9
#define EE_dateday 10
#define EE_alarmtype 11
#define EE_snooze_enabled 12
#ifdef HAVE_FLW
#define EE_flw_enabled 13
#endif
#ifdef HAVE_GPS
#define EE_gps_enabled 14  // 0, 48, or 96 - default no gps
#define EE_TZ_hour 15
#define EE_TZ_minute 16
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
#define EE_DST_mode 17  // 0: off, 1: on, 2: Auto
#define EE_DST_offset 18
#endif
#ifdef HAVE_AUTO_DATE
#define EE_date_format 19
#define EE_Region 20  // default European date format Y/M/D
#define EE_AutoDate 21
#endif
#ifdef HAVE_AUTO_DIM
#define EE_AutoDim 22
#define EE_AutoDimHour 23
#define EE_AutoDimLevel 24
#define EE_AutoBrtHour 25
#define EE_AutoBrtLevel 26
#endif
#ifdef HAVE_AUTO_DST
#define EE_DST_Rule0 27  // DST start month
#define EE_DST_Rule1 28  // DST start dotw
#define EE_DST_Rule2 29  // DST start week
#define EE_DST_Rule3 30  // DST start hour
#define EE_DST_Rule4 31 // DST end month
#define EE_DST_Rule5 30  // DST end dotw
#define EE_DST_Rule6 32  // DST end week
#define EE_DST_Rule7 33  // DST end hour
#define EE_DST_Rule8 34  // DST offset
#endif

extern int8_t g_24h_clock;
extern int8_t g_show_humid;
extern int8_t g_show_press;
extern int8_t g_show_temp;
extern int8_t g_show_dots;
extern int8_t g_brightness;
extern int8_t g_volume;

extern int8_t g_dateyear;
extern int8_t g_datemonth;
extern int8_t g_dateday;
extern uint8_t g_alarmtype;
extern uint8_t g_snooze_enabled;

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

