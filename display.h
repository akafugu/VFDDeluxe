/*
 * VFD Deluxe - Firmware for VFD Modular Clock mk2
 * (C) 2011-13 Akafugu Corporation
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

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "features.h"
#include <stdbool.h>
#include <avr/io.h>

#include <WireRtcLib.h>

// Shield signature
#define SIGNATURE_PORT  PORTD
#define SIGNATURE_DDR   DDRD
#define SIGNATURE_PIN   PIND
#define SIGNATURE_BIT_0 PD3
#define SIGNATURE_BIT_1 PD4
#define SIGNATURE_BIT_2 PD5

unsigned long wMillis(void);
void wDelay(unsigned long ms);

void display_init(uint8_t data, uint8_t clock, uint8_t latch, uint8_t blank, uint8_t brightness);
int get_digits(void);

// functions for showing current time and temperature
void show_time(WireRtcLib::tm* t, bool _24h_clock, uint8_t mode);
void show_time_setting(uint8_t hour, uint8_t min, uint8_t sec);
void show_temp(int8_t t, uint8_t f);
void show_humidity(uint8_t hum);
void show_pressure(uint8_t pressure);
void scroll_date(WireRtcLib::tm* te, uint8_t region);
void scroll_stop(void);
uint8_t scrolling(void);
void scroll_speed(uint16_t speed);

// gps integration
void set_gps_updated(bool b);

// functions for showing settings
void show_setting_string(const char* short_str, const char* long_str, const char* value, bool show_setting);
void show_setting_int(const char* short_str, const char* long_str, int value, bool show_setting);
void show_set_time(void);
void show_set_alarm(void);

void set_string(const char* str, uint8_t offset = 0);
void set_scroll(char* str);
void set_char_at(char c, uint8_t offset);

void set_brightness(uint8_t brightness);

void set_blink(bool on);
void set_display(bool on);
void flash_display(uint16_t ms);

void set_shield(shield_t shield_type, uint8_t digits = 4);
void detect_shield();

#endif // DISPLAY_H_
