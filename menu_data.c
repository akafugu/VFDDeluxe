//#define FEATURE_GPS_DEBUG  // enables GPS debugging counters & menu items
//#define FEATURE_AUTO_DIM  // moved to Makefile

#include <arduino.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

#include "features.h"
#include "global_vars.h"
#include "menu_data.h"

menu_state_t menu_state;
uint8_t alarm_hour, alarm_minute, alarm_second;
uint8_t time_hour, time_minute, time_second;

// MENU VALUES
//const menu_value menuOffOn[] = { {0, " off"}, {1, "  on"} };
const menu_value PROGMEM menu_gps[] = { {0, " off"}, {48, "  48"}, {96, "  96"} };
#if defined HAVE_AUTO_DST
const PROGMEM menu_value menu_adst[] = { {0, " off"}, {1, "  on"}, {2, "auto"} };
#endif
const PROGMEM menu_value menu_volume[] = { {0, "  lo"}, {1, "  hi"} };
const PROGMEM menu_value menu_region[] = { {0, " ymd"}, {1, " dmy"}, {2, " mdy"} };

// MENU ITEMS
//const PROGMEM menu_item menu24h = {MENU_24H,menu_offOn,"24H","24H",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
const PROGMEM menu_item menu24h = {MENU_24H,menu_offOn,"24H","24H",&globals.clock_24h,0,2,{NULL}};
#ifdef HAVE_MENU_TIME
const PROGMEM menu_item menuAlarm = {MENU_ALARM,menu_hasSub+menu_time,"ALRM","ALARM",NULL,0,0,{NULL}};
const PROGMEM menu_item menuAlarmHr = {MENU_ALARMHOUR,menu_num+menu_isSub,"HOUR","HOUR",&alarm_hour,0,23,{NULL}};
const PROGMEM menu_item menuAlarmMin = {MENU_ALARMMINUTE,menu_num+menu_isSub,"MIN","MIN ",&alarm_minute,0,59,{NULL}};
#endif
const PROGMEM menu_item menuBrt = {MENU_BRIGHTNESS,menu_num,"BRIT","BRITE",&globals.brightness,0,10,{NULL}};
#ifdef HAVE_AUTO_DATE
const PROGMEM menu_item menuAdate_ = {MENU_AUTODATE,menu_hasSub,"ADT","ADATE",NULL,0,0,{NULL}};
const PROGMEM menu_item menuAdate = {MENU_AUTODATE,menu_offOn+menu_isSub,"ADTE","ADATE",&globals.AutoDate,0,2,{NULL}};
const PROGMEM menu_item menuRegion = {MENU_REGION,menu_list+menu_isSub,"REGN","RGION",&globals.date_format,0,3,{menu_region}};
#endif
#ifdef HAVE_AUTO_DIM
const PROGMEM menu_item menuAdim_ = {MENU_AUTODIM,menu_hasSub,"ADM","ADIM ",NULL,0,0,{NULL}};
const PROGMEM menu_item menuAdim = {MENU_AUTODIM_ENABLE,menu_offOn+menu_isSub,"ADIM","ADIM",&globals.AutoDim,0,2,{NULL}};
const PROGMEM menu_item menuAdimHr1 = {MENU_AUTODIM_HOUR1,menu_num+menu_isSub,"ADH1","ADH1",&globals.AutoDimHour1,0,23,{NULL}};
const PROGMEM menu_item menuAdimLvl1 = {MENU_AUTODIM_LEVEL1,menu_num+menu_isSub,"ADL1","ADL1",&globals.AutoDimLevel1,0,10,{NULL}};
const PROGMEM menu_item menuAdimHr2 = {MENU_AUTODIM_HOUR2,menu_num+menu_isSub,"ADH2","ADH2",&globals.AutoDimHour2,0,23,{NULL}};
const PROGMEM menu_item menuAdimLvl2 = {MENU_AUTODIM_LEVEL2,menu_num+menu_isSub,"ADL2","ADL2",&globals.AutoDimLevel2,0,10,{NULL}};
const PROGMEM menu_item menuAdimHr3 = {MENU_AUTODIM_HOUR3,menu_num+menu_isSub,"ADH3","ADH3",&globals.AutoDimHour3,0,23,{NULL}};
const PROGMEM menu_item menuAdimLvl3 = {MENU_AUTODIM_LEVEL3,menu_num+menu_isSub,"ADL3","ADL3",&globals.AutoDimLevel3,0,10,{NULL}};
#endif
#ifdef HAVE_SET_DATE						
const PROGMEM menu_item menuDate_ = {MENU_DATE,menu_hasSub,"DAT","DATE ",NULL,0,0,{NULL}};
const PROGMEM menu_item menuYear = {MENU_DATEYEAR,menu_num+menu_isSub,"YEAR","YEAR",&globals.dateyear,10,29,{NULL}};
const PROGMEM menu_item menuMonth = {MENU_DATEMONTH,menu_num+menu_isSub,"MNTH","MONTH",&globals.datemonth,1,12,{NULL}};
const PROGMEM menu_item menuDay = {MENU_DATEDAY,menu_num+menu_isSub,"DAY","DAY",&globals.dateday,1,31,{NULL}};
#endif
const PROGMEM menu_item menuDots = {MENU_DOTS,menu_offOn,"DOTS","DOTS ",&globals.show_dots,0,1,{NULL}};
#ifdef HAVE_AUTO_DST
const PROGMEM menu_item menuDST_ = {MENU_DST,menu_hasSub,"DST","DST  ",NULL,0,0,{NULL}};
const PROGMEM menu_item menuDST = {MENU_DST_ENABLE,menu_list+menu_isSub,"DST","DST",&globals.DST_mode,0,3,{menu_adst}};
#elif defined HAVE_GPS
const PROGMEM menu_item menuDST = {MENU_DST_ENABLE,menu_offOn,"DST","DST",&globals.DST_mode,0,2,{NULL}};
#endif
#ifdef HAVE_AUTO_DST
const PROGMEM menu_item menuRules = {MENU_RULES,menu_hasSub+menu_isSub,"RUL","RULES",NULL,0,0,{NULL}};
const PROGMEM menu_item menuRule0 = {MENU_RULE0,menu_num+menu_isSub,"RUL0","RULE0",&globals.DST_Rules[0],1,12,{NULL}};
const PROGMEM menu_item menuRule1 = {MENU_RULE0,menu_num+menu_isSub,"RUL1","RULE1",&globals.DST_Rules[1],1,7,{NULL}};
const PROGMEM menu_item menuRule2 = {MENU_RULE1,menu_num+menu_isSub,"RUL2","RULE2",&globals.DST_Rules[2],1,5,{NULL}};
const PROGMEM menu_item menuRule3 = {MENU_RULE2,menu_num+menu_isSub,"RUL3","RULE3",&globals.DST_Rules[3],1,23,{NULL}};
const PROGMEM menu_item menuRule4 = {MENU_RULE3,menu_num+menu_isSub,"RUL4","RULE4",&globals.DST_Rules[4],1,12,{NULL}};
const PROGMEM menu_item menuRule5 = {MENU_RULE4,menu_num+menu_isSub,"RUL5","RULE5",&globals.DST_Rules[5],1,7,{NULL}};
const PROGMEM menu_item menuRule6 = {MENU_RULE5,menu_num+menu_isSub,"RUL6","RULE6",&globals.DST_Rules[6],1,5,{NULL}};
const PROGMEM menu_item menuRule7 = {MENU_RULE6,menu_num+menu_isSub,"RUL7","RULE7",&globals.DST_Rules[7],1,23,{NULL}};
const PROGMEM menu_item menuRule8 = {MENU_RULE7,menu_num+menu_isSub,"RUL8","RULE8",&globals.DST_Rules[8],1,1,{NULL}};
// offset can't be changed by the menu???
#endif
#if defined HAVE_FLW
const PROGMEM menu_item menuFLW = {MENU_FLW,menu_offOn,"FLW","FLW",&globals.flw_enabled,0,2,{NULL}};
#endif
#if defined HAVE_GPS
const PROGMEM menu_item menuGPS_ = {MENU_GPS,menu_hasSub,"GPS","GPS  ",NULL,0,0,{NULL}};
const PROGMEM menu_item menuGPS = {MENU_GPS_ENABLE,menu_list+menu_isSub,"GPS","GPS",&globals.gps_enabled,0,3,{menu_gps}};
const PROGMEM menu_item menuTZh = {MENU_TZH,menu_num+menu_isSub,"TZH","TZ-H",&globals.TZ_hour,-12,12,{NULL}};
const PROGMEM menu_item menuTZm = {MENU_TZM,menu_num+menu_isSub,"TZM","TZ-M",&globals.TZ_minute,0,59,{NULL}};
#endif
#if defined HAVE_GPS_DEBUG
const PROGMEM menu_item menuGPSc = {MENU_GPSC,menu_num+menu_isSub,"GPSC","GPSC",&g_gps_cks_errors,0,0,{NULL}};
const PROGMEM menu_item menuGPSp = {MENU_GPSP,menu_num+menu_isSub,"GPSP","GPSP",&g_gps_parse_errors,0,0,{NULL}};
const PROGMEM menu_item menuGPSt = {MENU_GPST,menu_num+menu_isSub,"GPST","GPST",&g_gps_time_errors,0,0,{NULL}};
#endif
#if defined HAVE_HUMIDITY
const PROGMEM menu_item menuHumid = {MENU_HUMID,menu_offOn,"HUMI","HUMID",&globals.show_humid,0,2,{NULL}};
#endif
#if defined HAVE_PRESSURE
const PROGMEM menu_item menuPress = {MENU_PRESS,menu_offOn,"PRES","PRESS",&globals.show_press,0,2,{NULL}};
#endif
#if defined HAVE_TEMPERATURE
const PROGMEM menu_item menuTemp = {MENU_TEMP,menu_offOn,"TEMP","TEMP",&globals.show_temp,0,2,{NULL}};
#endif
#ifdef HAVE_MENU_TIME
const PROGMEM menu_item menuTime = {MENU_TIME,menu_hasSub+menu_time,"TIME","TIME ",NULL,0,0,{NULL}};
const PROGMEM menu_item menuTimeHr = {MENU_TIMEHOUR,menu_num+menu_isSub,"HOUR","HOUR",&time_hour,0,23,{NULL}};
const PROGMEM menu_item menuTimeMin = {MENU_TIMEMINUTE,menu_num+menu_isSub,"MIN","MIN ",&time_minute,0,59,{NULL}};
#endif
const PROGMEM menu_item menuVol = {MENU_VOL,menu_list,"VOL","VOL",&globals.volume,0,2,{menu_volume}};

