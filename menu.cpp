/*
 * Menu for VFD Modular Clock
 * (C) 2012-2013 William B Phelps
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

#include "global.h"

//#include <util/delay.h>
//#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

#include <Wire.h>
#include <WireRtcLib.h>

#include "global_vars.h"
#include "display.h"
#include "gps.h"
#include "menu.h"
#include "time.h"
#define time_t unsigned long
#include "adst.h"

//void beep(uint16_t freq, uint8_t times);
extern WireRtcLib rtc;

menu_state_t g_menu_state;
extern WireRtcLib::tm* tt; // current local date and time as TimeElements (pointer)

#if defined HAVE_GPS || defined HAVE_AUTO_DST
void setDSToffset(uint8_t mode) {
	int8_t adjOffset;
	uint8_t newOffset;
#ifdef HAVE_AUTO_DST
	if (mode == 2) {  // Auto DST
		if (g_DST_updated) return;  // already done it once today
		if (tt == NULL) return;  // safety check
		newOffset = getDSToffset(tt, globals.DST_Rules);  // get current DST offset based on DST Rules
	}
	else
#endif // HAVE_AUTO_DST
		newOffset = mode;  // 0 or 1
	adjOffset = newOffset - globals.DST_offset;  // offset delta
	if (adjOffset == 0)  return;  // nothing to do
	if (adjOffset > 0)
		tone(PinMap::piezo, 1760, 100);  // spring ahead
	else
		tone(PinMap::piezo, 880, 100);  // fall back
	tt = rtc.getTime();  // fetch current time from RTC as time_t
	
//	tNow += adjOffset * SECS_PER_HOUR;  // add or subtract new DST offset
// we rely on the fact that the time change always occurs at a "safe" time, when the date won't change...
// there may be some locations on the planet where this is not true...
	tt->hour += adjOffset;
	rtc.setTime(tt);  // adjust RTC
	
	globals.DST_offset = newOffset;
//	save_globals();
	g_DST_updated = true;
	// save DST_updated in ee ???
}
#endif

#ifdef HAVE_SET_DATE
void set_date(uint8_t yy, uint8_t mm, uint8_t dd) {
	tt = rtc.getTime();  // refresh current time 
	tt->year = yy;
	tt->mon = mm;
	tt->mday = dd;
	rtc.setTime(tt);
#ifdef HAVE_AUTO_DST
	DSTinit(tt, globals.DST_Rules);  // re-compute DST start, end for new date
	g_DST_updated = false;  // allow automatic DST adjustment again
	setDSToffset(globals.DST_mode);  // set DSToffset based on new date
#endif
}
#endif

void menu_action(menu_item * menuPtr)
{
//    Serial.println("menu_action");
    
	switch(menuPtr->menuNum) {
		case MENU_ALARMHOUR:
		case MENU_ALARMMINUTE:
			rtc.setAlarm_s(alarm_hour, alarm_minute, 0);
			break;
		case MENU_BRIGHTNESS:
//			set_brightness(*menuPtr->setting);
      set_brightness(globals.brightness);
			break;
		case MENU_VOL:
			//piezo_init();
			//beep(1000, 1);
		  tone(PinMap::piezo, 1000, 200);
			break;
#ifdef HAVE_SET_DATE						
		case MENU_DATEYEAR:
		case MENU_DATEMONTH:
		case MENU_DATEDAY:
			set_date(globals.dateyear, globals.datemonth, globals.dateday);
			break;
#endif
#ifdef HAVE_AUTO_DST
		case MENU_RULE0:
		case MENU_RULE1:
		case MENU_RULE2:
		case MENU_RULE3:
		case MENU_RULE4:
		case MENU_RULE5:
		case MENU_RULE6:
		case MENU_RULE7:
		case MENU_RULE8:
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
		case MENU_DST_ENABLE:
			g_DST_updated = false;  // allow automatic DST adjustment again
			DSTinit(tt, globals.DST_Rules);  // re-compute DST start, end for new data
			setDSToffset(globals.DST_mode);
			break;
#endif		
#if defined HAVE_GPS
		case MENU_GPS_ENABLE:
			gps_init(globals.gps_enabled);  // change baud rate
			break;
		case MENU_TZH:
		case MENU_TZM:
			tGPSupdate = 0;  // allow GPS to refresh
			break;
#endif
		case MENU_TIMEHOUR:
		case MENU_TIMEMINUTE:
			rtc.setTime_s(time_hour, time_minute, 0);
			break;
	}
}

uint8_t menuIdx = 0;
uint8_t update = false;  // right button updates value?
uint8_t show = false;  // show value?

void menu_enable(menu_number num, uint8_t enable)
{
	uint8_t idx = 0;
//	menu_item * mPtr = (menu_item*) pgm_read_word(&menuItems[0]);  // start with first menu item
	menu_item * mPtr = getItem(0);  // current menu item
	while(mPtr != NULL) {
		if (mPtr->menuNum == num) {
			menu_disabled[idx] = !enable;  // default is enabled
			return;  // there should only be one item that matches
		}
		mPtr = getItem(++idx);  // fetch next menu item
	}
}

void menu(uint8_t btn)
{
uint8_t hour, min, sec;
//    Serial.print("menu("); Serial.print(btn); Serial.println(")");
	menu_item * menuPtr = getItem(menuIdx);  // next menu item
	uint8_t digits = get_digits();
//	tick();
	switch (btn) {
		case 0:  // start at top of menu
//											  Serial.println("restart menu");
			menuIdx = 0;  // restart menu
//			menuPtr = (menu_item*)pgm_read_word(&menuItems[menuIdx]);
			menuPtr = getItem(0);  // next menu item
			update = false;
			show = false;
			break;
		case 1:  // right button - show/update current item value
//                        Serial.println("right button");
			if (menuPtr->flags & menu_hasSub) {
//                                Serial.println("has sub");
				menuIdx = nextItem(menuIdx, false);  // show first submenu item
				menuPtr = getItem(menuIdx);
				update = false;
			}
			else {
//                                Serial.println("show value");
				show = true;  // show value
				if (digits>6)
					update = true;
			}
			break;
		case 2:  // left button - show next menu item
//                        Serial.println("left button");
			menuIdx = nextItem(menuIdx, true);  // next menu items (skip subs)
			menuPtr = getItem(menuIdx);
			update = false;
			show = false;
			if (menuPtr->flags & menu_time) { // time item?
				if (menuPtr->menuNum == MENU_ALARM)
					rtc.getAlarm_s(&alarm_hour, &alarm_minute, &alarm_second);
				if (menuPtr->menuNum == MENU_TIME)
					rtc.getTime_s(&time_hour, &time_minute, &time_second);
			}
			break;
	}
//        Serial.print("menuIdx "); Serial.println(menuIdx,DEC);
	if (menuPtr == NULL) {  // check for end of menu
//                Serial.println("end of menu reached");
		menuIdx = 0;
		update = false;
		g_menu_state = STATE_CLOCK;
		return;
	}
	char shortName[5];
	char longName[8];
	strncpy(shortName,menuPtr->shortName,4);
	shortName[4] = '\0';  // null terminate string
	strncpy(longName,menuPtr->longName,5);
	longName[5] = '\0';  // null terminate string
	int valNum = *(menuPtr->setting);
	char valStr[5] = "";  // item value name for display ("off", "on", etc)
	const menu_value * menuValues = *(menuPtr->menuList);  //get pointer to menu values
	volatile uint8_t idx = 0;
// numeric menu item
	if (menuPtr->flags & menu_num) {
			if (update) {
				valNum++;
				if (valNum > menuPtr->hiLimit)
					valNum = menuPtr->loLimit;
				*menuPtr->setting = valNum;
//				save_globals();
				menu_action(menuPtr);
			}
			show_setting_int(shortName, longName, valNum, show);
			if (show)
				update = true;
			else
				show = true;
		}
// off/on menu item 
	else if (menuPtr->flags & menu_offOn) {
			if (update) {
				valNum = !valNum;
				*menuPtr->setting = valNum;
//				save_globals();
				menu_action(menuPtr);
			}
			if (valNum)
				show_setting_string(shortName, longName, "  on", show);
			else
				show_setting_string(shortName, longName, " off", show);
			if (show)
				update = true;
			else
				show = true;
		}
// list menu item
		else if (menuPtr->flags & menu_list) {
			idx = 0;
			for (uint8_t i=0;i<menuPtr->hiLimit;i++) {  // search for the current item's value in the list
				if (pgm_read_byte(&menuValues[i].value) == valNum) {
					idx = i;
					}
			}
			strncpy_P(valStr,(char *)&menuValues[idx].valName,4);  // item name
			valStr[4] = '\0';  // null terminate string
			if (update) {
				idx++;  // next item in list
				if (idx >= menuPtr->hiLimit)  // for lists, hilimit is the # of elements! 
					idx = 0;  // wrap
				valNum = pgm_read_byte(&menuValues[idx].value);
				strncpy_P(valStr,(char *)&menuValues[idx].valName,4);  // item name
				valStr[4] = '\0';  // null terminate string
				*menuPtr->setting = valNum;
//				save_globals();
				menu_action(menuPtr);
			}
			show_setting_string(shortName, longName, valStr, show);
			if (show)
				update = true;
			else
				show = true;
		}
//#ifdef HAVE_MENU_TIME
// time menu item
		// else if (menuPtr->flags & menu_time) {
			// if (show) {
				// if (menuPtr->menuNum == MENU_ALARM) {
					// rtc.getAlarm_s(&hour, &min, &sec);
					// g_time_to_set = hour*60 + min;
					// if (update)
						// menu_action(menuPtr);
				// }
				// if (menuPtr->menuNum == MENU_TIME) {
					// rtc.getTime_s(&hour, &min, &sec);
					// g_time_to_set = hour*60 + min;
					// if (update)
						// menu_action(menuPtr);
				// }
				// show_time_setting(g_time_to_set / 60, g_time_to_set % 60, 0);
				// update = true;
			// }
			// else {
				// show_setting_string(shortName, longName, valStr, false);
			// }	
		// }
//#endif
// top of sub menu item
		else if (menuPtr->flags & menu_hasSub) {
			switch (digits) {
				case 4:
					strcat(shortName, "*");  // indicate top of sub
					show_setting_string(shortName, longName, valStr, false);
					break;
				case 6:
					strcat(longName, "-");  // indicate top of sub
					show_setting_string(shortName, longName, valStr, false);
					break;
				case 8:  // use longName instead of shortName for top menu item
					strcat(longName, " -");
					show_setting_string(longName, longName, valStr, false);
					break;
			}
		}
}  // menu

void menu_init(void)
{
	g_menu_state = STATE_CLOCK;
	menu_enable(MENU_TEMP, rtc.isDS3231());  // show temperature setting only when running on a DS3231
	menu_enable(MENU_DOTS, g_has_dots);  // don't show dots settings for shields that have no dots
#ifdef HAVE_FLW
	menu_enable(MENU_FLW, g_has_flw);  // don't show FLW settings when there is no EEPROM with database
#endif
}

