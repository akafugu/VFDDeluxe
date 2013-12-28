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

#ifndef FEATURES_H_
#define FEATURES_H_

// fixme: add config.h that contains local config for each compile

// Boards
#define BOARD_VFD_DELUXE 0
#define BOARD_VFD_MODULAR_CLOCK 1

// fixme: this is just a convenience declaration for now since there are only two board types
#ifdef __AVR_ATmega32U4__
#define BOARD BOARD_VFD_DELUXE
#else
#define BOARD BOARD_VFD_MODULAR_CLOCK
#endif

//#define BOARD BOARD_VFD_DELUXE
//#define BOARD BOARD_VFD_MODULAR_CLOCK

// Display Shields
enum shield_t {
	SHIELD_NONE = 0,
	// VFD displays
	SHIELD_7SEG,  // generic 7-seg display
	SHIELD_14SEG, // generic 14-seg display
	SHIELD_16SEG, // generic 16-seg display
	SHIELD_IV6,   // VFD Modular Clock IV-6 shield
	SHIELD_IV17,  // VFD Modular Clock IV-17 shield
	SHIELD_IV18,  // VFD Modular Clock IV-18 shield
	SHIELD_IV22,  // VFD Modular Clock IV-22 shield
	// Nixie displays
        SHIELD_IN8_2, // VFDDeluxe, IN-8-2 6-digit Nixie
	SHIELD_IN14,  // VFDDeluxe IN-14 6-digit Nixie
	SHIELD_HYBRID, // VFDDeluxe IV-18+IN-17 VFD/Nixie hybrid shield
};

//#define SHIELD SHIELD_IV18
//#define SHIELD_DIGITS 8

//#define SHIELD SHIELD_IN14
//#define SHIELD SHIELD_IN8_2
//#define SHIELD_DIGITS 6
//#define IN14_FIX

#define SHIELD SHIELD_IV17
#define SHIELD_DIGITS 4

// Display Shield identifiers
// add here

/*
#define FEATURE_HV518
#define FEATURE_NIXIE
#define FEATURE_BUTTONS
#define FEATURE_FLW
#define FEATURE_LEONARDO
#define FEATURE_MEGA328
#define FEATURE_ALARM_SWITCH
#define FEATURE_ALARM_BUTTON
*/

#define YES 2
#define NO  1

#define FEATURE_NIXIE_SUPPORT NO
#define FEATURE_SHIELD_AUTODETECT YES
#define FEATURE_RTC_TEMP YES // Get temperature from RTC
#define FEATURE_MPL115A2 NO // Temperature and Atmospheric pressure sensor
#define FEATURE_HIH6121 NO   // Temperature and Humidity sensor
#define FEATURE_ROTARY NO
#define FEATURE_GPS YES
#define FEATURE_GPS_DEBUG YES
#define FEATURE_RGB_BACKLIGHT NO
#define FEATURE_LOWERCASE YES
#define FEATURE_ALTERNATE_FONT YES
#define FEATURE_SERIAL_DEBUG YES // Wait for serial console to open before booting
#define FEATURE_SERIAL_DEBUG NO
#define FEATURE_FLW YES
#define FEATURE_RTC_SQW YES
#define FEATURE_RTC_SQW NO // wbp
#define FEATURE_AUTO_DATE YES
#define FEATURE_AUTO_DST YES
#define FEATURE_AUTO_DIM YES
#define FEATURE_SET_DATE YES // set date in menu?

// Support for generic displays (excludes the standard shields)
#define FEATURE_7SEG_SUPPORT NO
#define FEATURE_14SEG_SUPPORT NO
#define FEATURE_16SEG_SUPPORT NO


// fixme: this can be automatic based on __AVR_ATmega32U4__ define

#ifdef __AVR_ATmega32U4__
#define FEATURE_LEONARDO YES
#define FEATURE_ATMEGA328 NO
#elif defined(__AVR_ATmega328P__)
#define FEATURE_LEONARDO NO
#define FEATURE_ATMEGA328 YES
#else
#error Unsupported board: Only ATmega328 and ATmega32U4 are supported at this time
#endif

// fixme: this should probably be generated from a script
// list FEATURES and what each feature defines when set to on
// could also do dependencies

///////////////////////////////////////////

