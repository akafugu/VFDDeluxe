/*
 * VFD Deluxe
 * (C) 2011-13 Akafugu Corporation
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
 * CREDITS:
 *
 * William Phelps
 *  - Arduino Timer interrupts
 *  - GPS functionality
 *  - DTS and Date setting
 *  - Misc improvements and new features
 *
 */

/* DONE:
 * Implement brightness PWM
 * resolve speaker/tone
*/

/*
 * TODO:
 * fix region
 * add GPS "sanity check"
 * fix default DST rules (where did they go?)
 * Test FLW
 * Refactor FLW
 * Add menu items for GPS etc.
 * Port Date scrolling function (from William's newest branch)
 * Implement show alarm time when flipping switch
 * Implement alarm
 * Port William's new menu system
 * Rewrite display file to be a class with more features to support effects (scroll/fade/etc.)
 * serial slave feature
 */

#include "global.h"
#include "global_vars.h"
#include "menu.h"

#include <Wire.h>
#include <WireRtcLib.h>

#include "display.h"
#include "display_nixie.h"
#include "button.h"
//#include "pitches.h"
//#include "rotary.h"

#ifdef HAVE_MPL115A2
#include <MPL115A2.h>
#endif

//#include <Encoder.h>

//Encoder myEnc(6, 7);

#include "adst.h"
#include "gps.h"
#include "flw.h"
#include "rgbled.h"

#ifdef HAVE_FLW
FourLetterWord flw;
#endif

WireRtcLib rtc;
//GPS gps;

// Piezo
#ifdef HAVE_ATMEGA328
#define PIEZO 10
#define PIEZO_GND 9
#elif defined(HAVE_LEONARDO)
// Digital 11, PB7 (PCINT7/OC0A/OC1C/#RTS)
#define PIEZO 11
#endif

uint8_t g_second_dots_on = true;
uint8_t g_alarm_switch;
#define MENU_TIMEOUT 20 // 20*100 ms = 2 seconds

uint8_t g_alarming = false;
bool g_update_rtc = true;
uint8_t g_show_special_cnt = 0;  // display something special ("time", "alarm", etc)
#define SHOW_TIMEOUT 10 // 10*100 ms = 1 seconds
WireRtcLib::tm* tt = NULL; // for holding RTC values

volatile uint16_t g_rotary_moved_timer;

extern enum shield_t shield;
//Rotary rot;
#define TEMP_CORR -1

struct BUTTON_STATE buttons;

extern menu_state_t g_menu_state;
bool menu_b1_first = false;

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
    MODE_NORMAL = 0,  // Time mode 1 (HH:MM/HH:MM:SS)
    MODE_AMPM,        // Time mode 2 (SS/HH-MM)
    MODE_LAST,
    MODE_ALARM_TEXT,  // Shows "ALRM" or "ALARM"
    MODE_ALARM_TIME,  // Shows Alarm time
    MODE_AUTO_DATE,   // Scrolls date across the screen
} display_mode_t;

display_mode_t display_mode = MODE_NORMAL;

//// Auto date
//String g_date_string; // string holding the date to scroll across the screen in ADATE mode
//uint8_t g_date_scroll_offset; // offset used when scrolling date across the screen

void initialize(void)
{
  // read eeprom
  // fixme: implement

//  pinMode(PIEZO, OUTPUT);
//  digitalWrite(PIEZO, LOW);

#ifdef HAVE_ATMEGA328
  pinMode(PIEZO_GND, OUTPUT);
  digitalWrite(PIEZO_GND, LOW);
#endif

  // initialize alarm switch
  pinMode(PinMap::alarm_switch, OUTPUT);
  digitalWrite(PinMap::alarm_switch, HIGH); // enable pullup
  g_alarm_switch = digitalRead(PinMap::alarm_switch);

  // fixme: move to button class?
  // Set switch as input and enable pullup
  //SWITCH_DDR  &= ~(_BV(SWITCH_BIT));
  //SWITCH_PORT |= _BV(SWITCH_BIT);

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
  detect_shield();
#else
  set_shield(SHIELD, SHIELD_DIGITS); 
#endif

  globals_init();
  display_init(PinMap::data, PinMap::clock, PinMap::latch, PinMap::blank, g_brightness);
  
#ifdef HAVE_FLW
  flw.begin();
  g_has_flw = flw.has_eeprom();
  Serial.print("has_eeprom = ");
  Serial.println(g_has_flw);
  
//  if(g_has_flw)
//    g_flw_enabled = FLW_ON;
//  flw.setCensored(g_flw_enabled == FLW_ON);
#else
  g_has_flw = false;
  g_flw_enabled = FLW_OFF;
#endif

#ifdef HAVE_NIXIE_SUPPORT
  if (shield == SHIELD_IN14 || shield == SHIELD_IN8_2)
      init_nixie_6digit();
#endif

  //g_alarm_switch = get_alarm_switch();

  // set up interrupt for alarm switch
  /*
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18);
  */
  
#ifdef HAVE_RTC_SQW
    // set up interrupt for RTC SQW
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT4); // RTC SQW
  
    rtc.SQWSetFreq(WireRtcLib::FREQ_1);
    rtc.SQWEnable(true);
