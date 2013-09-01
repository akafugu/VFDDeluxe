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

#include "global.h"
#include "global_vars.h"

#include "display.h"
//#include "display_nixie.h"
#include "gps.h"

#include <Wire.h>
#include <WireRtcLib.h>

//void write_vfd_7seg(uint8_t digit, uint8_t segments);
//void write_vfd_standard(uint8_t digit, uint16_t segments);
//void write_vfd_16seg(uint8_t digit, uint16_t segments);

//void write_vfd_iv6(uint8_t digit, uint8_t segments);
//void write_vfd_iv17(uint8_t digit, uint16_t segments);
//void write_vfd_iv18(uint8_t digit, uint8_t segments);
//void write_vfd_iv22(uint8_t digit, uint8_t segments);

// nixie shields
//void write_nixie_6digit(uint8_t digit, uint8_t value);

//void write_vfd_8bit(uint8_t data);
//void clear_display(void);
//void clear_data();

//bool get_alarm_switch(void);

// see font-16seg.c
uint16_t calculate_segments_16(uint8_t character);
// see font-14seg.c
uint16_t calculate_segments_14(uint8_t character);
// see font-7seg.c
uint8_t calculate_segments_7(uint8_t character);

// HV5812 Data In (PF7 - A0)
#define DATA_HIGH DIRECT_PIN_HIGH(data_pin.reg, data_pin.bitmask)
#define DATA_LOW  DIRECT_PIN_LOW(data_pin.reg, data_pin.bitmask)

// HV5812 Clock (PF5 - A2)
#define CLOCK_HIGH DIRECT_PIN_HIGH(clock_pin.reg, clock_pin.bitmask)
#define CLOCK_LOW  DIRECT_PIN_LOW(clock_pin.reg, clock_pin.bitmask)

// HV5812 Latch / Strobe (PF6 - A1)
#define LATCH_ENABLE  DIRECT_PIN_LOW(latch_pin.reg, latch_pin.bitmask)
#define LATCH_DISABLE DIRECT_PIN_HIGH(latch_pin.reg, latch_pin.bitmask)

pin_direct_t data_pin;
pin_direct_t clock_pin;
pin_direct_t latch_pin;
pin_direct_t blank_pin;

enum shield_t shield = SHIELD_NONE;
uint8_t digits = 6;
uint8_t segments = 7;
byte dummy1;
volatile char data[16]; // Digit data
//uint8_t us_counter = 0; // microsecond counter
uint8_t multiplex_counter = 0;
uint8_t multiplex_limit = 8;
uint8_t reverse_display = false;
#ifdef HAVE_GPS
uint8_t gps_counter = 0;
#endif

volatile uint8_t _scrolling = false;
static char sData[32];  // scroll message - 25 chars plus 8 spaces
const uint8_t scroll_len = 32;
volatile uint16_t scroll_counter = 0;
uint16_t scroll_time = 300;  // a little over 3 chars/second
volatile uint8_t scroll_index = 0;
uint8_t scroll_limit = 0;

// globals from main.c
extern uint8_t g_alarm_switch;

// variables for controlling display blink
uint8_t blink;
uint16_t blink_counter = 0;
volatile uint8_t display_on = 1;

extern uint8_t g_second_dots_on;

// dots [bit 0~5]
volatile uint8_t dots = 0;

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

volatile uint16_t ms_counter = 0; // millisecond counter
volatile unsigned long _millis = 0;
unsigned long wMillis(void)
{
  unsigned long m;
  cli();
  m = _millis;
  sei();
  return m;
}

void wDelay(unsigned long ms)
{
  unsigned long t2 = wMillis() + ms;
  while (t2 != wMillis()) ;
}

void clear_display(void);
void clear_data(void);

uint8_t _brightness = 255;  // current brightness level 0-255
uint8_t brt_counter = 0;

