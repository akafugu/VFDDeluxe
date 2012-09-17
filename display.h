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

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdbool.h>
#include <avr/io.h>

// HV5812 Data In (PF7 - A0)
#define DATA_BIT PORTF7
#define DATA_PORT PORTF
#define DATA_DDR DDRF
#define DATA_HIGH DATA_PORT |= _BV(DATA_BIT)
#define DATA_LOW DATA_PORT &= ~(_BV(DATA_BIT))

// HV5812 Clock (PF5 - A2)
#define CLOCK_BIT PORTF5
#define CLOCK_PORT PORTF
#define CLOCK_DDR DDRF
#define CLOCK_HIGH CLOCK_PORT |= _BV(CLOCK_BIT)
#define CLOCK_LOW CLOCK_PORT &= ~(_BV(CLOCK_BIT))

// HV5812 Latch / Strobe (PF6 - A1)
#define LATCH_BIT PORTF6
#define LATCH_PORT PORTF
#define LATCH_DDR DDRF
#define LATCH_HIGH LATCH_PORT |= _BV(LATCH_BIT)
#define LATCH_LOW LATCH_PORT &= ~(_BV(LATCH_BIT))

#define LATCH_ENABLE LATCH_LOW
#define LATCH_DISABLE LATCH_HIGH

// HV5812 Blank (PB5 - D9)
#define BLANK_BIT PORTB5
#define BLANK_PORT PORTB
#define BLANK_DDR DDRB
#define BLANK_HIGH BLANK_PORT |= _BV(BLANK_BIT)
#define BLANK_LOW BLANK_PORT &= ~(_BV(BLANK_BIT))

#include <WireRtcLib.h>

void display_init(uint8_t brightness);
int get_digits(void);
void detect_shield(void);

// functions for showing current time and temperature
void show_time(WireRtcLib::tm* t, bool _24h_clock, uint8_t mode);
void show_time_setting(uint8_t hour, uint8_t min, uint8_t sec);
void show_temp(int8_t t, uint8_t f);

// functions for showing settings
void show_setting_string(const char* short_str, const char* long_str, const char* value, bool show_setting);
void show_setting_int(const char* short_str, const char* long_str, int value, bool show_setting);
void show_set_time(void);
void show_set_alarm(void);

void set_string(const char* str);
void set_char_at(char c, uint8_t offset);

void set_brightness(uint8_t brightness);

void set_blink(bool on);

enum shield_t {
	SHIELD_NONE = 0,
	SHIELD_7SEG,
	SHIELD_IV6,
	SHIELD_IV17,
	SHIELD_IV18,
	SHIELD_IV22,
};

#endif // DISPLAY_H_
