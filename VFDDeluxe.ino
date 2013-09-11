/*
 * VFD Deluxe - Firmware for VFD Modular Clock mk2
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
 * fix region
 * Port Date scrolling function (from William's newest branch)
 * Implement show alarm time when flipping switch
 * Implement alarm
 * Test FLW
 * Refactor FLW
 * Add menu items for GPS etc.
*/

/*
 * TODO:
 * FLW mode when FLW is on/full
 * IV-18/8+ digit improvements:
 * - FLW movement
 * - Show "Alarm off" on one screen
 * - dot blinks when showing temperature
 * reveille alarm?
 * scroll time with date
 * add GPS "sanity check"
 * fix default DST rules (where did they go?)
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

#ifdef HAVE_HIH6121
#include <HIH61XX.h>
HIH61XX hih(0x27);
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

uint8_t g_second_dots_on = true;
uint8_t g_alarm_switch;
#define MENU_TIMEOUT 20 // 20*100 ms = 2 seconds

uint8_t g_alarming = false;
uint16_t snooze_count = 0; // alarm snooze counter
uint16_t alarm_timer = 0; // how long has alarm been beeping?
uint16_t alarm_count, alarm_cycle, beep_count, beep_cycle;
uint8_t g_snooze_time = 7; // snooze for 7 minutes

bool g_update_rtc = true;
uint8_t g_show_special_cnt = 0;  // display something special ("time", "alarm", etc)
#define SHOW_TIMEOUT 10 // 10*100 ms = 1 seconds
WireRtcLib::tm* tt = NULL; // for holding RTC values

volatile uint16_t g_rotary_moved_timer;

extern enum shield_t shield;
//Rotary rot;
#define TEMP_CORR -1

struct BUTTON_STATE buttons;

pin_direct_t switch_pin;

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
#ifdef HAVE_FLW
    MODE_FLW,         // Time mode 3: Shows FLW with time and date scrolling
#endif
    MODE_LAST,
    MODE_ALARM_TEXT,  // Shows "ALRM" or "ALARM"
    MODE_ALARM_TIME,  // Shows Alarm time
    MODE_AUTO_DATE,   // Scrolls date across the screen
    MODE_AUTO_FLW,    // Shows FLW for 5 seconds
    MODE_AUTO_TEMP,   // Shows temperature/humidity/pressure for 2 seconds each
    MODE_AUTO_TIME,   // Shows time for 5 seconds (for FLW mode)
} display_mode_t;

display_mode_t display_mode = MODE_NORMAL;
display_mode_t saved_display_mode = MODE_NORMAL;

void push_display_mode(display_mode_t d)
{
    saved_display_mode = display_mode;
    display_mode = d;
}

void pop_display_mode()
{
    display_mode = saved_display_mode;    
}

void initialize(void)
{
  pinMode(PinMap::piezo, OUTPUT);
  digitalWrite(PinMap::piezo, LOW);
  pinMode(PinMap::piezo2, OUTPUT);
  digitalWrite(PinMap::piezo2, LOW);

  // initialize alarm switch
  pinMode(PinMap::alarm_switch, INPUT_PULLUP);  // input with pullup
  
  switch_pin.pin = PinMap::alarm_switch;
  switch_pin.reg = PIN_TO_INPUT_REG(switch_pin.pin);
  switch_pin.bitmask = PIN_TO_BITMASK(switch_pin.pin);
  
  g_alarm_switch = DIRECT_PIN_READ(switch_pin.reg,  switch_pin.bitmask);

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
  
  if (rtc.isDS3231())
    Serial.println("Clock detected as DS3231");
  else
    Serial.println("Clock detected as DS1307");

  //rtc.setTime_s(16, 10, 0);
  //rtc_set_alarm_s(17,0,0);

#ifdef HAVE_SHIELD_AUTODETECT
  detect_shield();
#else
  set_shield(SHIELD, SHIELD_DIGITS); 
#endif

  globals_init();
  display_init(PinMap::data, PinMap::clock, PinMap::latch, PinMap::blank, g_brightness);

#ifdef HAVE_NIXIE_SUPPORT
  if (shield == SHIELD_IN14 || shield == SHIELD_IN8_2)
      init_nixie_6digit();
#endif

#ifdef HAVE_FLW
  flw.begin();
  for (uint8_t i = 0; i < 5; i++) // randomize starting point
      flw.get_word();
  g_has_flw = flw.has_eeprom();
  flw.setCensored(g_flw_enabled == FLW_ON);
#else
  g_has_flw = false;
  g_flw_enabled = FLW_OFF;
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
    hih.start();
    hih.update();
    float temp = hih.temperature();

    int8_t t;
    uint8_t f;
    
    t = (int)temp;
    f = (int)((temp-t)*100);

    show_temp(t, f);
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
#ifdef HAVE_HIH6121
    hih.start();
    hih.update();
    int hum = (int)hih.humidity();
    show_humidity(hum);
#endif
}

void read_flw()
{
#ifdef HAVE_FLW
  static uint8_t flw_counter = 0;
  static uint8_t flw_offset = 0;
  static int8_t flw_offset_direction = 1;
  
  if (flw_counter++ == 2) {
    set_string(flw.get_word(), flw_offset);
    flw_counter = 0;
    
    if (get_digits() > 4)
        flw_offset += flw_offset_direction;
    
    if (flw_offset_direction == 1 && flw_offset > get_digits()-5)
        flw_offset_direction = -1;
    else if (flw_offset_direction == -1 && flw_offset == 0)
        flw_offset_direction = 1;
  }  
#endif
}

void update_display()
{
    tt = rtc.getTime();
    if (tt == NULL) {
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

    if (display_mode == MODE_ALARM_TEXT) {
        if (get_digits() == 4) set_string("ALRM");
        else set_string("ALARM");
    }
    else if (display_mode == MODE_ALARM_TIME) {
        if (g_alarm_switch) {
            tt = rtc.getAlarm();
            show_time(tt, g_24h_clock, false);
        }
        else {
            set_string("OFF");          
        }
    }
    else if (display_mode == MODE_AUTO_TEMP) {
        read_temp(); // fixme, encapsulate temp/pressure/humidity here
        if (tt->sec >= 33) pop_display_mode();
    }
    else if (have_temp_sensor() && g_show_temp && tt->sec == 30) {
        push_display_mode(MODE_AUTO_TEMP);
        read_temp();
        // fixme: implement temp sub mode 0-temp,1-pressure,2-humidity
    }
    else if (display_mode == MODE_AUTO_FLW) {
       read_flw();    
       if (tt->sec >= 50) pop_display_mode();
    }
    else if (g_has_flw && g_flw_enabled != FLW_OFF && tt->sec == 40) {
        read_flw();
        push_display_mode(MODE_AUTO_FLW);
    }
    else if (g_has_flw && display_mode == MODE_FLW && tt->sec == 58) {
        push_display_mode(MODE_AUTO_TIME);
        show_time(tt, g_24h_clock, MODE_NORMAL);   
    }
    else if (display_mode == MODE_AUTO_TIME) {
        show_time(tt, g_24h_clock, MODE_NORMAL);
       if (tt->sec >= 4 && tt->sec < 5) pop_display_mode();
    }
    else if (g_AutoDate && display_mode < MODE_LAST && tt->sec == 55) {
        scroll_speed(300);  // display date at 3 cps
	scroll_date(tt, g_date_format);  // show date from last rtc_get_time() call

        push_display_mode(MODE_AUTO_DATE);
    }
#ifdef HAVE_FLW
    else if (display_mode == MODE_FLW) {
        read_flw();
    }
#endif
    else {
        show_time(tt, g_24h_clock, display_mode);   
    } 
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

void start_alarm(void)
{
  g_alarming = true;
  snooze_count = 0;
  alarm_cycle = 200;  // 20 second initial cycle
  alarm_count = 0;  // reset cycle count
  beep_cycle = 1;  // start with single beep
  alarm_timer = 0;  // restart start alarm timer
  set_blink(true);
}

void stop_alarm(void)
{
  g_alarming = false;  // stop alarm
  snooze_count = 0;  // and snooze
  set_blink(false);  // and blink
}

void alarm(void)
{
  tone(PinMap::piezo, 880, 100);  // test tone
}

void setup()
{

//  for (int i=0; i<46; i++) {
//    pinMode(i, OUTPUT);  // set pins to OUTPUT to save power
//  }

//  tone(PinMap::piezo, NOTE_A5, 100);  // test tone
  tone(PinMap::piezo, 880, 100);  // test tone
//  _delay_ms(500);
    
#ifdef HAVE_SERIAL_DEBUG
  while (!Serial) ;
#endif
  _delay_ms(500); // temp
#ifdef HAVE_SERIAL_DEBUG
  while (!Serial) ;  // second time's the charm...
#endif

  Serial.begin(9600);
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
  unsigned long t1;
  uint8_t hour = 0, min = 0, sec = 0;
  // Counters used when setting time
  int16_t time_to_set = 0;
  uint16_t button_released_timer = 0;
  uint16_t button_speed = 25;
  
  push_display_mode(MODE_NORMAL);
  
  while (1) {
		t1 = wMillis();
		get_button_state(&buttons);

                if (buttons.b1_keyup)  tone(11, 1000, 1);  
                if (buttons.b2_keyup)  tone(11, 1000, 1);  

		if (snooze_count>0)
			snooze_count--;

		if (g_alarming) {
			alarm_timer ++;
			if (alarm_timer > 30*60*10) {  // alarm has been sounding for 30 minutes, turn it off
				stop_alarm();
			}
		}

		// When alarming: any button press snoozes alarm
		if (g_alarming /*&& (snooze_count==0)*/) {
			update_display();

			// fixed: if keydown is detected here, wait for keyup and clear state
			// this prevents going into the menu when disabling the alarm 
			if (buttons.b1_keydown || buttons.b1_keyup || buttons.b2_keydown || buttons.b2_keyup) {
			    buttons.b1_keyup = 0; // clear state
		            buttons.b2_keyup = 0; // clear state

                            if (g_snooze_enabled) {    
                                start_alarm();  // restart alarm sequence
		                snooze_count = g_snooze_time*60*10;  // start snooze timer
			        if (get_digits() == 8)
                                    set_string(" Snooze ");
                                else if (get_digits() == 6)
                                    set_string("Snooze");
			        else
		                    set_string("snze");
			    
                                wDelay(500);
			    
                                while (buttons.b1_keydown || buttons.b2_keydown) {  // wait for button to be released
	                            wDelay(100);
				    get_button_state(&buttons);
                                }
                            }
                            else {
                                stop_alarm();
                                pop_display_mode();
            			update_display();
                            }
			}
			else {
                            if (g_alarmtype == ALARM_PROGRESSIVE) {
				alarm_count++;
				if (alarm_count > alarm_cycle) {  // once every alarm_cycle
					beep_count = alarm_count = 0;  // restart cycle 
					if (alarm_cycle>20)  // if more than 2 seconds
						alarm_cycle = alarm_cycle - 20;  // shorten delay by 2 seconds
					if (beep_cycle<20)
						beep_cycle += 2;  // add another beep
				}
				beep_count++;
				if (beep_count <= beep_cycle) {  // how many beeps this cycle?
					if (beep_count%2)  // odd = beep
						alarm();
				}
                            }
                            else {
 				beep_count++;
                                if (beep_count % 2) alarm();
                            }
			}
		} // g_alarming

		// If both buttons are held:
		//  * If the ALARM BUTTON SWITCH is on the LEFT, go into set time mode
		//  * If the ALARM BUTTON SWITCH is on the RIGHT, go into set alarm mode
		else if (g_menu_state == STATE_CLOCK && buttons.both_held) {
                        //Serial.println("Both held");
                        stop_alarm();  // setting time or alarm, cancel alarm
    
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
                else if (g_menu_state == STATE_CLOCK && display_mode == MODE_AUTO_TEMP && (buttons.b1_keyup || buttons.b2_keyup)) {
                    pop_display_mode();
                    update_display();
                }
                else if (g_menu_state == STATE_CLOCK && display_mode == MODE_AUTO_FLW && (buttons.b1_keyup || buttons.b2_keyup)) {
                    pop_display_mode();
                    update_display();
                }
                else if (g_menu_state == STATE_CLOCK && display_mode == MODE_AUTO_TIME && (buttons.b1_keyup || buttons.b2_keyup)) {
                    pop_display_mode();
                    update_display();
                }                
                else if (g_menu_state == STATE_CLOCK && display_mode == MODE_AUTO_DATE && !scrolling()) {
                    pop_display_mode();
                    update_display();
                }
                else if (g_menu_state == STATE_CLOCK && display_mode == MODE_AUTO_DATE && (buttons.b1_keyup || buttons.b2_keyup)) {
        	    scroll_stop();

                    pop_display_mode();
                    update_display();
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
        
#ifdef HAVE_FLW
                        if (!g_has_flw) display_mode = (display_mode_t)((int)display_mode + 1); // skip if no EEPROM
#endif

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
                            update_display();  // read RTC and display time
                        }    
#else
			// read RTC approx ever other time thru loop (every 200ms)
			static uint8_t cnt = 0;
			if (++cnt%2) {
				update_display();  // read RTC and display time
				}
#endif // HAVE_RTC_SQW
		}

                uint8_t sw = DIRECT_PIN_READ(switch_pin.reg,  switch_pin.bitmask);
                
                if (sw != g_alarm_switch) {
                    g_alarm_switch = sw;
                    if (!g_alarm_switch)
			stop_alarm();  // cancel alarm
                    display_mode = MODE_ALARM_TEXT;
                    g_show_special_cnt = 10;
                    g_update_rtc = true;
                }

		// fixme: alarm should not be checked when setting time or alarm
		if (g_alarm_switch && rtc.checkAlarm())
			start_alarm();

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