// Experimental support for driving nixies
#if !(defined FEATURE_NIXIE_SUPPORT) || FEATURE_NIXIE_SUPPORT < NO || FEATURE_NIXIE_SUPPORT > YES
#  error Must define FEATURE_NIXIE_SUPPORT to be YES or NO
#endif

#if FEATURE_NIXIE_SUPPORT == YES
#  define HAVE_NIXIE_SUPPORT
#endif


///////////////////////////////////////////

#if !(defined FEATURE_SHIELD_AUTODETECT) || FEATURE_SHIELD_AUTODETECT < NO || FEATURE_SHIELD_AUTODETECT > YES
#  error Must define FEATURE_SHIELD_AUTODETECT to be YES or NO
#endif

#if FEATURE_SHIELD_AUTODETECT == YES
#  define HAVE_SHIELD_AUTODETECT
#endif

///////////////////////////////////////////

#if !(defined FEATURE_MPL115A2) || FEATURE_MPL115A2 < NO || FEATURE_MPL115A2 > YES
#  error Must define FEATURE_MPL115A2 to be YES or NO
#endif

#if FEATURE_MPL115A2 == YES
#  define HAVE_MPL115A2
#  define HAVE_TEMPERATURE
#  define HAVE_PRESSURE
#endif

///////////////////////////////////////////

#if !(defined FEATURE_HIH6121) || FEATURE_HIH6121 < NO || FEATURE_HIH6121 > YES
#  error Must define FEATURE_HIH6121 to be YES or NO
#endif

#if FEATURE_HIH6121 == YES
#  define HAVE_HIH6121
#  define HAVE_TEMPERATURE
#  define HAVE_HUMIDITY
#endif

///////////////////////////////////////////

#if !(defined FEATURE_ROTARY) || FEATURE_ROTARY < NO || FEATURE_ROTARY > YES
#  error Must define FEATURE_ROTARY to be YES or NO
#endif

#if FEATURE_ROTARY == YES
// we have a rotary encoder with a single button for input
#  define HAVE_ROTARY
#else
// we have two buttons for input
#  define HAVE_BUTTONS
#endif

///////////////////////////////////////////

#if !(defined FEATURE_GPS) || FEATURE_GPS < NO || FEATURE_GPS > YES
#  error Must define FEATURE_GPS to be YES or NO
#endif

#if FEATURE_GPS == YES
#  define HAVE_GPS
#endif

///////////////////////////////////////////

#if !(defined FEATURE_GPS_DEBUG) || FEATURE_GPS_DEBUG < NO || FEATURE_GPS_DEBUG > YES
#  error Must define FEATURE_GPS_DEBUG to be YES or NO
#endif

#if FEATURE_GPS_DEBUG == YES
#  define HAVE_GPS_DEBUG
#endif

///////////////////////////////////////////

#if !(defined FEATURE_LEONARDO) || FEATURE_LEONARDO < NO || FEATURE_LEONARDO > YES
#  error Must define FEATURE_LEONARDO to be YES or NO
#endif

#if FEATURE_LEONARDO == YES
#  define HAVE_LEONARDO
#endif

///////////////////////////////////////////

#if !(defined FEATURE_ATMEGA328) || FEATURE_ATMEGA328 < NO || FEATURE_ATMEGA328 > YES
#  error Must define FEATURE_ATMEGA328 to be YES or NO
#endif

#if FEATURE_ATMEGA328 == YES
#  define HAVE_ATMEGA328
#endif

///////////////////////////////////////////

#if !(defined FEATURE_RGB_BACKLIGHT) || FEATURE_RGB_BACKLIGHT < NO || FEATURE_RGB_BACKLIGHT > YES
#  error Must define FEATURE_RGB_BACKLIGHT to be YES or NO
#endif

#if FEATURE_RGB_BACKLIGHT == YES
#  define HAVE_RGB_BACKLIGHT
#endif

///////////////////////////////////////////

#if !(defined FEATURE_LOWERCASE) || FEATURE_LOWERCASE < NO || FEATURE_LOWERCASE > YES
#  error Must define FEATURE_LOWERCASE to be YES or NO
#endif

#if FEATURE_LOWERCASE == YES
#  define HAVE_LOWERCASE
#endif

///////////////////////////////////////////

#if !(defined FEATURE_ALTERNATE_FONT) || FEATURE_ALTERNATE_FONT < NO || FEATURE_ALTERNATE_FONT > YES
#  error Must define FEATURE_ALTERNATE_FONT to be YES or NO
#endif

