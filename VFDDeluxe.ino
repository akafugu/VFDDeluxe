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
 * TODO:
 * - Use SQW interrupt to refresh clock (FEATURE)
 * - Rewrite direct port access to use pin to port mapping features
 * - Rewrite display file to be a class with more features to support effects (scroll/fade/etc.)
 * - Add support for Rotary encoder
 * - pin mapping header file
 * - board header file boards.h
 * - change features.h to globals.h
 *   (globals includes features.h, boards.h)
 * - serial slave feature
 * - integrate GPS menu stuff
 * - rewrite menu system into a more flexible class
 */

#include "global.h"

#include "display.h"
#include "display_nixie.h"
#include "button.h"
#include "pitches.h"
//#include "rotary.h"

#include <Wire.h>
#include <WireRtcLib.h>
#ifdef HAVE_MPL115A2
#include <MPL115A2.h>
#endif

//#include <Encoder.h>

//Encoder myEnc(6, 7);

#include "gps.h"
#include "flw.h"
#include "rgbled.h"

WireRtcLib rtc;
//GPS gps;

// Piezo
#define PIEZO 11

// Cached settings
uint8_t g_24h_clock = true;
uint8_t g_show_temp = true;
uint8_t g_show_dots = true;
uint8_t g_brightness = 5;
uint8_t g_volume = 0;
#ifdef HAVE_GPS
uint8_t g_gps_enabled = 96;
uint8_t g_region = 0;
uint8_t g_autodate = false;
#endif // HAVE_GPS

// Other globals
uint8_t g_has_flw = false;  // does the unit have an EEPROM with the FLW database?
uint8_t g_has_dots = false; // can current shield show dot (decimal points)
uint8_t g_alarming = false; // alarm is going off
uint8_t g_alarm_switch;
uint8_t g_show_special_cnt = 0;  // display something special ("time", "alarm", etc)
WireRtcLib::tm* tt = NULL; // for holding RTC values

volatile uint16_t g_rotary_moved_timer;

extern enum shield_t shield;
//Rotary rot;
#define TEMP_CORR -1

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
} menu_state_t;

menu_state_t menu_state = STATE_CLOCK;


/*
 * FIXME: more display modes
 *
 * Auto 1 - toggle between showing time/date/temp/etc. automatically
 * Auto 2 - same, but show secondary info (seconds on 4 digit displays, alternate display mode on other displays
 * (example: 8-digits: Auto 1: HH.MM.SS, Auto 2: HH-MM-SS)
 * temp - show temp and associated info
 * flw  - show FLW
 * 
 */

// display modes
typedef enum {
	MODE_NORMAL = 0, // normal mode: show time/seconds
	MODE_AMPM, // shows time AM/PM
	MODE_LAST,
	MODE_ALARM_TEXT,  // show "alarm" (wm)
	MODE_ALARM_TIME,  // show alarm time (wm)
} display_mode_t;

display_mode_t display_mode = MODE_NORMAL;

