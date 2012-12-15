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
#include <avr/pgmspace.h>

#ifdef __FLASH
#define FLASH __flash
#else
#define FLASH
#endif

typedef unsigned char menu_flags;

const uint8_t menu_noflags = 0x00;
const uint8_t menu_num = 0x01;
const uint8_t menu_offOn = 0x02;  
const uint8_t menu_list = 0x04;
const uint8_t menu_hasSub = 0x10;
const uint8_t menu_isSub = 0x20;
//const uint8_t  menu_disabled = 0x80,

/*
typedef enum {
	menu_noflags = 0x00,
	menu_num = 0x01,
	menu_offOn = 0x02,  
	menu_list = 0x04,
	menu_hasSub = 0x10,
	menu_isSub = 0x20,
//	menu_disabled = 0x80,
} menu_flags;
*/

struct menu_value {
	const uint8_t value;  // list of possible values in order
	const char valName[5];  // list of value names for display
};

struct menu_item {
	const uint8_t menuNum;  // menu item number
	menu_flags flags;  // flags
	const char shortName[5];
	const char longName[6];
	int8_t* setting;
	uint8_t* eeAddress;
	const int8_t loLimit;  // low limit for num
	const int8_t hiLimit;  // high limit for num, # of values for list
	const menu_value* menuList[3];  // list of menu choices
	//const menu_value** menuList;
};

struct menu_item_rw {
	uint8_t menuNum;  // menu item number
	menu_flags flags;  // flags
	char shortName[4];
	char longName[5];
	int8_t* setting;
	uint8_t* eeAddress;
	int8_t loLimit;  // low limit for num
	int8_t hiLimit;  // high limit for num, # of values for list
	menu_value* menuList[];  // list of menu choices
} ;

// menu states
typedef enum {
	// basic states
	STATE_CLOCK = 0,  // show clock time
	STATE_SET_CLOCK,
	STATE_SET_ALARM,
	// menu
	STATE_MENU,
	STATE_MENU_LAST,
} menu_state_t;

typedef enum {  // no ifdefs, no harm in defining unused items
	MENU_24H = 0,
	MENU_AUTODATE,
	MENU_AUTODATE_ENABLE,
	MENU_AUTODIM,
	MENU_AUTODIM_ENABLE,
	MENU_AUTODIM_HOUR,
	MENU_AUTODIM_LEVEL,
	MENU_AUTOBRT_HOUR,
	MENU_AUTOBRT_LEVEL,
	MENU_BRIGHTNESS,
	MENU_DATE,
	MENU_DATEYEAR,
	MENU_DATEMONTH,
	MENU_DATEDAY,
	MENU_DOTS,
	MENU_FLW,
	MENU_GPS,
	MENU_GPS_ENABLE,
	MENU_TZH,
	MENU_TZM,
	MENU_GPSC,  // GPS error counter
	MENU_GPSP,  // GPS error counter
	MENU_GPST,  // GPS error counter
	MENU_DST,
	MENU_DST_ENABLE,
	MENU_REGION,
	MENU_RULES,
	MENU_RULE0,
	MENU_RULE1,
	MENU_RULE2,
	MENU_RULE3,
	MENU_RULE4,
	MENU_RULE5,
	MENU_RULE6,
	MENU_RULE7,
	MENU_RULE8,
	MENU_TEMP,
	MENU_VOL,
	MENU_END  // must be last
} menu_number;

menu_state_t menu_state;
//uint8_t menuDisabled[MENU_COUNT];

#if defined FEATURE_WmGPS || defined FEATURE_AUTO_DST
void setDSToffset(uint8_t mode);
#endif
void menu_init(void);
void menu(uint8_t n);

#endif
