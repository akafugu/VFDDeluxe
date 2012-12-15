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

#include "global.h"

#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

#include <Wire.h>
#include <WireRtcLib.h>

#include "global_vars.h"
#include "display.h"
#include "gps.h"
#include "menu.h"
#include "time.h"

/*
#include "display.h"
#include "piezo.h"
#include "gps.h"
#include "adst.h"
*/

void beep(uint16_t freq, uint8_t times);
extern WireRtcLib rtc;

menu_state_t g_menu_state;

// 
// This code produces lots of warnings:
// warning: only initialized variables can be placed into program memory area
//
// It appears to be a known issue in avr-g++:
// http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=57011&highlight=warning+initialized
//

//const menu_value menuOffOn[] = { {0, " off"}, {1, "  on"} };
const menu_value PROGMEM menu_gps[] = { {0, " off"}, {48, "  48"}, {96, "  96"} };
//const menu_value PROGMEM menu_gps[] = { {0, { ' ', 'o', 'f', 'f' } }, {48, { ' ', ' ', '4', '8'} }, {96, {' ', ' ', '9', '6'} } };
#if defined HAVE_AUTO_DST
const PROGMEM menu_value menu_adst[] = { {0, " off"}, {1, "  on"}, {2, "auto"} };
#endif
const PROGMEM menu_value menu_volume[] = { {0, "  lo"}, {1, "  hi"}, {NULL, NULL} };
const PROGMEM menu_value menu_region[] = { {0, " dmy"}, {1, " mdy"}, {2, " ymd"} };

//const PROGMEM menu_value menu_none[] = { {NULL, NULL}, {NULL, NULL}, {NULL, NULL} };