void display_init(uint8_t data, uint8_t clock, uint8_t latch, uint8_t blank, uint8_t brightness)
{
    // outputs
    pinMode(data, OUTPUT);
    pinMode(clock, OUTPUT);
    pinMode(latch, OUTPUT);
    pinMode(blank, OUTPUT);

    data_pin.pin = data;
    data_pin.reg = PIN_TO_OUTPUT_REG(data);
    data_pin.bitmask = PIN_TO_BITMASK(data);

    clock_pin.pin = data;
    clock_pin.reg = PIN_TO_OUTPUT_REG(clock);
    clock_pin.bitmask = PIN_TO_BITMASK(clock);

    latch_pin.pin = latch;
    latch_pin.reg = PIN_TO_OUTPUT_REG(latch);
    latch_pin.bitmask = PIN_TO_BITMASK(latch);

    blank_pin.pin = blank;
    blank_pin.reg = PIN_TO_OUTPUT_REG(blank);
    blank_pin.bitmask = PIN_TO_BITMASK(blank);

	LATCH_ENABLE;

	clear_display();
	clear_data();

#ifdef HAVE_ATMEGA328
  // TIMER 2 overflow interrupt
  //cli();
  TCCR2B = 0;

  // enable Timer2 overflow interrupt:
  TIMSK2 |= (1<<TOIE2);
  // Set CS00 bit so timer runs at clock speed:
  TCCR2B |= (1<<CS22)|(1<<CS21); // Set Prescaler to clk/8 : 1 click = 1us. CS21=1
#elif defined(HAVE_LEONARDO)

// we use 2 timers - one set for PWM for brightness control, the other set to
// do interrupts for timing: display multiplex and millis();

  TCCR1B = (1<<WGM13) | (1<<WGM12);  // fast PWM
  TCCR1A = (1<<WGM11) | (1<<WGM10);  // set for OCR1=TOP
  OCR1A = 16000;  // 16000000/16000 = 1,000 hz
  TCNT1 = 0;
  TCCR1B |= (1<<CS10); // connect at 1x to start counter
  TIMSK1 |= (1<<OCIE1A); // enable Timer1 COMPA interrupt:

//  TCCR3B = (1<<WGM32);  // fast PWM for display brightness control
//  TCCR3A = (1<<WGM30);  // set for 8 bit
//  TCNT3 = 0;
//  TCCR3B |= (1<<CS30); // connect at 1x to start counter
//  TIMSK3 |= (1<<TOIE3); // enable Timer1 overflow interrupt:

// PWM on PD7/OC4D (digital 6)
//  TCCR4E = 0; 
  TCCR4D = 0; // (1<<WGM40); // fast pwm
  TCCR4C = (1<<COM4D1) | (1<<COM4D0) | (1<<PWM4D);
//  TCCR4B = 0;
//  TCCR4A = 0;
  OCR4C = 255;  // clear on compare match value
  OCR4D = 255;  // set maximum brightness
  TCNT4 = 0;  // start count
  TCCR4B = (1<<CS40); // connect CK at 1x to start counter
//  TIMSK4 |= (1<<OCIE4A); // no interrupt needed for Timer4

#endif
    
  set_brightness(brightness);
}

// Brightness is set by setting the PWM duty cycle for the blank
// pin of the VFD driver.
//byte brt[] = {2, 14, 27, 41, 58, 78, 103, 134, 179, 255};
byte brt[] = {3, 15, 27, 42, 59, 79, 103, 135, 179, 255};
// brightness value: 1 (low) - 10 (high)
// fixme: BLANK must always be set to GND when driving Nixies
void set_brightness(uint8_t brightness) {

  if (brightness > 10) brightness = 10;
//  _brightness = brt[brightness-1];
  OCR4D = brt[brightness-1];  // set PWM comparand for given brightness 
  TCNT4 = 0; // restart timer counter
//  digitalWrite(blank_pin.pin, LOW);  // blanking off
  PORTD &= B01111111; // set PD7 LOW

}

int get_digits(void)
{
  return digits;
}

#ifdef HAVE_SHIELD_AUTODETECT
// detect which shield is connected
void detect_shield()
{
    pinMode(PinMap::sig0, INPUT);
    pinMode(PinMap::sig1, INPUT);
    pinMode(PinMap::sig2, INPUT);
    
    // Turn on pull-ups
    digitalWrite(PinMap::sig0, HIGH);
    digitalWrite(PinMap::sig1, HIGH);
    digitalWrite(PinMap::sig2, HIGH);
    
    // read shield bits
    uint8_t sig = 
        ((digitalRead(PinMap::sig0) ? 0b1   : 0) |
         (digitalRead(PinMap::sig1) ? 0b10  : 0) |
         (digitalRead(PinMap::sig2) ? 0b100 : 0 ));    

//    Serial.print("Signature = ");
//    Serial.println(sig);

    switch (sig) {
    case(1):  // IV-17 shield
        shield = SHIELD_IV17;
        digits = 4;
        multiplex_limit = 4;
        //mpx_count = 4;
        g_has_dots = false;
        reverse_display = false;
        segments = 16;
        break;
    case(2):  // IV-6 shield
        shield = SHIELD_IV6;
        digits = 6;
        multiplex_limit = 6;
        //mpx_count = 8;
        g_has_dots = true;
        reverse_display = false;
        break;
        /*
    case(6):  // IV-22 shield
        shield = SHIELD_IV22;
        digits = 4;
        multiplex_limit = 4;
        //mpx_count = 8;
        g_has_dots = true;
        reverse_display = false;
        break;
        */
    case(7):  // IV-18 shield (note: save value as no shield - all bits on)
        shield = SHIELD_IV18;
        digits = 8;
        multiplex_limit = 9; // 8 digits plus dot/dash
        //mpx_count = 7; 
        g_has_dots = true;
        reverse_display = true;
        break;
    default:
        shield = SHIELD_NONE;
        break;
    }
}
#endif // HAVE_SHIELD_AUTODETECT