// MENU TABLE, in mostly alphabetical order
const PROGMEM menu_item* PROGMEM const menuItems[] = { 
	&menu24h, 
#ifdef HAVE_AUTO_DATE
	&menuAdate_, &menuAdate, &menuRegion,
#endif
#ifdef HAVE_AUTO_DIM
	&menuAdim_,	&menuAdim, &menuAdimHr1, &menuAdimLvl1, &menuAdimHr2, &menuAdimLvl2, &menuAdimHr3, &menuAdimLvl3,
#endif
#ifdef HAVE_MENU_TIME
	&menuAlarm, &menuAlarmHr, &menuAlarmMin,
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
#if defined HAVE_HUMIDITY
	&menuHumid,
#endif
#if defined HAVE_PRESSURE
	&menuPress,
#endif
#if defined HAVE_TEMPERATURE
	&menuTemp,
#endif
#ifdef HAVE_MENU_TIME
	&menuTime, &menuTimeHr, &menuTimeMin,
#endif
	&menuVol,
	NULL};  // end of list marker must be here

uint8_t menu_disabled[MENU_END];

static menu_item_rw current_item;
menu_item* getItem(uint8_t idx)
{
	menu_item* mPtr = (menu_item*)pgm_read_word(&menuItems[idx]);  // address of current menu item
	if (mPtr == NULL)  return(NULL);
//	memcpy_P(&current_item, &mPtr, sizeof(current_item));
	current_item.menuNum = pgm_read_byte(&mPtr->menuNum);
	current_item.flags = (menu_flags)pgm_read_byte(&mPtr->flags);
	strncpy_P(current_item.shortName,(char *)&mPtr->shortName,4); 
	strncpy_P(current_item.longName,(char *)&mPtr->longName,5); 
	current_item.setting = (int8_t *)pgm_read_word(&mPtr->setting);
	current_item.loLimit = pgm_read_byte(&mPtr->loLimit);
	current_item.hiLimit = pgm_read_byte(&mPtr->hiLimit);
	*current_item.menuList = (menu_value*)pgm_read_word(&mPtr->menuList);
	return (menu_item*)&current_item;
}

uint8_t nextItem(uint8_t idx, uint8_t skipSub)  // next menu item
{
	menu_item * mPtr = getItem(idx);  // current menu item
	uint8_t inSub = (mPtr->flags & menu_isSub) && !(mPtr->flags & menu_hasSub);  // are we in a sub menu now?
	mPtr = getItem(++idx);  // next menu item
	while ((mPtr != NULL) && ((menu_disabled[idx]) || (skipSub && (mPtr->flags & menu_isSub) && !inSub)) ) { 
		mPtr = getItem(++idx);  // next menu item
	}
	return idx;
}

