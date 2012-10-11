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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include <stdbool.h>
#include <stdlib.h>

#include "display.h"
#include "button.h"
#include "pitches.h"
#include "rotary.h"

#include <Wire.h>
#include <WireRtcLib.h>
#include <MPL115A2.h>

#include "gps.h"

WireRtcLib rtc;
GPS gps;

// Piezo
#define PIEZO 11

// Cached settings
uint8_t g_24h_clock = true;
uint8_t g_show_temp = true;
uint8_t g_show_dots = true;
uint8_t g_brightness = 5;
uint8_t g_volume = 0;

// Other globals
uint8_t g_has_dots = false; // can current shield show dot (decimal points)
uint8_t g_alarming = false; // alarm is going off
uint8_t g_alarm_switch;
WireRtcLib::tm* tt = NULL; // for holding RTC values

volatile uint16_t g_rotary_moved_timer;

extern enum shield_t shield;

Rotary rot;

#define TEMP_CORR -1

void initialize(void)
{
  // read eeprom
  // fixme: implement

  pinMode(PIEZO, OUTPUT);
  digitalWrite(PIEZO, LOW);

  // initialize button
  initialize_button();

  // fixme: move to button class?
  // Set switch as input and enable pullup
  //SWITCH_DDR  &= ~(_BV(SWITCH_BIT));
  //SWITCH_PORT |= _BV(SWITCH_BIT);

  // test
  pinMode(13, OUTPUT);

  for (int i = 0; i < 5; i++) {
    digitalWrite(13, LOW);
    delay(100);
    digitalWrite(13, HIGH);
    delay(100);
  }

  rot.begin();

  sei();
  Wire.begin();

  rtc.begin();
  rtc.runClock(true);
  

  //rtc.setTime_s(16, 9, 0);
  //rtc_set_alarm_s(17,0,0);

  //set_shield(SHIELD_IV17, 4);
  set_shield(SHIELD_IV18, 8);
  //set_shield(SHIELD_7SEG, 10);
  //set_shield(SHIELD_14SEG, 8);
  
  display_init(g_brightness);

  //g_alarm_switch = get_alarm_switch();

  // set up interrupt for alarm switch
  /*
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18);
  */
}

/*
// Alarm switch changed interrupt
ISR( PCINT2_vect )
{
	if ( (SWITCH_PIN & _BV(SWITCH_BIT)) == 0)
		g_alarm_switch = false;
	else
		g_alarm_switch = true;
}
*/

uint8_t print_digits(uint8_t num, uint8_t offset);
void clear_data(void);

void read_rtc(bool show_extra_info)
{
  static uint16_t counter = 0;
	
  if (g_show_temp && rtc.isDS3231() && counter > 5) {
    MPL115A2.ReadSensor();
    MPL115A2.shutdown();

    float temp = MPL115A2.GetTemperature() + TEMP_CORR;
    
    int8_t t;
    uint8_t f;
    
    t = (int)temp;
    f = (int)((temp-t)*100);

    show_temp(t, f);

    /*
    int8_t t;
    uint8_t f;
    rtc.getTemp(&t, &f);
    show_temp(t, f);
    */
  }
  else {
    tt = rtc.getTime();
    if (tt == NULL) return;
    show_time(tt, g_24h_clock, show_extra_info);
  }

  counter++;
  if (counter == 10) counter = 0;
}

struct BUTTON_STATE buttons;

// menu states
typedef enum {
	// basic states
	STATE_CLOCK = 0,
	STATE_SET_CLOCK,
	STATE_SET_ALARM,
	// menu
	STATE_MENU_BRIGHTNESS,
	STATE_MENU_24H,
	STATE_MENU_VOL,
	STATE_MENU_TEMP,
	STATE_MENU_DOTS,
	STATE_MENU_LAST,
} state_t;

state_t clock_state = STATE_CLOCK;

// display modes
typedef enum {
	MODE_NORMAL = 0, // normal mode: show time/seconds
	MODE_AMPM, // shows time AM/PM
	MODE_LAST,
} display_mode_t;

display_mode_t clock_mode = MODE_NORMAL;

void setup()
{
  Serial.begin(9600);
  Serial.println("VFD Deluxe");
  
  MPL115A2.begin();
  MPL115A2.shutdown();

  initialize();
  
  gps.begin();
}

void loop()
{
  /*
  // test: write alphabet
  while (1) {
    for (int i = 'A'; i <= 'Z'+1; i++) {
      set_char_at(i, 0);
      set_char_at(i+1, 1);
      set_char_at(i+2, 2);
      set_char_at(i+3, 3);

      if (get_digits() == 6) {
        set_char_at(i+4, 4);
        set_char_at(i+5, 5);
      }

      delay(250);
      Serial.println(i);
    }
  }
  */
  
  /*
  tt = rtc.getTime();

  if (1) {
    Serial.print(tt->hour);
    Serial.print(":");
    Serial.print(tt->min);
    Serial.print(":");
    Serial.println(tt->sec);

    show_time(tt, true, 0);
  }
  */
  
  read_rtc(true);
  //clear_data();
  //print_digits(rot.getRawPosition(), 4);
  
  delay(1000);
  //gps.tick();
  //delay(10);
}

