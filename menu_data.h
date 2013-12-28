#ifndef MENU_DATA_H_
#define MENU_DATA_H_
#include <avr/pgmspace.h>

#ifdef __cplusplus 
extern "C" { 
#endif

#ifdef __FLASH
#define FLASH __flash
#else
#define FLASH
#endif

typedef enum {
	menu_noflags = 0x00,
	menu_num = 0x01,
	menu_offOn = 0x02,  
	menu_list = 0x04,
	menu_hasSub = 0x10,
	menu_isSub = 0x20,
//	menu_disabled = 0x80,
} menu_flags;

typedef struct {
	const uint8_t value;  // list of possible values in order
	const char valName[4];  // list of value names for display
} menu_value;

typedef struct {
	const uint8_t menuNum;  // menu item number
	menu_flags flags;  // flags
	const char shortName[4];
	const char longName[5];
	int8_t* setting;
	const int8_t loLimit;  // low limit for num
	const int8_t hiLimit;  // high limit for num, # of values for list
	const menu_value* menuList[];  // list of menu choices
} menu_item;

typedef struct {
	uint8_t menuNum;  // menu item number
	menu_flags flags;  // flags
	char shortName[4];
	char longName[5];
	int8_t* setting;
	int8_t loLimit;  // low limit for num
	int8_t hiLimit;  // high limit for num, # of values for list
	menu_value* menuList[];  // list of menu choices
} menu_item_rw;

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
	MENU_AUTODIM_HOUR1,
	MENU_AUTODIM_LEVEL1,
	MENU_AUTODIM_HOUR2,
	MENU_AUTODIM_LEVEL2,
	MENU_AUTODIM_HOUR3,
	MENU_AUTODIM_LEVEL3,
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
	MENU_HUMID,
	MENU_TEMP,
	MENU_PRESS,
	MENU_VOL,
	MENU_END  // must be last
} menu_number;

extern menu_state_t menu_state;
extern uint8_t menu_disabled[MENU_END];

menu_item* getItem(uint8_t idx);
uint8_t nextItem(uint8_t idx, uint8_t skipSub);

#ifdef __cplusplus 
}
#endif 

#endif // MENU_DATA_H_