void set_shield(shield_t shield_type, uint8_t _digits /* = 4 */)
{
  if (shield_type == SHIELD_7SEG) {
    shield = SHIELD_7SEG;
    digits = _digits;
    multiplex_limit = 10;  // ???
    g_has_dots = true;
    reverse_display = true;
  }
#ifdef HAVE_14SEG_SUPPORT
  else if (shield_type == SHIELD_14SEG) {
    shield = SHIELD_14SEG;
    digits = _digits;
    g_has_dots = true;
    reverse_display = true;
    segments = 14;
  }
#endif
  else if (shield_type == SHIELD_16SEG) {
    shield = SHIELD_16SEG;
    digits = _digits;
    multiplex_limit = digits;
    g_has_dots = true;
    reverse_display = true;
    segments = 16;
  }
  else if (shield_type == SHIELD_IV6) {
    shield = SHIELD_IV6;
    digits = 6;
    multiplex_limit = digits;
    g_has_dots = true;
    reverse_display = false;
  }
  else if (shield_type == SHIELD_IV17) {
    shield = SHIELD_IV17;
    digits = 4;
    multiplex_limit = digits;
    g_has_dots = true;
    reverse_display = false;
    segments = 16;
  }
  else if (shield_type == SHIELD_IV18) {
    shield = SHIELD_IV18;
    digits = 8;
    multiplex_limit = digits+1;
    g_has_dots = true;    
    reverse_display = true;
  }
//  else if (shield_type == SHIELD_IV22) {
//    shield = SHIELD_IV22;
//    digits = 4;
//    multiplex_limit = digits;
//    g_has_dots = true;    
//    reverse_display = false;
//  }
  else if (shield_type == SHIELD_IN14) {
    shield = SHIELD_IN14;
    digits = 6;
    multiplex_limit = digits;
    g_has_dots = true;    
    reverse_display = true;
  }
  else if (shield_type == SHIELD_IN8_2) {
    shield = SHIELD_IN8_2;
    digits = 6;
    multiplex_limit = digits;
    g_has_dots = true;    
    reverse_display = true;
  }
}

void clear_data()
{
  for (uint8_t i = 0; i<16; i++) {
    data[i] = ' ';
  }
}

void clear_sData(void)
{
  for (int i = 0; i<scroll_len; i++) {
    sData[i] = ' ';
  }
}

void set_blink(bool on)
{
  blink = on;
  if (!blink) display_on = 1;
}

void set_display(bool on)
{
  display_on = on;
}

void flash_display(uint16_t ms)  // this does not work but why???
{
    display_on = false;
    clear_display();
//	_delay_ms(ms);
    wDelay(ms);
    display_on = true;
}

void button_timer(void);

// utility functions
// fixme: generalize this to print any length of number
uint8_t print_digits (int8_t num, uint8_t offset)
{
    uint8_t ret = offset+2;
    
    if (num < 0) {
        data[offset-1] = '-';  // note assumption that offset is always positive!
        num = -num;
    }

    if (num >= 100) {
        ret = offset+3;
        data[offset+2] = num % 10;
        num /= 10;
    }
    
    data[offset+1] = num % 10;
    num /= 10;
    data[offset] = num % 10;
    
    return ret;
}

uint8_t print_hour(uint8_t num, uint8_t offset, bool _24h_clock)
{
	data[offset+1] = num % 10;  // units
	//num /= 10;
	uint8_t h2 = num / 10 % 10;  // tens
	data[offset] = h2;
	if (!_24h_clock && (h2 == 0)) {
		data[offset] = ' ';  // blank leading zero
	}
	return offset+2;
}

