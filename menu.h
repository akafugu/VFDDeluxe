/*
 * Menu for VFD Modular Clock
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
#ifndef MENU_H_
#define MENU_H_

// menu states
typedef enum {
    // basic states
    STATE_CLOCK = 0,
    STATE_SET_CLOCK,
    STATE_SET_ALARM,
    // menu
    STATE_MENU_BRIGHTNESS,
    STATE_MENU_24H,
    STATE_MENU_YEAR,
    STATE_MENU_MONTH,
    STATE_MENU_DAY,
    STATE_MENU_AUTODATE,
#if defined(HAVE_AUTO_DST) || defined(HAVE_GPS)
    STATE_MENU_DST,
#endif
    STATE_MENU_REGION,
#ifdef HAVE_GPS
    STATE_MENU_GPS,
    STATE_MENU_ZONEH,
    STATE_MENU_ZONEM,
#endif
    STATE_MENU_TEMP,
    STATE_MENU_DOTS,
#ifdef HAVE_FLW
    STATE_MENU_FLW,
#endif
    STATE_MENU_LAST,
} menu_state_t;

extern menu_state_t g_menu_state;

#if defined HAVE_GPS || defined HAVE_AUTO_DST
void setDSToffset(uint8_t mode);
#endif
void menu_init(void);

void menu_enable(menu_state_t item, bool enabled);
void menu(bool update, bool show);

void first_menu_item();
void next_menu_item();

#endif