#endif
  
#ifdef HAVE_GPS
    // setup UART for GPS
    gps_init(g_gps_enabled);
#endif // HAVE_GPS

  // initialize button
  // fixme: change depending on HAVE_ROTARY define
//  initialize_button(PinMap::button1, PinMap::button2);
  initialize_button(PinMap::button2, PinMap::button1);

}

#ifdef HAVE_RTC_SQW
// RTC SQW interrupt
ISR( PCINT0_vect )
{
    g_update_rtc = true;
    g_gps_updating = false;
}
#endif

uint8_t print_digits(uint8_t num, uint8_t offset);
//void clear_data(void);

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

void read_flw()
{
#ifdef HAVE_FLW
//    set_string(flw.get_word());

  char* foo = flw.get_word();
  Serial.println(foo);
  set_string(foo);
#endif
}

//void update_date_string(WireRtcLib::tm* t)
//{
//  if (!t) return;
//  
//  String temp;
//  
//  switch (g_date_format) {
//  case FORMAT_YMD:
//    temp.concat(t->year+2000);
//    temp.concat('-');
//    if (t->mon < 10) temp.concat('0');
//    temp.concat(t->mon);
//    temp.concat('-');
//    if (t->mday < 10) temp.concat('0');    
//    temp.concat(t->mday);
//    break;
//  case FORMAT_DMY:
//    if (t->mday < 10) temp.concat('0');
//    temp.concat(t->mday);
//    temp.concat('-');
//    if (t->mon < 10) temp.concat('0');
//    temp.concat(t->mon);
//    temp.concat('-');
//    temp.concat(t->year+2000);
//    break;
//  case FORMAT_MDY:
//    if (t->mon < 10) temp.concat('0');
//    temp.concat(t->mon);
//    temp.concat('-');
//    if (t->mday < 10) temp.concat('0');
//    temp.concat(t->mday);
//    temp.concat('-');
//    temp.concat(t->year+2000);
//    break;
//  }
//
//  temp.concat("    ");
//
//  g_date_string = temp;
//}

//void scroll_date()
//{
//  char buf[16];
//  g_date_string.toCharArray(buf, 16);
//  
//  set_string(buf + g_date_scroll_offset);
//  g_date_scroll_offset++;
//  
//  if (g_date_scroll_offset == 14) // fixme: check length of buffer to allow scrolling other messages
//    display_mode = MODE_NORMAL;
//}
//
//void start_date_scroll()
//{
//  display_mode = MODE_AUTO_DATE;
//  g_date_scroll_offset = 0;
//  scroll_date();
//}