uint8_t print_ch(char ch, uint8_t offset)
{
	data[offset++] = ch;
	return offset;
}

uint8_t print_strn(const char* str, uint8_t offset, uint8_t n)
{
	uint8_t i = 0;

	while (n-- >= 0) {
		data[offset++] = str[i++];
		if (str[i] == '\0') break;
	}

	return offset;
}

// set dots based on mode and seconds
void print_dots(uint8_t mode, uint8_t seconds)
{
	if (g_show_dots) {
  		if (digits == 10 && mode == 0) {
			sbi(dots, 3);
			sbi(dots, 5);
		}
  		else if (digits == 8 && mode == 0) {
			sbi(dots, 2);
			sbi(dots, 4);
		}
		else if (digits == 6 && mode == 0) {
			sbi(dots, 1);
			sbi(dots, 3);
		}
		else if (digits == 4 && seconds % 2 && mode == 0) {
			sbi(dots, 1);
		}
	}
}

// shows time based on mode
// 4 digits: hour:min / sec
// 6 digits: hour:min:sec / hour-min
// 8 digits: hour:min:sec / hour-min-sec
void show_time(WireRtcLib::tm* t, bool _24h_clock, uint8_t mode)
{
	dots = 0;

	uint8_t offset = 0;
//	uint8_t hour = _24h_clock ? t->hour : t->twelveHour;
	uint8_t hour = _24h_clock ? t->hour : t->hour%12;
        if (!_24h_clock && hour == 0)  // show 12 for midnight and noon
          hour = 12;  // wbp

	print_dots(mode, t->sec);

	if (mode == 0) { // normal display mode
//            nixie_print(hour, t->min, t->sec);

            if (digits == 10) { // "  HH.MM.SS  "
                offset = print_ch(' ', offset); 

                if (!_24h_clock && !t->am)
                    offset = print_ch('P', offset);
                else
                    offset = print_ch(' ', offset); 

                offset = print_hour(hour, offset, _24h_clock);
                offset = print_digits(t->min, offset);
                offset = print_digits(t->sec, offset);
                offset = print_ch(' ', offset);
                offset = print_ch(' ', offset);
            }
            else if (digits == 8) { // "P HH.MM.SS "
                if (!_24h_clock && !t->am)
                    offset = print_ch('P', offset);
                else
                    offset = print_ch(' ', offset);
                offset = print_ch(' ', offset); // shift time 1 space to rig
                offset = print_hour(hour, offset, _24h_clock);
                offset = print_digits(t->min, offset);
                offset = print_digits(t->sec, offset);
                offset = print_ch(' ', offset);
            }
            else if (digits == 6) { // "HH.MM.SS"
                offset = print_hour(hour, offset, _24h_clock);
                offset = print_digits(t->min, offset);
                offset = print_digits(t->sec, offset);			
            }
            else { // HH.MM
                offset = print_hour(hour, offset, _24h_clock);
                offset = print_digits(t->min, offset);
            }
	}
	else if (mode == 1) { // extra display mode
//            nixie_print_compact(hour, t->min, t->sec);

		if (digits == 10) { // " HH-MM-SS "
			offset = print_ch('-', offset);
			offset = print_hour(hour, offset, _24h_clock);
			offset = print_ch('-', offset);
			offset = print_digits(t->min, offset);
			offset = print_ch('-', offset);
			offset = print_digits(t->sec, offset);
			offset = print_ch('-', offset);

		}
		else if (digits == 8) { // "HH-MM-SS"
			offset = print_hour(hour, offset, _24h_clock);
			offset = print_ch('-', offset);
			offset = print_digits(t->min, offset);
			offset = print_ch('-', offset);
			offset = print_digits(t->sec, offset);
		}
		else if (digits == 6) { // " HH-MM"
			offset = print_hour(hour, offset, _24h_clock);
			offset = print_ch('-', offset);
			offset = print_digits(t->min, offset);
			if (!_24h_clock && !t->am)
				offset = print_ch('P', offset);
			else
				offset = print_ch(' ', offset); 
		}
		else { // HH.MM
			if (_24h_clock) {
				offset = print_ch(' ', offset);
				offset = print_digits(t->sec, offset);
				offset = print_ch(' ', offset);
			}
			else {
				if (t->am)
					offset = print_ch('A', offset);
				else
					offset = print_ch('P', offset);

				offset = print_ch('M', offset);
				offset = print_digits(t->sec, offset);
			}
		}
	}
}

