//#define FEATURE_GPS_DEBUG  // enables GPS debugging counters & menu items
//#define FEATURE_AUTO_DIM  // moved to Makefile

#include "features.h"
#include "menu_data.h"

#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

menu_state_t menu_state;

uint8_t EEMEM b_menu_dummy = 0; 
uint8_t g_menu_dummy = 0;

//const menu_value menuOffOn[] = { {0, " off"}, {1, "  on"} };
const menu_value PROGMEM menu_gps[] = { {0, " off"}, {48, "  48"}, {96, "  96"} };
#if defined HAVE__AUTO_DST
const PROGMEM menu_value menu_adst[] = { {0, " off"}, {1, "  on"}, {2, "auto"} };
#endif
const PROGMEM menu_value menu_volume[] = { {0, "  lo"}, {1, "  hi"} };
const PROGMEM menu_value menu_region[] = { {0, " dmy"}, {1, " mdy"}, {2, " ymd"} };

const PROGMEM menu_item menu24h = {MENU_24H,menu_offOn,"24H","24H",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
const PROGMEM menu_item menuBrt = {MENU_BRIGHTNESS,menu_num,"BRIT","BRITE",&g_menu_dummy,&b_menu_dummy,0,10,{NULL}};
#ifdef HAVE_AUTO_DATE
const PROGMEM menu_item menuAdate_ = {MENU_AUTODATE,menu_hasSub,"ADT","ADATE",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuAdate = {MENU_AUTODATE,menu_offOn+menu_isSub,"ADTE","ADATE",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
const PROGMEM menu_item menuRegion = {MENU_REGION,menu_list+menu_isSub,"REGN","RGION",&g_menu_dummy,&b_menu_dummy,0,3,{menu_region}};
#endif
#ifdef HAVE_AUTO_DIM
const PROGMEM menu_item menuAdim_ = {MENU_AUTODIM,menu_hasSub,"ADM","ADIM ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuAdim = {MENU_AUTODIM_ENABLE,menu_offOn+menu_isSub,"ADIM","ADIM",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
const PROGMEM menu_item menuAdimHr = {MENU_AUTODIM_HOUR,menu_num+menu_isSub,"ADMH","ADMH",&g_menu_dummy,&b_menu_dummy,0,23,{NULL}};
const PROGMEM menu_item menuAdimLvl = {MENU_AUTODIM_LEVEL,menu_num+menu_isSub,"ADML","ADML",&g_menu_dummy,&b_menu_dummy,0,10,{NULL}};
const PROGMEM menu_item menuAbrtHr = {MENU_AUTOBRT_HOUR,menu_num+menu_isSub,"ABTH","ABTH",&g_menu_dummy,&b_menu_dummy,0,23,{NULL}};
const PROGMEM menu_item menuAbrtLvl = {MENU_AUTOBRT_LEVEL,menu_num+menu_isSub,"ABTL","ABTL",&g_menu_dummy,&b_menu_dummy,1,10,{NULL}};
#endif
#ifdef HAVE_SET_DATE						
const PROGMEM menu_item menuDate_ = {MENU_DATE,menu_hasSub,"DAT","DATE ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuYear = {MENU_DATEYEAR,menu_num+menu_isSub,"YEAR","YEAR",&g_menu_dummy,NULL,10,29,{NULL}};
const PROGMEM menu_item menuMonth = {MENU_DATEMONTH,menu_num+menu_isSub,"MNTH","MONTH",&g_menu_dummy,NULL,1,12,{NULL}};
const PROGMEM menu_item menuDay = {MENU_DATEDAY,menu_num+menu_isSub,"DAY","DAY",&g_menu_dummy,NULL,1,31,{NULL}};
#endif
const PROGMEM menu_item menuDots = {MENU_DOTS,menu_offOn,"DOTS","DOTS ",&g_menu_dummy,&b_menu_dummy,0,1,{NULL}};
#ifdef HAVE_AUTO_DST
const PROGMEM menu_item menuDST_ = {MENU_DST,menu_hasSub,"DST","DST  ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuDST = {MENU_DST_ENABLE,menu_list+menu_isSub,"DST","DST",&g_menu_dummy,&b_menu_dummy,0,3,{menu_adst}};
#elif defined HAVE_GPS
const PROGMEM menu_item menuDST = {MENU_DST_ENABLE,menu_offOn,"DST","DST",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
#endif
#ifdef HAVE_AUTO_DST
const PROGMEM menu_item menuRules = {MENU_RULES,menu_hasSub+menu_isSub,"RUL","RULES",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuRule0 = {MENU_RULE0,menu_num+menu_isSub,"RUL0","RULE0",&g_menu_dummy,&b_menu_dummy,1,12,{NULL}};
const PROGMEM menu_item menuRule1 = {MENU_RULE1,menu_num+menu_isSub,"RUL1","RULE1",&g_menu_dummy,&b_menu_dummy,1,7,{NULL}};
const PROGMEM menu_item menuRule2 = {MENU_RULE2,menu_num+menu_isSub,"RUL2","RULE2",&g_menu_dummy,&b_menu_dummy,1,5,{NULL}};
const PROGMEM menu_item menuRule3 = {MENU_RULE3,menu_num+menu_isSub,"RUL3","RULE3",&g_menu_dummy,&b_menu_dummy,0,23,{NULL}};
const PROGMEM menu_item menuRule4 = {MENU_RULE4,menu_num+menu_isSub,"RUL4","RULE4",&g_menu_dummy,&b_menu_dummy,1,12,{NULL}};
const PROGMEM menu_item menuRule5 = {MENU_RULE5,menu_num+menu_isSub,"RUL5","RULE5",&g_menu_dummy,&b_menu_dummy,1,7,{NULL}};
const PROGMEM menu_item menuRule6 = {MENU_RULE6,menu_num+menu_isSub,"RUL6","RULE6",&g_menu_dummy,&b_menu_dummy,1,5,{NULL}};
const PROGMEM menu_item menuRule7 = {MENU_RULE7,menu_num+menu_isSub,"RUL7","RULE7",&g_menu_dummy,&b_menu_dummy,0,23,{NULL}};
const PROGMEM menu_item menuRule8 = {MENU_RULE8,menu_num+menu_isSub,"RUL8","RULE8",&g_menu_dummy,&b_menu_dummy,1,1,{NULL}};  // offset can't be changed
#endif
#if defined HAVE_FLW
const PROGMEM menu_item menuFLW = {MENU_FLW,menu_offOn,"FLW","FLW",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
#endif
#if defined HAVE_GPS
const PROGMEM menu_item menuGPS_ = {MENU_GPS,menu_hasSub,"GPS","GPS  ",NULL,NULL,0,0,{NULL}};
const PROGMEM menu_item menuGPS = {MENU_GPS_ENABLE,menu_list+menu_isSub,"GPS","GPS",&g_menu_dummy,&b_menu_dummy,0,3,{menu_gps}};
const PROGMEM menu_item menuTZh = {MENU_TZH,menu_num+menu_isSub,"TZH","TZ-H",&g_menu_dummy,&b_menu_dummy,-12,12,{NULL}};
const PROGMEM menu_item menuTZm = {MENU_TZM,menu_num+menu_isSub,"TZM","TZ-M",&g_menu_dummy,&b_menu_dummy,0,59,{NULL}};
#endif
#if defined HAVE_GPS_DEBUG
const PROGMEM menu_item menuGPSc = {MENU_GPSC,menu_num+menu_isSub,"GPSC","GPSC",&g_menu_dummy,NULL,0,0,{NULL}};
const PROGMEM menu_item menuGPSp = {MENU_GPSP,menu_num+menu_isSub,"GPSP","GPSP",&g_menu_dummy,NULL,0,0,{NULL}};
const PROGMEM menu_item menuGPSt = {MENU_GPST,menu_num+menu_isSub,"GPST","GPST",&g_menu_dummy,NULL,0,0,{NULL}};
#endif
const PROGMEM menu_item menuTemp = {MENU_TEMP,menu_offOn,"TEMP","TEMP",&g_menu_dummy,&b_menu_dummy,0,2,{NULL}};
const PROGMEM menu_item menuVol = {MENU_VOL,menu_list,"VOL","VOL",&g_menu_dummy,&b_menu_dummy,0,2,{menu_volume}};

const PROGMEM menu_item* PROGMEM const menuItems[] = { 
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

uint8_t menu_disabled[MENU_END];

static volatile uint8_t menuIdx = 0;
static menu_item_rw current_item;

menu_item* getItem(uint8_t idx)
{
	menu_item* mPtr = (menu_item*)pgm_read_word(&menuItems[idx]);  // address of current menu item
	if (mPtr == NULL)  return(NULL);
        
//	memcpy_P(&current_item, &mPtr, sizeof(current_item));
	current_item.menuNum = pgm_read_byte(&mPtr->menuNum);
	current_item.flags = pgm_read_byte(&mPtr->flags);
	strncpy_P(current_item.shortName,(char *)&mPtr->shortName,4); 
        return NULL;
	strncpy_P(current_item.longName,(char *)&mPtr->longName,5); 
	current_item.setting = (int8_t *)pgm_read_word(&mPtr->setting);
	current_item.eeAddress = (uint8_t*)pgm_read_word(&mPtr->eeAddress);
	current_item.loLimit = pgm_read_byte(&mPtr->loLimit);
	current_item.hiLimit = pgm_read_byte(&mPtr->hiLimit);
	*current_item.menuList = (menu_value*)pgm_read_word(&mPtr->menuList);
	return (menu_item*)&current_item;
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