void read_rtc(bool show_extra_info)
{
    tt = rtc.getTime();
    if (tt == NULL) {
//      Serial.println("no RTC!");  // temp
      return;
    }

#ifdef HAVE_AUTO_DST
    if (tt->sec % 10 == 0)  // check DST Offset every 10 seconds (60?)
        setDSToffset(g_DST_mode); 
        
        if ((tt->hour == 0) && (tt->min == 0) && (tt->sec == 0)) {  // MIDNIGHT!
//          Serial.println("Midnight");  // wbp debug
            g_DST_updated = false;
            if (g_DST_mode)
                DSTinit(tt, g_DST_Rules);  // re-compute DST start, end
        }
#endif // HAVE_AUTO_DST

    g_second_dots_on = (g_menu_state == STATE_CLOCK && display_mode == MODE_NORMAL && tt->sec % 2 == 0) ? true : false;
    
//    update_date_string(tt);

    if (have_temp_sensor() && tt->sec >= 31 && tt->sec <= 33)
        read_temp();
    else if (have_pressure_sensor() && tt->sec >= 34 && tt->sec <= 36)
        read_pressure();
    else if (have_humidity_sensor() && tt->sec >= 37 && tt->sec <= 39)
        read_humidity();
    else if (g_has_flw  && g_flw_enabled != FLW_OFF && tt->sec >= 40 && tt->sec <= 50)
        read_flw();
    else if (g_AutoDate && display_mode <= MODE_AMPM && tt->sec == 55) {
//        start_date_scroll();
//    else if (display_mode == MODE_AUTO_DATE)
//        scroll_date();
        scroll_speed(300);  // display date at 3 cps
	scroll_date(tt, g_date_format);  // show date from last rtc_get_time() call
        while (scrolling())
          wDelay(100); // wait a bit (temp)
    }
    else
        show_time(tt, g_24h_clock, show_extra_info);        
}

void set_date(uint8_t yy, uint8_t mm, uint8_t dd) {
  tt = rtc.getTime();
  tt->year = yy;
  tt->mon = mm;
  tt->mday = dd;
  rtc.setTime(tt);
  
#ifdef HAVE_AUTO_DST
  DSTinit(tt, g_DST_Rules);  // re-compute DST start, end for new date
  g_DST_updated = false;  // allow automatic DST adjustment again
  setDSToffset(g_DST_mode);  // set DSToffset based on new date
#endif // HAVE_AUTO_DST 
}

void setup()
{

//  for (int i=0; i<46; i++) {
//    pinMode(i, OUTPUT);  // set pins to OUTPUT to save power
//  }

//  tone(PinMap::piezo, NOTE_A5, 100);  // test tone
  tone(PinMap::piezo, 880, 100);  // test tone
//  _delay_ms(500);
    
  Serial.begin(9600);
#ifdef HAVE_SERIAL_DEBUG
  while (!Serial) ;
#endif
  _delay_ms(500); // temp
#ifdef HAVE_SERIAL_DEBUG
  while (!Serial) ;  // second time's the charm...
#endif
  Serial.println("VFD Deluxe");
  
#ifdef HAVE_MPL115A2
  MPL115A2.begin();
  MPL115A2.shutdown();
#endif // HAVE_MPL115A2

  initialize();
//  set_string("0000");

//  set_display(0);
//  tone(11, 440, 500);  // test tone
//  wDelay(500);
//  set_display(1);
	
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

  Serial.println("setup done");
  wDelay(1000);
  
  /*
  // test: write alphabet
  while (1) {
    for (int i = 'a'; i <= 'z'+1; i++) {
      set_char_at(i, 0);
      set_char_at(i+1, 1);
      set_char_at(i+2, 2);
      set_char_at(i+3, 3);

      if (get_digits() == 6) {
        set_char_at(i+4, 4);
        set_char_at(i+5, 5);
      }

      delay(1000);
      //Serial.println(i);
    }
  }
  */

}

