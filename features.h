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
 
/*
 * Feature system IDEA:
 *
 * #define FEATURE_XXX YES/NO
 * 
 * a feature may define one or more HAVE_XXX defines
 *
 * e.g:
 * FEATURE_GPS
 * defines
 * HAVE_GPS
 */

#ifndef FEATURES_H_
#define FEATURES_H_

// fixme: add config.h that contains local config for each compile

// Boards
#define BOARD_VFD_DELUXE 0
#define BOARD_VFD_DELUXE_BETA 1
#define BOARD_VFD_MODULAR_CLOCK 2

#define BOARD BOARD_VFD_DELUXE

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
#define SHIELD SHIELD_IN8_2
#define SHIELD_DIGITS 6
//#define IN14_FIX

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

#define FEATURE_SHIELD_AUTODETECT NO
#define FEATURE_MPL115A2 YES // Temperature and Atmospheric pressure sensor
#define FEATURE_HIH6121 NO   // Temperature and Humidity sensor
#define FEATURE_ROTARY YES
#define FEATURE_GPS YES
#define FEATURE_RGB_BACKLIGHT NO

// fixme: this can be automatic based on __AVR_ATmega32U4__ define
#define FEATURE_LEONARDO YES

// fixme: this should probably be generated from a script
// list FEATURES and what each feature defines when set to on
// could also do dependencies

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
#endif

///////////////////////////////////////////

#if !(defined FEATURE_HIH6121) || FEATURE_HIH6121 < NO || FEATURE_HIH6121 > YES
#  error Must define FEATURE_HIH6121 to be YES or NO
#endif

#if FEATURE_HIH6121 == YES
#  define HAVE_HIH6121
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

#if !(defined FEATURE_LEONARDO) || FEATURE_LEONARDO < NO || FEATURE_LEONARDO > YES
#  error Must define FEATURE_LEONARDO to be YES or NO
#endif

#if FEATURE_LEONARDO == YES
#  define HAVE_LEONARDO
#endif

///////////////////////////////////////////

#if !(defined FEATURE_RGB_BACKLIGHT) || FEATURE_RGB_BACKLIGHT < NO || FEATURE_RGB_BACKLIGHT > YES
#  error Must define FEATURE_RGB_BACKLIGHT to be YES or NO
#endif

#if FEATURE_RGB_BACKLIGHT == YES
#  define HAVE_RGB_BACKLIGHT
#endif


#endif // FEATURES_H_