const PROGMEM menu_item menu24h = {MENU_24H,menu_offOn,"24H","24H",&g_24h_clock,&b_24h_clock,0,2,{NULL}};
const PROGMEM menu_item menuBrt = {MENU_BRIGHTNESS,menu_num,"BRIT","BRITE",&g_brightness,&b_brightness,0,10,{}};
#ifdef HAVE_AUTO_DATE
const PROGMEM menu_item menuAdate_ = {MENU_AUTODATE,menu_hasSub,"ADT","ADATE",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuAdate = {MENU_AUTODATE,menu_offOn+menu_isSub,"ADTE","ADATE",&g_AutoDate,&b_AutoDate,0,2,{NULL}};
const PROGMEM menu_item menuRegion = {MENU_REGION,menu_list+menu_isSub,"REGN","RGION",&g_Region,&b_Region,0,3,{menu_region}};
#endif
#ifdef HAVE_AUTO_DIM
const PROGMEM menu_item menuAdim_ = {MENU_AUTODIM,menu_hasSub,"ADM","ADIM ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuAdim = {MENU_AUTODIM_ENABLE,menu_offOn+menu_isSub,"ADIM","ADIM",&g_AutoDim,&b_AutoDim,0,2,{NULL}};
const PROGMEM menu_item menuAdimHr = {MENU_AUTODIM_HOUR,menu_num+menu_isSub,"ADMH","ADMH",&g_AutoDimHour,&b_AutoDimHour,0,23,{NULL}};
const PROGMEM menu_item menuAdimLvl = {MENU_AUTODIM_LEVEL,menu_num+menu_isSub,"ADML","ADML",&g_AutoDimLevel,&b_AutoDimLevel,0,10,{NULL}};
const PROGMEM menu_item menuAbrtHr = {MENU_AUTOBRT_HOUR,menu_num+menu_isSub,"ABTH","ABTH",&g_AutoBrtHour,&b_AutoBrtHour,0,23,{NULL}};
const PROGMEM menu_item menuAbrtLvl = {MENU_AUTOBRT_LEVEL,menu_num+menu_isSub,"ABTL","ABTL",&g_AutoBrtLevel,&b_AutoBrtLevel,1,10,{NULL}};
#endif
#ifdef HAVE_SET_DATE						
const PROGMEM menu_item menuDate_ = {MENU_DATE,menu_hasSub,"DAT","DATE ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuYear = {MENU_DATEYEAR,menu_num+menu_isSub,"YEAR","YEAR",&g_dateyear,NULL,10,29,{NULL}};
const PROGMEM menu_item menuMonth = {MENU_DATEMONTH,menu_num+menu_isSub,"MNTH","MONTH",&g_datemonth,NULL,1,12,{NULL}};
const PROGMEM menu_item menuDay = {MENU_DATEDAY,menu_num+menu_isSub,"DAY","DAY",&g_dateday,NULL,1,31,{NULL}};
#endif
const PROGMEM menu_item menuDots = {MENU_DOTS,menu_offOn,"DOTS","DOTS ",&g_show_dots,&b_show_dots,0,1,{}};
#ifdef HAVE_AUTO_DST
const PROGMEM menu_item menuDST_ = {MENU_DST,menu_hasSub,"DST","DST  ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuDST = {MENU_DST_ENABLE,menu_list+menu_isSub,"DST","DST",&g_DST_mode,&b_DST_mode,0,3,{menu_adst}};
#elif defined HAVE_GPS
const PROGMEM menu_item menuDST = {MENU_DST_ENABLE,menu_offOn,"DST","DST",&g_DST_mode,&b_DST_mode,0,2,{NULL}};
#endif
#ifdef HAVE_AUTO_DST
const PROGMEM menu_item menuRules = {MENU_RULES,menu_hasSub+menu_isSub,"RUL","RULES",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuRule0 = {MENU_RULE0,menu_num+menu_isSub,"RUL0","RULE0",&g_DST_Rules[0],&b_DST_Rule0,1,12,{NULL}};
const PROGMEM menu_item menuRule1 = {MENU_RULE1,menu_num+menu_isSub,"RUL1","RULE1",&g_DST_Rules[1],&b_DST_Rule1,1,7,{NULL}};
const PROGMEM menu_item menuRule2 = {MENU_RULE2,menu_num+menu_isSub,"RUL2","RULE2",&g_DST_Rules[2],&b_DST_Rule2,1,5,{NULL}};
const PROGMEM menu_item menuRule3 = {MENU_RULE3,menu_num+menu_isSub,"RUL3","RULE3",&g_DST_Rules[3],&b_DST_Rule3,0,23,{NULL}};
const PROGMEM menu_item menuRule4 = {MENU_RULE4,menu_num+menu_isSub,"RUL4","RULE4",&g_DST_Rules[4],&b_DST_Rule4,1,12,{NULL}};
const PROGMEM menu_item menuRule5 = {MENU_RULE5,menu_num+menu_isSub,"RUL5","RULE5",&g_DST_Rules[5],&b_DST_Rule5,1,7,{NULL}};
const PROGMEM menu_item menuRule6 = {MENU_RULE6,menu_num+menu_isSub,"RUL6","RULE6",&g_DST_Rules[6],&b_DST_Rule6,1,5,{NULL}};
const PROGMEM menu_item menuRule7 = {MENU_RULE7,menu_num+menu_isSub,"RUL7","RULE7",&g_DST_Rules[7],&b_DST_Rule7,0,23,{NULL}};
const PROGMEM menu_item menuRule8 = {MENU_RULE8,menu_num+menu_isSub,"RUL8","RULE8",&g_DST_Rules[8],&b_DST_Rule8,1,1,{NULL}};  // offset can't be changed
#endif
#if defined HAVE_FLW
const PROGMEM menu_item menuFLW = {MENU_FLW,menu_offOn,"FLW","FLW",&g_flw_enabled,&b_flw_enabled,0,2,{NULL}};
#endif
#if defined HAVE_GPS
const PROGMEM menu_item menuGPS_ = {MENU_GPS,menu_hasSub,"GPS","GPS  ",NULL,NULL,0,0,{}};
const PROGMEM menu_item menuGPS = {MENU_GPS_ENABLE,menu_list+menu_isSub,"GPS","GPS",&g_gps_enabled,&b_gps_enabled,0,3,{menu_gps}};
const PROGMEM menu_item menuTZh = {MENU_TZH,menu_num+menu_isSub,"TZH","TZ-H",&g_TZ_hour,&b_TZ_hour,-12,12,{}};
const PROGMEM menu_item menuTZm = {MENU_TZM,menu_num+menu_isSub,"TZM","TZ-M",&g_TZ_minute,&b_TZ_minute,0,59,{}};
#endif
#if defined HAVE_GPS_DEBUG
const PROGMEM menu_item menuGPSc = {MENU_GPSC,menu_num+menu_isSub,"GPSC","GPSC",&g_gps_cks_errors,NULL,0,0,NULL};
const PROGMEM menu_item menuGPSp = {MENU_GPSP,menu_num+menu_isSub,"GPSP","GPSP",&g_gps_parse_errors,NULL,0,0,NULL};
const PROGMEM menu_item menuGPSt = {MENU_GPST,menu_num+menu_isSub,"GPST","GPST",&g_gps_time_errors,NULL,0,0,NULL};
#endif
const PROGMEM menu_item menuTemp = {MENU_TEMP,menu_offOn,"TEMP","TEMP",&g_show_temp,&b_show_temp,0,2,{NULL}};
const PROGMEM menu_item menuVol = {MENU_VOL,menu_list,"VOL","VOL",&g_volume,&b_volume,0,2,{menu_volume}};

const PROGMEM menu_item* const menuItems[] = { 
//const PROGMEM menu_item* PROGMEM const menuItems[] = { 
	&menu24h, 
#ifdef HAVE_AUTO_DATE
	&menuAdate_, &menuAdate, &menuRegion,
#endif
#ifdef HAVE_AUTO_DIM
	&menuAdim_,	&menuAdim, &menuAdimHr, &menuAdimLvl, &menuAbrtHr, &menuAbrtLvl,
#endif
	&menuBrt, 
#ifdef HAVE_SET_DATE
	&menuDate_, &menuYear, &menuMonth, &menuDay,
#endif
	&menuDots,
#if defined HAVE_AUTO_DST
	&menuDST_, &menuDST,
#elif defined HAVE_GPS
	&menuDST,
#endif
#ifdef HAVE_AUTO_DST
	&menuRules,	
	&menuRule0, &menuRule1, &menuRule2, &menuRule3, &menuRule4, &menuRule5, &menuRule6, &menuRule7, &menuRule8,
#endif
#ifdef HAVE_FLW
	&menuFLW,
#endif
#if defined HAVE_GPS
	&menuGPS_, &menuGPS, &menuTZh, &menuTZm,
#endif
#if defined HAVE_GPS_DEBUG
	&menuGPSc, &menuGPSp, &menuGPSt,
#endif
	&menuTemp,
	&menuVol,
	NULL};  // end of list marker must be here

static uint8_t menu_disabled[MENU_END];

extern WireRtcLib::tm* tt; // current local date and time as TimeElements (pointer)

#if defined HAVE_GPS || defined HAVE_AUTO_DST
void setDSToffset(uint8_t mode) {
	int8_t adjOffset;
	uint8_t newOffset;
#ifdef HAVE_AUTO_DST
	if (mode == 2) {  // Auto DST
		if (g_DST_updated) return;  // already done it once today
		if (tt == NULL) return;  // safety check
		newOffset = getDSToffset(tt, g_DST_Rules);  // get current DST offset based on DST Rules
	}
	else
#endif // HAVE_AUTO_DST
		newOffset = mode;  // 0 or 1
	adjOffset = newOffset - g_DST_offset;  // offset delta
	if (adjOffset == 0)  return;  // nothing to do
	if (adjOffset > 0)
		beep(880, 1);  // spring ahead
	else
		beep(440, 1);  // fall back

	time_t tNow = 0; // fixme rtc_get_time_t();  // fetch current time from RTC as time_t

	tNow += adjOffset * SECS_PER_HOUR;  // add or subtract new DST offset

	//rtc_set_time_t(tNow);  // adjust RTC

	g_DST_offset = newOffset;
	//eeprom_update_byte(&b_DST_offset, g_DST_offset);
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
	//rtc_set_time(tm_);
#ifdef HAVE_AUTO_DST
	DSTinit(tm_, g_DST_Rules);  // re-compute DST start, end for new date
	g_DST_updated = false;  // allow automatic DST adjustment again
	setDSToffset(g_DST_mode);  // set DSToffset based on new date
#endif
}
#endif

void menu_action(menu_item * menuPtr)
{
	switch(menuPtr->menuNum) {
		case MENU_BRIGHTNESS:
			set_brightness(*menuPtr->setting);
			break;
		case MENU_VOL:
			//piezo_init();
			//beep(1000, 1);
			break;
		case MENU_DATEYEAR:
		case MENU_DATEMONTH:
		case MENU_DATEDAY:
			//set_date(g_dateyear, g_datemonth, g_dateday);
			break;
		case MENU_GPS_ENABLE:
			gps_init(g_gps_enabled);  // change baud rate
			break;
		case MENU_RULE0:
		case MENU_RULE1:
		case MENU_RULE2:
		case MENU_RULE3:
		case MENU_RULE4:
		case MENU_RULE5:
		case MENU_RULE6:
		case MENU_RULE7:
		case MENU_RULE8:
		case MENU_DST_ENABLE:
			//g_DST_updated = false;  // allow automatic DST adjustment again
			//DSTinit(tm_, g_DST_Rules);  // re-compute DST start, end for new data
			//setDSToffset(g_DST_mode);
			break;
		case MENU_TZH:
		case MENU_TZM:
			tGPSupdate = 0;  // allow GPS to refresh
			break;
	}
}

volatile uint8_t menuIdx = 0;
uint8_t update = false;  // right button updates value?
uint8_t show = false;  // show value?

static menu_item_rw current_item;
menu_item* getItem(uint8_t idx)
{
	menu_item* mPtr = (menu_item*)pgm_read_word(&menuItems[idx]);  // address of current menu item
	if (mPtr == NULL)  return(NULL);
//	memcpy_P(&current_item, &mPtr, sizeof(current_item));
	current_item.menuNum = pgm_read_byte(&mPtr->menuNum);
	current_item.flags = pgm_read_byte(&mPtr->flags);
	strncpy_P(current_item.shortName,(char *)&mPtr->shortName,4); 
	strncpy_P(current_item.longName,(char *)&mPtr->longName,5); 
	current_item.setting = (int8_t *)pgm_read_word(&mPtr->setting);
	current_item.eeAddress = (uint8_t*)pgm_read_word(&mPtr->eeAddress);
	current_item.loLimit = pgm_read_byte(&mPtr->loLimit);
	current_item.hiLimit = pgm_read_byte(&mPtr->hiLimit);
	*current_item.menuList = (menu_value*)pgm_read_word(&mPtr->menuList);
	return (menu_item*)&current_item;
}

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

menu_item * nextItem(uint8_t skipSub)  // next menu item
{
//	menu_item * mPtr = (menu_item*)pgm_read_word(&menuItems[menuIdx]);  // current menu item
	menu_item * mPtr = getItem(menuIdx);  // current menu item
	uint8_t inSub = (mPtr->flags & menu_isSub) && !(mPtr->flags & menu_hasSub);  // are we in a sub menu now?
//	mPtr = (menu_item*)pgm_read_word(&menuItems[++menuIdx]);  // next menu item
	mPtr = getItem(++menuIdx);  // next menu item
//	while ((menuPtr != NULL) && ((menuPtr->flags & menu_disabled) || (skipSub && (menuPtr->flags & menu_isSub) && !inSub)) ) { 
	while ((mPtr != NULL) && ((menu_disabled[menuIdx]) || (skipSub && (mPtr->flags & menu_isSub) && !inSub)) ) { 
//		mPtr = (menu_item*)pgm_read_word(&menuItems[++menuIdx]);  // next menu item
		mPtr = getItem(++menuIdx);  // next menu item
	}
	return mPtr;
}

void menu(uint8_t btn)
{
//	menu_item * menuPtr = (menu_item*)pgm_read_word(&menuItems[menuIdx]);  // current menu item
	menu_item * menuPtr = getItem(menuIdx);  // next menu item
	uint8_t digits = get_digits();
//	tick();
	switch (btn) {
		case 0:  // start at top of menu
			menuIdx = 0;  // restart menu
//			menuPtr = (menu_item*)pgm_read_word(&menuItems[menuIdx]);
			menuPtr = getItem(menuIdx);  // next menu item
			update = false;
			show = false;
			break;
		case 1:  // right button - show/update current item value
			if (menuPtr->flags & menu_hasSub) {
				menuPtr = nextItem(false);  // show first submenu item
				update = false;
			}
			else {
				show = true;  // show value
				if (digits>6)
					update = true;
			}
			break;
		case 2:  // left button - show next menu item
			menuPtr = nextItem(true);  // next menu items (skip subs)
			update = false;
			show = false;
			break;
	}
	if (menuPtr == NULL) {  // check for end of menu
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
//	menu_value* menuValues = (menu_value*)pgm_read_word(&menuPtr->menuList);  //get pointer to menu values
	volatile uint8_t idx = 0;
// numeric menu item
	if (menuPtr->flags & menu_num) {
			if (update) {
				valNum++;
				if (valNum > menuPtr->hiLimit)
					valNum = menuPtr->loLimit;
				*menuPtr->setting = valNum;
				if (menuPtr->eeAddress != NULL) {
					//if (menuPtr->menuNum == MENU_TZH)
					//	eeprom_update_byte(menuPtr->eeAddress, valNum+12);
					//else
					//	eeprom_update_byte(menuPtr->eeAddress, valNum);
				}
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
				//if (menuPtr->eeAddress != NULL) 
				//	eeprom_update_byte(menuPtr->eeAddress, valNum);
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
//				if (menuPtr->menuList[i]->value == valNum) {
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
				//if (menuPtr->eeAddress != NULL) 
				//	eeprom_update_byte(menuPtr->eeAddress, valNum);
				menu_action(menuPtr);
			}
			show_setting_string(shortName, longName, valStr, show);
			if (show)
				update = true;
			else
				show = true;
		}
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