#if FEATURE_ALTERNATE_FONT == YES
#  define HAVE_ALTERNATE_FONT
#endif

///////////////////////////////////////////

#if !(defined FEATURE_SERIAL_DEBUG) || FEATURE_SERIAL_DEBUG < NO || FEATURE_SERIAL_DEBUG > YES
#  error Must define FEATURE_SERIAL_DEBUG to be YES or NO
#endif

#if FEATURE_SERIAL_DEBUG == YES
#  define HAVE_SERIAL_DEBUG
#endif

///////////////////////////////////////////

#if !(defined FEATURE_FLW) || FEATURE_FLW < NO || FEATURE_FLW > YES
#  error Must define FEATURE_FLW to be YES or NO
#endif

#if FEATURE_FLW == YES
#  define HAVE_FLW
#endif

///////////////////////////////////////////

#if !(defined FEATURE_RTC_SQW) || FEATURE_RTC_SQW < NO || FEATURE_RTC_SQW > YES
#  error Must define FEATURE_RTC_SQW to be YES or NO
#endif

#if FEATURE_RTC_SQW == YES
#  define HAVE_RTC_SQW
#endif

///////////////////////////////////////////

#if !(defined FEATURE_RTC_TEMP) || FEATURE_RTC_TEMP < NO || FEATURE_RTC_TEMP > YES
#  error Must define FEATURE_RTC_TEMP to be YES or NO
#endif

#if FEATURE_RTC_TEMP == YES
#  define HAVE_RTC_TEMP
#  define HAVE_TEMPERATURE
#endif

///////////////////////////////////////////

#if !(defined FEATURE_AUTO_DATE) || FEATURE_AUTO_DATE < NO || FEATURE_AUTO_DATE > YES
#  error Must define FEATURE_AUTO_DATE to be YES or NO
#endif

#if FEATURE_AUTO_DATE == YES
#  define HAVE_AUTO_DATE
#endif

///////////////////////////////////////////

#if !(defined FEATURE_AUTO_DST) || FEATURE_AUTO_DST < NO || FEATURE_AUTO_DST > YES
#  error Must define FEATURE_AUTO_DST to be YES or NO
#endif

#if FEATURE_AUTO_DST == YES
#  define HAVE_AUTO_DST
#endif

///////////////////////////////////////////

#if !(defined FEATURE_AUTO_DIM) || FEATURE_AUTO_DIM < NO || FEATURE_AUTO_DIM > YES
#  error Must define FEATURE_AUTO_DIM to be YES or NO
#endif

#if FEATURE_AUTO_DIM == YES
#  define HAVE_AUTO_DIM
#endif

///////////////////////////////////////////

#if !(defined FEATURE_SET_DATE) || FEATURE_SET_DATE < NO || FEATURE_SET_DATE > YES
#  error Must define FEATURE_SET_DATE to be YES or NO
#endif

#if FEATURE_SET_DATE == YES
#  define HAVE_SET_DATE
#endif

///////////////////////////////////////////

#if !(defined FEATURE_7SEG_SUPPORT) || FEATURE_7SEG_SUPPORT < NO || FEATURE_7SEG_SUPPORT > YES
#  error Must define FEATURE_7SEG_SUPPORT to be YES or NO
#endif

#if FEATURE_7SEG_SUPPORT == YES
#  define HAVE_7SEG_SUPPORT
#endif

///////////////////////////////////////////

#if !(defined FEATURE_14SEG_SUPPORT) || FEATURE_14SEG_SUPPORT < NO || FEATURE_14SEG_SUPPORT > YES
#  error Must define FEATURE_14SEG_SUPPORT to be YES or NO
#endif

#if FEATURE_14SEG_SUPPORT == YES
#  define HAVE_14SEG_SUPPORT
#endif

///////////////////////////////////////////

#if !(defined FEATURE_16SEG_SUPPORT) || FEATURE_16SEG_SUPPORT < NO || FEATURE_16SEG_SUPPORT > YES
#  error Must define FEATURE_16SEG_SUPPORT to be YES or NO
#endif

#if FEATURE_16SEG_SUPPORT == YES
#  define HAVE_16SEG_SUPPORT
#endif

#endif // FEATURES_H_

