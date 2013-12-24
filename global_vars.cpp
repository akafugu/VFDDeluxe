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
//#define FEATURE_AUTO_DIM  // moved to Makefile

#include "global.h"

#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "global_vars.h"

int8_t g_24h_clock = true;
int8_t g_show_temp = true;
int8_t g_show_humid = false;
int8_t g_show_press = false;
int8_t g_show_dots = true;
int8_t g_brightness = 5;
int8_t g_volume = 0;

int8_t g_dateyear;
int8_t g_datemonth;
int8_t g_dateday;
extern int8_t g_autodate;

uint8_t g_has_flw;  // does the unit have an EEPROM with the FLW database?
int8_t g_flw_enabled;

#ifdef HAVE_GPS 
int8_t g_gps_enabled;
int8_t g_TZ_hour;
int8_t g_TZ_minute;
// debugging counters 
int8_t g_gps_cks_errors;  // gps checksum error counter
int8_t g_gps_parse_errors;  // gps parse error counter
int8_t g_gps_time_errors;  // gps time error counter
#endif
int8_t g_gps_updating;  // for signalling GPS update on some displays
#if defined HAVE_GPS || defined HAVE_AUTO_DST
int8_t g_DST_mode;  // DST off, on, auto?
int8_t g_DST_offset;  // DST offset in Hours
int8_t g_DST_updated;  // DST update flag = allow update only once per day
#endif
#ifdef HAVE_AUTO_DST  // DST rules
int8_t g_DST_Rules[9];
#endif
#ifdef HAVE_AUTO_DATE
date_format_t g_date_format;
int8_t g_AutoDate;
#endif
#ifdef HAVE_AUTO_DIM
int8_t g_AutoDim;
int8_t g_AutoDimHour;
int8_t g_AutoDimLevel;
int8_t g_AutoBrtHour;
int8_t g_AutoBrtLevel;
#endif
uint8_t g_has_dots; // can current shield show dot (decimal points)

// workaround: Arduino avr-gcc toolchain is missing eeprom_update_byte
#define eeprom_update_byte eeprom_write_byte

void clean_eeprom()
{
    eeprom_update_byte((uint8_t *)EE_24h_clock, 1);    
#ifdef HAVE_HUMIDITY
    eeprom_update_byte((uint8_t *)EE_show_humid, 0);    
#endif
#ifdef HAVE_PRESSURE
    eeprom_update_byte((uint8_t *)EE_show_press, 0);    
#endif
#ifdef HAVE_TEMPERATURE
    eeprom_update_byte((uint8_t *)EE_show_temp, 0);
#endif
    eeprom_update_byte((uint8_t *)EE_show_dots, 1);    
    eeprom_update_byte((uint8_t *)EE_brightness, 8);    
    eeprom_update_byte((uint8_t *)EE_volume, 0);    

    eeprom_update_byte((uint8_t *)EE_dateyear, 13);    
    eeprom_update_byte((uint8_t *)EE_datemonth, 1);
    eeprom_update_byte((uint8_t *)EE_dateday, 1);
    eeprom_update_byte((uint8_t *)EE_alarmtype, ALARM_NORMAL);
    eeprom_update_byte((uint8_t *)EE_snooze_enabled, false);

#ifdef HAVE_FLW
    eeprom_update_byte((uint8_t *)EE_flw_enabled, 0);
#endif
#ifdef HAVE_GPS
    eeprom_update_byte((uint8_t *)EE_gps_enabled, 0);    
    eeprom_update_byte((uint8_t *)EE_TZ_hour, 0);    
    eeprom_update_byte((uint8_t *)EE_TZ_minute, 0);    
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
    eeprom_update_byte((uint8_t *)EE_DST_mode, 0);    
    eeprom_update_byte((uint8_t *)EE_DST_offset, 0);
#endif
#ifdef HAVE_AUTO_DATE
    eeprom_update_byte((uint8_t *)EE_date_format, 0);    
    eeprom_update_byte((uint8_t *)EE_Region, 0);    
    eeprom_update_byte((uint8_t *)EE_AutoDate, 0);
#endif
#ifdef HAVE_AUTO_DIM
    eeprom_update_byte((uint8_t *)EE_AutoDim, 0);    
    eeprom_update_byte((uint8_t *)EE_AutoDimHour, 22);  // dim at 10 pm
    eeprom_update_byte((uint8_t *)EE_AutoDimLevel, 2);  // dim to level 2
    eeprom_update_byte((uint8_t *)EE_AutoBrtHour, 7);  // bright at 7 am
    eeprom_update_byte((uint8_t *)EE_AutoBrtLevel, 8);  // bright to level 8
#endif
#ifdef HAVE_AUTO_DST
#ifdef DST_NSW
    eeprom_update_byte((uint8_t *)EE_DST_Rule0, 10);
    eeprom_update_byte((uint8_t *)EE_DST_Rule1, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule2, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule3, 2);
    eeprom_update_byte((uint8_t *)EE_DST_Rule4, 3);
    eeprom_update_byte((uint8_t *)EE_DST_Rule5, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule6, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule7, 2);
    eeprom_update_byte((uint8_t *)EE_DST_Rule8, 1);
#else
    eeprom_update_byte((uint8_t *)EE_DST_Rule0, 3);
    eeprom_update_byte((uint8_t *)EE_DST_Rule1, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule2, 2);
    eeprom_update_byte((uint8_t *)EE_DST_Rule3, 2);
    eeprom_update_byte((uint8_t *)EE_DST_Rule4, 11);
    eeprom_update_byte((uint8_t *)EE_DST_Rule5, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule6, 1);
    eeprom_update_byte((uint8_t *)EE_DST_Rule7, 2);
    eeprom_update_byte((uint8_t *)EE_DST_Rule8, 1);
#endif
#endif
    eeprom_update_byte((uint8_t *)EE_check_byte, EE_CHECK);    
}