void loop()
{
  unsigned long t1, t2;
  uint8_t hour = 0, min = 0, sec = 0;
  // Counters used when setting time
  int16_t time_to_set = 0;
  uint16_t button_released_timer = 0;
  uint16_t button_speed = 25;
  
  while (1) {
		t1 = wMillis();
		get_button_state(&buttons);

  if (buttons.b1_keyup)  tone(11, 440, 1);  
  if (buttons.b2_keyup)  tone(11, 880, 1);  

                //long pos = myEnc.read();
           
//                if (buttons.b1_keyup)
//                    Serial.println("Got buttons.b1_keyup");
//                if (buttons.b2_keyup)
//                    Serial.println("Got buttons.b2_keyup");
		
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
		else if (g_menu_state == STATE_CLOCK && buttons.both_held) {
                        //Serial.println("Both held");
    
			if (g_alarm_switch) {
				g_menu_state = STATE_SET_ALARM;
				show_set_alarm();
                                g_second_dots_on = false;
				rtc.getAlarm_s(&hour, &min, &sec);
				time_to_set = hour*60 + min;
			}
			else {
				g_menu_state = STATE_SET_CLOCK;
				show_set_time();		
                                g_second_dots_on = false;
				rtc.getTime_s(&hour, &min, &sec);
				time_to_set = hour*60 + min;
			}

			set_blink(true);
			
			// wait until both buttons are released
			while (1) {
				wDelay(50);
				get_button_state(&buttons);
				if (buttons.none_held)
					break;
			}
		}
		// Set time or alarm
		else if (g_menu_state == STATE_SET_CLOCK || g_menu_state == STATE_SET_ALARM) {
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
			if (button_released_timer >= MENU_TIMEOUT) {
				set_blink(false);
				button_released_timer = 0;
				button_speed = 1;
				
				// fixme: should be different in 12h mode
				if (g_menu_state == STATE_SET_CLOCK)
					rtc.setTime_s(time_to_set / 60, time_to_set % 60, 0);
				else
					rtc.setAlarm_s(time_to_set / 60, time_to_set % 60, 0);

				g_menu_state = STATE_CLOCK;
			}

			// Increase / Decrease time counter
			if (buttons.b1_repeat) time_to_set+=(button_speed/10);
			if (buttons.b1_keyup)  time_to_set++;
			if (buttons.b2_repeat) time_to_set-=(button_speed/10);
			if (buttons.b2_keyup)  time_to_set--;

			if (time_to_set  >= 1440) time_to_set = 0;
			if (time_to_set  < 0) time_to_set = 1439;

			show_time_setting(time_to_set / 60, time_to_set % 60, 0);
		}
		// Left button enters menu
		else if (g_menu_state == STATE_CLOCK && buttons.b2_keyup) {
                        first_menu_item();
			show_setting_int("BRIT", "BRITE", g_brightness, false);
			buttons.b2_keyup = 0; // clear state
		}
		// Right button toggles display mode
		else if (g_menu_state == STATE_CLOCK && buttons.b1_keyup) {
			display_mode = (display_mode_t)((int)display_mode + 1);
//			if (display_mode == MODE_ALARM_TEXT)  g_show_special_cnt = SHOW_TIMEOUT;  // show alarm text for 1 second
//			if (display_mode == MODE_ALARM_TIME)  g_show_special_cnt = SHOW_TIMEOUT;  // show alarm time for 1 second
			if (display_mode == MODE_LAST) display_mode = MODE_NORMAL;
			buttons.b1_keyup = 0; // clear state
		}
		else if (g_menu_state >= STATE_MENU_BRIGHTNESS) {
			if (buttons.none_held)
				button_released_timer++;
			else
				button_released_timer = 0;
			
			if (button_released_timer >= MENU_TIMEOUT) {
				button_released_timer = 0;
				g_menu_state = STATE_CLOCK;
			}

                        if (buttons.b1_keyup) {  // right button
                            menu(!menu_b1_first, true);
                            buttons.b1_keyup = false;
                            menu_b1_first = false;  // b1 not first time now
                        }
                        if (buttons.b2_keyup) {  // left button
                           menu_b1_first = true;  // reset first time flag
                           
                           next_menu_item();      
                           menu(false, false);
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
							g_show_special_cnt = SHOW_TIMEOUT;
							break;
						case MODE_ALARM_TIME:
							display_mode = MODE_NORMAL;
							break;
						default:
							display_mode = MODE_NORMAL;
					}
			}

#ifdef HAVE_RTC_SQW
                        if (g_update_rtc) {
                            g_update_rtc = false;
                            read_rtc(display_mode);  // read RTC and display time
                        }    
#else
			// read RTC approx ever other time thru loop (every 200ms)
			static uint8_t cnt = 0;
			if (++cnt%2) {
				read_rtc(display_mode);  // read RTC and display time
				}
#endif // HAVE_RTC_SQW
		}

                g_alarm_switch = digitalRead(PinMap::alarm_switch);

		// fixme: alarm should not be checked when setting time or alarm
		if (g_alarm_switch && rtc.checkAlarm())
			g_alarming = true;

#ifdef HAVE_GPS
        if (g_gps_enabled && g_menu_state == STATE_CLOCK) {
            if (gpsDataReady()) {
                parseGPSdata(gpsNMEA());  // get the GPS serial stream and possibly update the clock 
            }
        }
#endif
        while ((wMillis()-t1)<100) ; // wait until 100 ms since start of loop
    }

}