// shows time - used when setting time
void show_time_setting(uint8_t hour, uint8_t min, uint8_t sec)
{
//	nixie_print_compact(hour, min, sec);

	dots = 0;
	uint8_t offset = 0;
	
	switch (digits) {
	case 8:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 6:
		offset = print_digits(hour, offset);
		offset = print_ch('-', offset);
		offset = print_digits(min, offset);
		offset = print_ch(' ', offset);
		break;
	case 4:
		offset = print_digits(hour, offset);
		offset = print_digits(min, offset);		
	}
}

void show_temp(int8_t t, uint8_t f)
{
	dots = 0;
	uint8_t offset = 0;

//	nixie_print(0, t, f);
	
	switch (digits) {
	case 10:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 8:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 6:
		offset = print_ch(' ', offset);
		offset = print_digits(t, offset);
		offset = print_digits(f, offset);
		offset = print_ch('C', offset);
		break;
	case 4:
		offset = print_digits(t, offset);
		offset = print_ch('&', offset);		
		offset = print_ch('C', offset);  
	}

  if (digits == 10) dots = (1<<3);
  else if (digits == 8) dots = (1<<3);
  else if (digits == 6) dots = (1<<2);
  else if (digits == 4) dots = 0;
}

void show_humidity(uint8_t hum)
{
	dots = 0;
	uint8_t offset = 0;
	
//	nixie_print(0, 0, hum);

	switch (digits) {
	case 10:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 8:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 6:
		offset = print_ch(' ', offset);
		offset = print_digits(hum, offset);
		offset = print_ch('R', offset);
		offset = print_ch('H', offset);
		break;
	case 4:
		offset = print_digits(hum, offset);
		offset = print_ch(' ', offset);
		offset = print_ch('H', offset);
	}
}

void show_pressure(uint8_t pressure)
{
	dots = 0;
	uint8_t offset = 0;
	
	switch (digits) {
	case 10:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 8:
		offset = print_ch(' ', offset);
		offset = print_ch(' ', offset);
		// fall-through
	case 6:
		offset = print_digits(pressure, offset);
		offset = print_ch('k', offset);
		offset = print_ch('P', offset);
		offset = print_ch('a', offset);
		break;
	case 4:
		offset = print_digits(pressure, offset);
		offset = print_ch('P', offset);
	}
}

void set_string(const char* str)
{
	if (!str) return;

	dots = 0;
//	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';
        clear_data();
	
	for (int i = 0; i <= digits-1; i++) {
		if (!*str) break;
		data[i] = *(str++);
	}
}

void set_scroll(char* str)
{
	uint8_t i = 0;
	if (!str) return;
	dots = 0;
	clear_sData();
	for (i = 0; i <25; i++) {
		if (!*str) break;
		sData[i] = *(str++);
	}
	scroll_limit = i+1;
	scroll_index = 0;

	scroll_counter = scroll_time;  // start scrolling
	_scrolling = true;
}

// shows setting string
void show_setting_string(const char* short_str, const char* long_str, const char* value, bool show_setting)
{
//	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';
	clear_data();

	if (get_digits() == 8) {
		set_string(short_str);
		print_strn(value, 4, 3);
	}
	else if (get_digits() == 6) {
		if (show_setting)
			print_strn(value, 2, 3);
		else
			set_string(long_str);
	}
	else {
		if (show_setting)
			print_strn(value, 0, 3);
		else
			set_string(short_str);
	}
}