void initialize(void)
{
  // read eeprom
  // fixme: implement

  pinMode(PIEZO, OUTPUT);
  digitalWrite(PIEZO, LOW);

  // initialize button
  // fixme: change depending on HAVE_ROTARY define
  initialize_button(PinMap::button1, PinMap::button2);

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

  //rot.begin();

  sei();
  Wire.begin();

#ifdef HAVE_RGB_BACKLIGHT
  // fixme: refactor into backlight class
  pca9685_wake();
  
  // Turn off all LEDs
  for (uint8_t i = 0; i < 16; i++)
    pca9685_set_channel(i, 0);
#endif // HAVE_RGB_BACKLIGHT

  rtc.begin();
  rtc.runClock(true);  

  //rtc.setTime_s(16, 10, 0);
  //rtc_set_alarm_s(17,0,0);

#ifdef HAVE_SHIELD_AUTODETECT
  autodetect_shield();
#else
  set_shield(SHIELD, SHIELD_DIGITS); 
#endif

  display_init(PinMap::data, PinMap::clock, PinMap::latch, PinMap::blank, g_brightness);
  
  if (shield == SHIELD_IN14)
      init_nixie_6digit();

  //g_alarm_switch = get_alarm_switch();

  // set up interrupt for alarm switch
  /*
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18);
  */
  
  g_has_flw = has_eeprom();
  
    // setup UART for GPS
    gps_init(g_gps_enabled);
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

bool have_temp_sensor(void)
{
#ifdef HAVE_MPL115A2      
    return true;
#endif
#ifdef HAVE_HIH6121
    return true;
#endif

    return rtc.isDS3231();
}

void read_temp()
{
#ifdef HAVE_HIH6121
    // fixme: implement
#elif defined(HAVE_MPL115A2)
    MPL115A2.ReadSensor();
    MPL115A2.shutdown();

    float temp = MPL115A2.GetTemperature() + TEMP_CORR;
    
    int8_t t;
    uint8_t f;
    
    t = (int)temp;
    f = (int)((temp-t)*100);

    show_temp(t, f);
#else
    int8_t t;
    uint8_t f;
    rtc.getTemp(&t, &f);
    show_temp(t, f);
#endif
}

inline bool have_pressure_sensor()
{
#ifdef HAVE_MPL115A2
    return true;
#else
    return false;
#endif
}

void read_pressure()
{
#ifdef HAVE_MPL115A2
    MPL115A2.ReadSensor();
    MPL115A2.shutdown();

    float pressure = MPL115A2.GetPressure();

    show_pressure(pressure);
#endif
}

inline bool have_humidity_sensor()
{
#ifdef HAVE_HIH6121
    return true;
#else
    return false;
#endif
}

void read_humidity()
{
    //fixme: implement
    show_humidity(96);    
}

unsigned long offset = 0;
char current_word[6];

void read_flw()
{
    offset = get_word(offset, current_word);        
    set_string(current_word);
}

void read_rtc(bool show_extra_info)
{
    tt = rtc.getTime();
    if (tt == NULL) return;

/*
    if (have_temp_sensor() && tt->sec >= 31 && tt->sec <= 33)
        read_temp();
    else if (have_pressure_sensor() && tt->sec >= 34 && tt->sec <= 36)
        read_pressure();
    else if (have_humidity_sensor() && tt->sec >= 37 && tt->sec <= 39)
        read_humidity();
    else if (g_has_flw  && tt->sec >= 40 && tt->sec <= 59)
        read_flw();
    else
        show_time(tt, g_24h_clock, show_extra_info);
 */

    if (have_temp_sensor() && tt->sec >= 31 && tt->sec <= 33)
        show_time(tt, g_24h_clock, show_extra_info);
//        read_temp();
    else if (have_pressure_sensor() && tt->sec >= 34 && tt->sec <= 36)
        show_time(tt, g_24h_clock, show_extra_info);
//        read_pressure();
    else if (have_humidity_sensor() && tt->sec >= 37 && tt->sec <= 39)
        show_time(tt, g_24h_clock, show_extra_info);
//        read_humidity();
    else if (g_has_flw  && tt->sec >= 40 && tt->sec <= 59)
//        show_time(tt, g_24h_clock, show_extra_info);
        read_flw();
    else
        show_time(tt, g_24h_clock, show_extra_info);
}

void setup()
{
  //while (!Serial) ;
    
  Serial.begin(9600);
  Serial.println("VFD Deluxe");
  
#ifdef HAVE_MPL115A2
  MPL115A2.begin();
  MPL115A2.shutdown();
#endif // HAVE_MPL115A2

  initialize();
  
  //gps.begin();
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
  
  uint8_t hour = 0, min = 0, sec = 0;

  // Counters used when setting time
  int16_t time_to_set = 0;
  uint16_t button_released_timer = 0;
  uint16_t button_speed = 25;
	
  switch (shield) {
    case(SHIELD_IV6):
      set_string("IV-6");
      break;
    case(SHIELD_IV17):
      set_string("IV17");
      break;
    case(SHIELD_IV18):
      set_string("IV-18");
      break;
    case(SHIELD_IV22):
      set_string("IV22");
      break;
    default:
      break;
   }

  delay(500);
  set_string("--------");

  /*
  while (1) { 
    read_rtc(true);
    //clear_data();
    //print_digits(rot.getRawPosition(), 4);
  
    delay(1000);
    //gps.tick();
    //delay(10);
  }
  */
  
	while (1) {
		get_button_state(&buttons);
                //long pos = myEnc.read();
		
		// When alarming:
		// any button press cancels alarm
		if (g_alarming) {
			read_rtc(display_mode);  // read and display time (??)

			// fixme: if keydown is detected here, wait for keyup and clear state
			// this prevents going into the menu when disabling the alarm
			if (buttons.b1_keydown || buttons.b1_keyup || buttons.b2_keydown || buttons.b2_keyup) {
				buttons.b1_keyup = 0; // clear state
				buttons.b2_keyup = 0; // clear state
				g_alarming = false;
			}
			else {
				//alarm();	
			}
		}
		// If both buttons are held:
		//  * If the ALARM BUTTON SWITCH is on the LEFT, go into set time mode
		//  * If the ALARM BUTTON SWITCH is on the RIGHT, go into set alarm mode
		else if (menu_state == STATE_CLOCK && buttons.both_held) {
                        Serial.println("Both held");
    
			if (g_alarm_switch) {
				menu_state = STATE_SET_ALARM;
				show_set_alarm();
				rtc.getAlarm_s(&hour, &min, &sec);
				time_to_set = hour*60 + min;
			}
			else {
				menu_state = STATE_SET_CLOCK;
				show_set_time();		
				rtc.getTime_s(&hour, &min, &sec);
				time_to_set = hour*60 + min;
			}

			set_blink(true);
			
			// wait until both buttons are released
			while (1) {
				_delay_ms(50);
				get_button_state(&buttons);
				if (buttons.none_held)
					break;
			}
		}
		// Set time or alarm
		else if (menu_state == STATE_SET_CLOCK || menu_state == STATE_SET_ALARM) {
			// Check if we should exit STATE_SET_CLOCK or STATE_SET_ALARM
			if (buttons.none_held) {
				set_blink(true);
				button_released_timer++;
				button_speed = 1;
			}
			else {
				set_blink(false);
				button_released_timer = 0;
				button_speed++;
			}

			// exit mode after no button has been touched for a while
			if (button_released_timer >= 160) {
				set_blink(false);
				button_released_timer = 0;
				button_speed = 1;
				
				// fixme: should be different in 12h mode
				if (menu_state == STATE_SET_CLOCK)
					rtc.setTime_s(time_to_set / 60, time_to_set % 60, 0);
				else
					rtc.setAlarm_s(time_to_set / 60, time_to_set % 60, 0);

				menu_state = STATE_CLOCK;
			}
			
			// Increase / Decrease time counter
			if (buttons.b1_repeat) time_to_set+=(button_speed/100);
			if (buttons.b1_keyup)  time_to_set++;
			if (buttons.b2_repeat) time_to_set-=(button_speed/100);
			if (buttons.b2_keyup)  time_to_set--;

			if (time_to_set  >= 1440) time_to_set = 0;
			if (time_to_set  < 0) time_to_set = 1439;

			show_time_setting(time_to_set / 60, time_to_set % 60, 0);
		}
		// Left button enters menu
		else if (menu_state == STATE_CLOCK && buttons.b2_keyup) {
			menu_state = STATE_MENU_BRIGHTNESS;
			show_setting_int("BRIT", "BRITE", g_brightness, false);
			buttons.b2_keyup = 0; // clear state
		}
		// Right button toggles display mode
		else if (menu_state == STATE_CLOCK && buttons.b1_keyup) {
			display_mode = (display_mode_t)((int)display_mode + 1);
//			if (display_mode == MODE_ALARM_TEXT)  g_show_special_cnt = 100;  // show alarm text for 1 second
//			if (display_mode == MODE_ALARM_TIME)  g_show_special_cnt = 100;  // show alarm time for 1 second
			if (display_mode == MODE_LAST) display_mode = MODE_NORMAL;
			buttons.b1_keyup = 0; // clear state
		}
		else if (menu_state >= STATE_MENU_BRIGHTNESS) {
			if (buttons.none_held)
				button_released_timer++;
			else
				button_released_timer = 0;
			
			if (button_released_timer >= 35 ) {
//			if (button_released_timer >= 200) {  // 2 seconds (wm)
				button_released_timer = 0;
				menu_state = STATE_CLOCK;
			}
			
			switch (menu_state) {
				case STATE_MENU_BRIGHTNESS:
					if (buttons.b1_keyup) {
						g_brightness++;
						buttons.b1_keyup = false;
					
						if (g_brightness > 10) g_brightness = 1;
					
						//eeprom_update_byte(&b_brightness, g_brightness);
					
//						if (shield == SHIELD_IV17)  // wm ???
//							show_setting_string("BRIT", "BRITE", (g_brightness % 2 == 0) ? "  lo" : "  hi", true);
//						else
						show_setting_int("BRIT", "BRITE", g_brightness, true);
						set_brightness(g_brightness);
					}
					break;
				case STATE_MENU_24H:
					if (buttons.b1_keyup) {
						g_24h_clock = !g_24h_clock;
						//eeprom_update_byte(&b_24h_clock, g_24h_clock);
						
						show_setting_string("24H", "24H", g_24h_clock ? " on" : " off", true);
						buttons.b1_keyup = false;
					}
					break;
				case STATE_MENU_VOL:
					if (buttons.b1_keyup) {
						g_volume = !g_volume;
						//eeprom_update_byte(&b_volume, g_volume);
						
						show_setting_string("VOL", "VOL", g_volume ? " hi" : " lo", true);
						//piezo_init();
						//beep(1000, 1);
						buttons.b1_keyup = false;
					}
					break;
				case STATE_MENU_TEMP:
					if (buttons.b1_keyup) {
						g_show_temp = !g_show_temp;
						//eeprom_update_byte(&b_show_temp, g_show_temp);
						
						show_setting_string("TEMP", "TEMP", g_show_temp ? " on" : " off", true);
						buttons.b1_keyup = false;
					}
					break;
				case STATE_MENU_DOTS:
					if (buttons.b1_keyup) {
						g_show_dots = !g_show_dots;
						//eeprom_update_byte(&b_show_dots, g_show_dots);
						
						show_setting_string("DOTS", "DOTS", g_show_dots ? " on" : " off", true);
						buttons.b1_keyup = false;
					}
					break;
				default:
					break; // do nothing
			}

			if (buttons.b2_keyup) {
				menu_state = (menu_state_t)((int)menu_state + 1);
				
				// show temperature setting only when running on a DS3231
				if (menu_state == STATE_MENU_TEMP && !rtc.isDS3231()) menu_state = (menu_state_t)((int)menu_state + 1);

				// don't show dots settings for shields that have no dots
				if (menu_state == STATE_MENU_DOTS && !g_has_dots) menu_state = (menu_state_t)((int)menu_state + 1);

				if (menu_state == STATE_MENU_LAST) menu_state = STATE_MENU_BRIGHTNESS;
				
				switch (menu_state) {
					case STATE_MENU_BRIGHTNESS:
						show_setting_int("BRIT", "BRITE", g_brightness, false);
						break;
					case STATE_MENU_VOL:
						show_setting_string("VOL", "VOL", g_volume ? " hi" : " lo", false);
						break;
					case STATE_MENU_24H:
						show_setting_string("24H", "24H", g_24h_clock ? " on" : " off", false);
						break;
					case STATE_MENU_DOTS:
						show_setting_string("DOTS", "DOTS", g_show_dots ? " on" : " off", false);
						break;
					case STATE_MENU_TEMP:
						show_setting_string("TEMP", "TEMP", g_show_temp ? " on" : " off", false);
						break;
					default:
						break; // do nothing
				}
				
				buttons.b2_keyup = 0; // clear state
			}
		}
		else {
			if (g_show_special_cnt>0) {
				g_show_special_cnt--;
				if (g_show_special_cnt == 0)
					switch (display_mode) {
						case MODE_ALARM_TEXT:
							display_mode = MODE_ALARM_TIME;
							g_show_special_cnt = 100;
							break;
						case MODE_ALARM_TIME:
							display_mode = MODE_NORMAL;
							break;
						default:
							display_mode = MODE_NORMAL;
					}
			}
			// read RTC approx every 200ms  (wm)
			static uint16_t cnt = 0;
			if (cnt++ > 15) {
				read_rtc(display_mode);  // read RTC and display time
				cnt = 0;
				}
		}
		
		// fixme: alarm should not be checked when setting time or alarm
		if (g_alarm_switch && rtc.checkAlarm())
			g_alarming = true;

#ifdef HAVE_GPS
		if (g_gps_enabled)
			for (long i = 0; i < 2500; i++) {  // loop count set to create equivalent delay, and works at 9600 bps
				if (gpsDataReady())
					getGPSdata();  // get the GPS serial stream and possibly update the clock 
			}
		else
			_delay_ms(7);  // do something that takes about the same amount of time
#else
		_delay_ms(7);  // roughly 10 ms per loop
#endif
	}

}