void globals_init(void)
{
  uint8_t ee_check = eeprom_read_byte((uint8_t *)EE_check_byte);
  if (ee_check!=EE_CHECK) {
//    Serial.println("clean ee");
    clean_eeprom();
  }

	// read eeprom
	g_24h_clock  = eeprom_read_byte((uint8_t *)EE_24h_clock);
#ifdef HAVE_HUMDITY
	g_show_humid  = eeprom_read_byte((uint8_t *)EE_show_humid);
#endif
#ifdef HAVE_PRESSURE
	g_show_press  = eeprom_read_byte((uint8_t *)EE_show_press);
#endif
#ifdef HAVE_TEMPERATURE
	g_show_temp  = eeprom_read_byte((uint8_t *)EE_show_temp);
#endif
	g_show_dots  = eeprom_read_byte((uint8_t *)EE_show_dots);
	g_brightness = eeprom_read_byte((uint8_t *)EE_brightness);
	g_volume     = eeprom_read_byte((uint8_t *)EE_volume);
#ifdef HAVE_FLW
	g_flw_enabled = eeprom_read_byte((uint8_t *)EE_flw_enabled);
//    Serial.print("g_flw_enabled = ");
//    Serial.println(g_flw_enabled);
#else
        g_flw_enabled = 0;
#endif
#ifdef HAVE_GPS
	g_gps_enabled = eeprom_read_byte((uint8_t *)EE_gps_enabled);
//    Serial.print("g_gps_enabled = ");
//    Serial.println(g_gps_enabled);
	if (g_gps_enabled != 0 && g_gps_enabled != 48 && g_gps_enabled != 96) g_gps_enabled = 0;
	g_TZ_hour = eeprom_read_byte((uint8_t *)EE_TZ_hour) - 12;
	if ((g_TZ_hour<-12) || (g_TZ_hour>12))  g_TZ_hour = 0;  // add range check
	g_TZ_minute = eeprom_read_byte((uint8_t *)EE_TZ_minute);
	if (g_TZ_minute % 15 != 0) g_TZ_minute = 0;
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
	g_DST_mode = eeprom_read_byte((uint8_t *)EE_DST_mode);
#ifdef HAVE_AUTO_DST
	if (g_DST_mode < 0 || g_DST_mode > 2) g_DST_mode = 0;
#else
	if (g_DST_mode < 0 || g_DST_mode > 1) g_DST_mode = 0;
#endif

	g_DST_offset = eeprom_read_byte((uint8_t *)EE_DST_offset);
	g_DST_offset = 1; // use fixed offset for now
	g_DST_updated = false;  // allow automatic DST update

//        Serial.print("g_DST_mode = ");
//        Serial.println(g_DST_mode);
//        Serial.print("g_DST_offset = ");
//        Serial.println(g_DST_offset);

#endif
#ifdef HAVE_AUTO_DATE
	g_date_format = (date_format_t)eeprom_read_byte((uint8_t *)EE_date_format);
	g_AutoDate = eeprom_read_byte((uint8_t *)EE_AutoDate);
#endif
#ifdef HAVE_AUTO_DIM
	g_AutoDim = eeprom_read_byte((uint8_t *)EE_AutoDim);
	g_AutoDimHour = eeprom_read_byte((uint8_t *)EE_AutoDimHour);
	g_AutoDimLevel = eeprom_read_byte((uint8_t *)EE_AutoDimLevel);
	g_AutoBrtHour = eeprom_read_byte((uint8_t *)EE_AutoBrtHour);
	g_AutoBrtLevel = eeprom_read_byte((uint8_t *)EE_AutoBrtLevel);
#endif
	g_dateyear = eeprom_read_byte((uint8_t *)EE_dateyear);
	g_datemonth = eeprom_read_byte((uint8_t *)EE_datemonth);
	g_dateday = eeprom_read_byte((uint8_t *)EE_dateday);
#ifdef HAVE_AUTO_DST
	g_DST_Rules[0] = eeprom_read_byte((uint8_t *)EE_DST_Rule0);  // DST start month
	g_DST_Rules[1] = eeprom_read_byte((uint8_t *)EE_DST_Rule1);  // DST start dotw
	g_DST_Rules[2] = eeprom_read_byte((uint8_t *)EE_DST_Rule2);  // DST start week
	g_DST_Rules[3] = eeprom_read_byte((uint8_t *)EE_DST_Rule3);  // DST start hour
	g_DST_Rules[4] = eeprom_read_byte((uint8_t *)EE_DST_Rule4); // DST end month
	g_DST_Rules[5] = eeprom_read_byte((uint8_t *)EE_DST_Rule5);  // DST end dotw
	g_DST_Rules[6] = eeprom_read_byte((uint8_t *)EE_DST_Rule6);  // DST end week
	g_DST_Rules[7] = eeprom_read_byte((uint8_t *)EE_DST_Rule7);  // DST end hour
	g_DST_Rules[8] = eeprom_read_byte((uint8_t *)EE_DST_Rule8);  // DST offset
#endif
 }