#ifdef FEATURE_AUTO_DATE
// scroll the date - called every 100 ms
void scroll_date(WireRtcLib::tm* te_, uint8_t region)
{
	dots = 0;
//	uint8_t di;
	char sl;
	char sd[13]; // = "  03/14/1947";
	sd[0] = sd[1] = ' ';
//	clr_sData();
	if (shield == SHIELD_IV17)
		sl = '/';
	else
		sl = '-';
	switch (region) {
		case FORMAT_DMY:
			sd[2] = te_->mday / 10 + '0';
			sd[3] = te_->mday % 10 + '0';
			sd[4] = sd[7] = sl;
			sd[5] = te_->mon / 10 + '0';
			sd[6] = te_->mon % 10 + '0';
			sd[8] = '2';
			sd[9] = '0';
			sd[10] = te_->year / 10 + '0';
			sd[11] = te_->year % 10 + '0';
			break;
		case FORMAT_MDY:
			sd[2] = te_->mon / 10 + '0';
			sd[3] = te_->mon % 10 + '0';
			sd[4] = sd[7] = sl;
			sd[5] = te_->mday / 10 + '0';
			sd[6] = te_->mday % 10 + '0';
			sd[8] = '2';
			sd[9] = '0';
			sd[10] = te_->year / 10 + '0';
			sd[11] = te_->year % 10 + '0';
			break;
		case FORMAT_YMD:
		default:  
			sd[2] = '2';
			sd[3] = '0';
			sd[4] = te_->year / 10 + '0';
			sd[5] = te_->year % 10 + '0';
			sd[6] = sd[9] = sl;
			sd[7] = te_->mon / 10 + '0';
			sd[8] = te_->mon % 10 + '0';
			sd[10] = te_->mday / 10 + '0';
			sd[11] = te_->mday % 10 + '0';
			break;
		}
	sd[12] = 0;  // null terminate
//  Serial.print("date: "); Serial.println(sd);
	set_scroll(sd);
}
#endif

void show_setting_int(const char* short_str, const char* long_str, int value, bool show_setting)
{
//  Serial.print("show_setting_int ");
//  Serial.println(value);
  
//	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';
        clear_data();

	if (get_digits() == 8) {
		set_string(long_str);
		print_digits(value, 6);
	}
	else if (get_digits() == 6) {
		set_string(long_str);
		print_digits(value, 4);
	}
	else {
		if (show_setting)
			print_digits(value, 2);
		else
			set_string(short_str);
	}
}

void show_set_time(void)
{
	if (get_digits() == 8)
		set_string("Set Time");
	else if (get_digits() == 6)
		set_string(" Time ");
	else
		set_string("Time");
}

void show_set_alarm(void)
{
	if (get_digits() == 8)
		set_string("Set Alrm");
	else if (get_digits() == 6)
		set_string("Alarm");
	else
		set_string("Alrm");
}

// Write 8 bits to HV5812 driver
void write_vfd_8bit(uint8_t data)
{
	// shift out MSB first
	for (uint8_t i = 0; i < 8; i++)  {
		if (!!(data & (1 << (7 - i))))
			DATA_HIGH;
		else
			DATA_LOW;

		CLOCK_HIGH;
		CLOCK_LOW;
  }
}

void write_vfd_7seg(uint8_t digit, uint8_t segments)
{
	// temporary correction for incorrectly wired display
	uint8_t x = 0;
  
	if (segments & (1<<0)) x |= (1<<0);
	if (segments & (1<<1)) x |= (1<<1);
	if (segments & (1<<5)) x |= (1<<2);
	if (segments & (1<<6)) x |= (1<<3);
	if (segments & (1<<2)) x |= (1<<4);
	if (segments & (1<<4)) x |= (1<<5);
	if (segments & (1<<3)) x |= (1<<6);
	if (segments & (1<<7)) x |= (1<<7);
  
	segments = x;

	uint8_t segments_hi = 0;
  
	if (dots & (1<<digit))
		segments_hi |= (1<<0);
		//segments |= (1<<7); // DP is at bit 7
  
	write_vfd_8bit(segments_hi);
	write_vfd_8bit(segments);

	if (digit > 7) {
		write_vfd_8bit(1<<(digit-8));  
		write_vfd_8bit(0);
	}
	else {
		write_vfd_8bit(0);
		write_vfd_8bit(1<<digit);
	}
	
	LATCH_DISABLE;
	LATCH_ENABLE;
}

// fixme: need to determine placement of dots
// fixme: need to determine placement of second dot (if present)
void write_vfd_standard(uint8_t digit, uint16_t segments)
{
	uint16_t d = 1<<digit;

	write_vfd_8bit(segments >> 8);
        write_vfd_8bit(segments);
	write_vfd_8bit(d >> 8);
	write_vfd_8bit(d);

	LATCH_DISABLE;
	LATCH_ENABLE;
}

// Writes to the HV5812 driver for IV-6
// HV1~6:   Digit grids, 6 bits
// HV7~14:  VFD segments, 8 bits
// HV15~20: NC
void write_vfd_iv6(uint8_t digit, uint8_t segments)
{
	if (dots & (1<<digit))
		segments |= (1<<7); // DP is at bit 7
	
	uint32_t val = (1 << digit) | ((uint32_t)segments << 6);
	
	write_vfd_8bit(0); // unused upper byte: for HV518P only
	write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);
	
	LATCH_DISABLE;
	LATCH_ENABLE;	
}

#define IV17_LEFT_DOT  0b00010000
#define IV17_RIGHT_DOT 0b00100000

// Writes to the HV5812 driver for IV-17
// HV1~4:  Digit grids, 4 bits
// HV 5~2: VFD segments, 16-bits
void write_vfd_iv17(uint8_t digit, uint16_t segments)
{
	uint32_t val = (1 << digit) | ((uint32_t)segments << 4);

	//write_vfd_8bit(val >> 24);
        write_vfd_8bit(0);
        
        if (dots & (1<<digit))
	  write_vfd_8bit(val >> 16 | IV17_RIGHT_DOT);
        else
          write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);

	LATCH_DISABLE;
	LATCH_ENABLE;
}

uint32_t t;

// Writes to the HV5812 driver for IV-6
// HV1~10:  Digit grids, 10 bits
// HV11~18: VFD segments, 8 bits
// HV19~20: NC
void write_vfd_iv18(uint8_t digit, uint8_t segments)
{
	if (dots & (1<<digit))
		segments |= (1<<7); // DP is at bit 7
	
	uint32_t val = (1 << digit) | ((uint32_t)segments << 10);

	write_vfd_8bit(0); // unused upper byte: for HV518P only
	write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);
	
	LATCH_DISABLE;
	LATCH_ENABLE;	
}

// Writes to the HV5812 driver for IV-22
// HV1~4:   Digit grids, 4 bits
// HV5~6:   NC
// HV7~14:  VFD segments, 8 bits
// HV15~20: NC
void write_vfd_iv22(uint8_t digit, uint8_t segments)
{
	uint32_t val = (1 << digit) | ((uint32_t)segments << 6);
	
	write_vfd_8bit(0); // unused upper byte: for HV518P only
	write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);
	
	LATCH_DISABLE;
	LATCH_ENABLE;	
}

void write_nixie(uint8_t value1, uint8_t value2, uint8_t value3)
{
    uint32_t val = 0;
    val <<= 2;
    val |= value3 == 10 ? 0 : (1<<(uint32_t)value3);
    val <<= 10;
    val |= value2 == 10 ? 0 : (1<<(uint32_t)value2);
    val <<= 10;
    val |= value1 == 10 ? 0 : (1<<(uint32_t)value1);
    
    val = ~val;
    
    write_vfd_8bit(val >> 24);
    write_vfd_8bit(val >> 16);
    write_vfd_8bit(val >> 8);
    write_vfd_8bit(val);

    LATCH_DISABLE;
    LATCH_ENABLE;   
}

void write_nixie_dots()
{
    uint32_t val = 0;
    
    // fixme: dot printing for nixies should be based on the normal dots variable
    // and be dependent on the dots setting as well as display mode
    // (for example: compact display on 6-digit nixie shields will have no dots for
    // compact display mode)
    if (g_second_dots_on) {
        val |= ((uint32_t)1<<(uint32_t)30);
        val |= ((uint32_t)1<<(uint32_t)31);
    }

    val = ~val;

    write_vfd_8bit(val >> 24);
    write_vfd_8bit(val >> 16);
    write_vfd_8bit(val >> 8);
    write_vfd_8bit(val);

    LATCH_DISABLE;
    LATCH_ENABLE;   
}

void set_char_at(char c, uint8_t offset)
{
	data[offset] = c;
}

void clear_display(void)
{
	write_vfd_8bit(0);
	write_vfd_8bit(0);
	write_vfd_8bit(0);
	write_vfd_8bit(0);

	LATCH_DISABLE;
	LATCH_ENABLE;
}

volatile uint8_t nixie_multiplex_counter;

extern uint16_t segments_16[];

void display_multiplex(void)
{
    multiplex_counter++;	
    if (multiplex_counter > multiplex_limit)
        multiplex_counter = 0;  
//    clear_display();  // not needed?
//    nixie_multiplex_counter = !nixie_multiplex_counter;
    char d;
    if (_scrolling) {
      if (reverse_display)
        d = sData[digits-multiplex_counter-1+scroll_index];
      else
        d = sData[multiplex_counter+scroll_index];
    }
    else {
      if (reverse_display)
        d = data[digits-multiplex_counter-1];
      else
        d = data[multiplex_counter];
    }
    switch (shield) {
      case(SHIELD_IV6):
          write_vfd_iv6(multiplex_counter, calculate_segments_7(d));
          break;
      case(SHIELD_IV17): {
//          uint16_t seg = calculate_segments_16(d);
          uint16_t seg = segments_16[d];
          if (multiplex_counter == 0) {
            if (g_gps_updating)
            seg |= ((1<<5)|(1<<4)|(1<<13)|(1<<14)|(1<<15));
          }
          write_vfd_iv17(multiplex_counter, seg);
          break;
      }
      case(SHIELD_IV18): {
          uint8_t seg = 0;
          if (multiplex_counter < 8) {
              write_vfd_iv18(multiplex_counter, calculate_segments_7(d));
          }
          else { // show alarm switch & gps status
              if (g_alarm_switch)
                  seg |= (1<<7);
              if (g_gps_updating)
                  seg |= (1<<6);
              write_vfd_iv18(8, seg);
          }
          break;
      }
//      case(SHIELD_IV22):
//            write_vfd_iv22(multiplex_counter, calculate_segments_7(data[multiplex_counter]));
//      break;
#ifdef HAVE_7SEG_SUPPORT
      case(SHIELD_7SEG):
          write_vfd_7seg(multiplex_counter, calculate_segments_7(d));
      break;
#endif
#ifdef HAVE_14SEG_SUPPORT
      case(SHIELD_14SEG):
          uint16_t segments = calculate_segments_14(data[digits-multiplex_counter-1]);
          if ((segments & 1<<6) || (segments & 1<<8) || (segments & 1<<10)) // left dash is set (segment G1)
            segments |= 1<<12;
          write_vfd_standard(multiplex_counter, segments);
      break;
#endif
#ifdef HAVE_16SEG_SUPPORT
      case(SHIELD_16SEG):
      break;
    }
#endif
#ifdef HAVE_NIXIE_SUPPORT
      case(SHIELD_IN14):
          if (nixie_multiplex_counter)
            display_multiplex_in14();
      break;
      case(SHIELD_IN8_2):
          if (nixie_multiplex_counter)
            display_multiplex_in14();
      break;
    }
#endif
  }
}

uint16_t display_counter = 0;
uint16_t button_counter = 0;

#define BUTTON_TIMER_MAX 20  // 20 ms

#ifdef HAVE_ATMEGA328
ISR(TIMER2_OVF_vect)
#elif defined (HAVE_LEONARDO)
// Timer1 COMPA running at 1,000 hz
ISR(TIMER1_COMPA_vect)
#endif
{
#ifdef HAVE_ATMEGA328
  TCNT2 = 0x0;
#elif defined (HAVE_LEONARDO)
//    TCNT1 = 0xff00;
  TCNT1 = 0x0;
#endif
    _millis++;

    sei(); // enable interrupts during display mux to allow tone() to run
    if (display_on)  display_multiplex();
    cli();

    // control blinking: on time is slightly longer than off time
    if (blink) {
      blink_counter++;
      if (display_on && blink_counter >= 550) { // on time 0.55 secs
        display_on = false;
        blink_counter = 0;
        clear_display();
        }
      else if (!display_on && blink_counter >= 450) { // off time 0.45 secs
        display_on = true;
        blink_counter = 0;
      }
    }

#ifdef HAVE_GPS
  GPSread();  // check for data on the serial port every 1 ms
#endif // HAVE_GPS

    // button polling
    button_counter++;
    if (button_counter == BUTTON_TIMER_MAX) {
      button_timer();
      button_counter = 0;
    }

    if (_scrolling) {
      if (scroll_counter > 0)
        scroll_counter--;
      else {
        scroll_index++;
        scroll_counter = scroll_time;
        if (scroll_index==scroll_limit)
          _scrolling = false;
      }
    }

}

//// Timer3 OVF display PMWM
//ISR(TIMER3_OVF_vect)  // Timer3 for brightness PWM
//{
////  TCNT3 = 0;
//  brt_counter++;
//  if (brt_counter == 0)
////    digitalWrite(blank_pin.pin, LOW);  // blanking off
//    PORTD &= B01111111; // set PD7 LOW
//  else if (brt_counter == _brightness)  // once per pwm cycle
////    digitalWrite(blank_pin.pin, HIGH);  // blank display
//    PORTD |= B10000000; // set PD7 HIGH
//}
void scroll_stop(void)
{
  _scrolling = false;
}

uint8_t scrolling(void)
{
  return (_scrolling);
}

void scroll_speed(uint16_t speed)
{
  scroll_time = speed;
}

