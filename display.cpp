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
#include "display.h"
#include "display_nixie.h"
#include "gps.h"

#include <Wire.h>
#include <WireRtcLib.h>

void write_vfd_7seg(uint8_t digit, uint8_t segments);
void write_vfd_standard(uint8_t digit, uint16_t segments);
void write_vfd_16seg(uint8_t digit, uint16_t segments);

void write_vfd_iv6(uint8_t digit, uint8_t segments);
void write_vfd_iv17(uint8_t digit, uint16_t segments);
void write_vfd_iv18(uint8_t digit, uint8_t segments);
void write_vfd_iv22(uint8_t digit, uint8_t segments);

// nixie shields
void write_nixie_6digit(uint8_t digit, uint8_t value);

void write_vfd_8bit(uint8_t data);
void clear_display(void);
void clear_data();

bool get_alarm_switch(void);

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
volatile char data[16]; // Digit data
uint8_t us_counter = 0; // microsecond counter
uint8_t multiplex_counter = 0;
#ifdef HAVE_GPS
uint8_t gps_counter = 0;
#endif

// globals from main.c
extern uint8_t g_show_dots;
extern uint8_t g_has_dots;
extern uint8_t g_alarm_switch;
extern uint8_t g_brightness;

// variables for controlling display blink
uint8_t blink;
uint16_t blink_counter = 0;
uint8_t display_on = 1;

extern uint8_t g_second_dots_on;

uint8_t gps_updated;

// dots [bit 0~5]
uint8_t dots = 0;

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

int get_digits(void)
{
	return digits;
}

#ifdef HAVE_SHIELD_AUTODETECT
// detect which shield is connected
void detect_shield(void)
{
	// read shield bits
	uint8_t sig = 	
		(((SIGNATURE_PIN & _BV(SIGNATURE_BIT_0)) ? 0b1   : 0) |
		 ((SIGNATURE_PIN & _BV(SIGNATURE_BIT_1)) ? 0b10  : 0) |
		 ((SIGNATURE_PIN & _BV(SIGNATURE_BIT_2)) ? 0b100 : 0 ));

	switch (sig) {
		case(0):  // 6-digit Nixie shield
			break;
		case(1):  // IV-17 shield
			shield = SHIELD_IV17;
			digits = 4;
			mpx_count = 4;
			g_has_dots = false;
			break;
		case(2):  // IV-6 shield
			shield = SHIELD_IV6;
			digits = 6;
			mpx_count = 8;
			g_has_dots = true;
			break;
		case(3):  // IV-11 shield
			break;
		case(4):  // RESERVED
			break;
		case(5):  // RESERVED
			break;
		case(6):  // IV-22 shield
			shield = SHIELD_IV22;
			digits = 4;
			mpx_count = 8;
			g_has_dots = true;
			break;
		case(7):  // IV-18 shield (note: same value as no shield - all bits on)
			shield = SHIELD_IV18;
			digits = 8;
			mpx_count = 7; 
			g_has_dots = true;
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
    g_has_dots = true;
  }
  else if (shield_type == SHIELD_14SEG) {
    shield = SHIELD_14SEG;
    digits = _digits;
    g_has_dots = true;
  }
  else if (shield_type == SHIELD_16SEG) {
    shield = SHIELD_16SEG;
    digits = _digits;
    g_has_dots = true;
  }
  else if (shield_type == SHIELD_IV6) {
    shield = SHIELD_IV6;
    digits = 6;
    g_has_dots = true;
  }
  else if (shield_type == SHIELD_IV17) {
    shield = SHIELD_IV17;
    digits = 4;
    g_has_dots = true;
  }
  else if (shield_type == SHIELD_IV18) {
    shield = SHIELD_IV18;
    digits = 8;
    g_has_dots = true;    
  }
  else if (shield_type == SHIELD_IV22) {
    shield = SHIELD_IV22;
    digits = 4;
    g_has_dots = true;    
  }
  else if (shield_type == SHIELD_IN14) {
    shield = SHIELD_IN14;
    digits = 6;
    g_has_dots = true;    
  }
  else if (shield_type == SHIELD_IN8_2) {
    shield = SHIELD_IN8_2;
    digits = 6;
    g_has_dots = true;    
  }
}

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
  // TIMER 3 overflow interrupt
  //cli();             // disable global interrupts
  TCCR3A = 0;        // set entire TCCR1A register to 0
  TCCR3B = 0;
 
  // enable Timer3 overflow interrupt:
  TIMSK3 = (1 << TOIE3);
  // Set CS30 bit so timer runs at clock speed:
  TCCR3B |= (1 << CS30);
#endif

  // enable global interrupts:
  //sei();
    
  //set_brightness(brightness);
  digitalWrite(blank_pin.pin, LOW);

  //analogWrite(blank_pin.pin, 255);
}

// brightness value: 1 (low) - 10 (high)
// fixme: BLANK must always be set to GND when driving Nixies
void set_brightness(uint8_t brightness) {
	// workaround: IV17 shield not compatible with PWM dimming method
	// using simple software dimming instead
	if (shield == SHIELD_IV17) {
		return;
	}

	if (brightness > 10) brightness = 10;
	brightness = (10 - brightness) * 25; // translate to PWM value

/*
	// Brightness is set by setting the PWM duty cycle for the blank
	// pin of the VFD driver.
	// 255 = min brightness, 0 = max brightness 
	OCR0A = brightness;

	// fast PWM, fastest clock, set OC0A (blank) on match
	TCCR0A = _BV(WGM00) | _BV(WGM01);  
 
	TCCR0A |= _BV(COM0A1);
*/
}

void clear_data()
{
  data[0] = data[1] =  data[2] =  data[3] =  data[4] =  data[5] =  data[6] = data[7]  = ' '; 
  data[8] = data[9] = data[10] = data[11] = data[12] = data[13] = data[14] = data[15] = ' '; 
}

void set_blink(bool on)
{
	blink = on;
	if (!blink) display_on = 1;
}

void flash_display(uint16_t ms)  // this does not work but why???
{
	display_on = false;
	_delay_ms(ms);
	display_on = true;
}

void set_gps_updated(bool b) {
    if (shield == SHIELD_IV18)
        gps_updated = b;
    else
        set_blink(b);
}

//bool led = true;

void display_multiplex_7seg(void)
{
  clear_display();
  write_vfd_7seg(multiplex_counter, calculate_segments_7(display_on ? data[digits-multiplex_counter-1] : ' '));
	
  multiplex_counter++;	
  if (multiplex_counter == 10) multiplex_counter = 0;
}

void display_multiplex_14seg(void)
{
  clear_display();
  
  uint16_t segments = calculate_segments_14(display_on ? data[digits-multiplex_counter-1] : ' ');
  
  if ((segments & 1<<6) || (segments & 1<<8) || (segments & 1<<10)) // left dash is set (segment G1)
    segments |= 1<<12;
  
  write_vfd_standard(multiplex_counter, segments);
	
  multiplex_counter++;	
  if (multiplex_counter == digits+1) multiplex_counter = 0;  
}

void display_multiplex_16seg(void)
{
  
}

int slow = 0;
int counter = 'A';

// display multiplexing routine for 4 digits: run once every 5us
void display_multiplex_iv17(void)
{
	if (multiplex_counter == 0) {
		clear_display();
		write_vfd_iv17(0, calculate_segments_16(display_on ? data[0] : ' '));
	}
	else if (multiplex_counter == 1) {
		clear_display();
		write_vfd_iv17(1, calculate_segments_16(display_on ? data[1] : ' '));
	}
	else if (multiplex_counter == 2) {
		clear_display();
		write_vfd_iv17(2, calculate_segments_16(display_on ? data[2] : ' '));
	}
	else if (multiplex_counter == 3) {
		clear_display();
		write_vfd_iv17(3, calculate_segments_16(display_on ? data[3] : ' '));
	}
	else {
		clear_display();
	}

	multiplex_counter++;

	// high brightness
	if (multiplex_counter == 4 && g_brightness % 2 == 1) multiplex_counter = 0;
	// low brightness
	if (multiplex_counter == 8) multiplex_counter = 0;
}

// display multiplexing routine for IV6 shield: run once every 5us
void display_multiplex_iv6(void)
{
	if (multiplex_counter == 0) {
		clear_display();
		write_vfd_iv6(0, calculate_segments_7(display_on ? data[0] : ' '));
	}
	else if (multiplex_counter == 1) {
		clear_display();
		write_vfd_iv6(1, calculate_segments_7(display_on ? data[1] : ' '));
	}
	else if (multiplex_counter == 2) {
		clear_display();
		write_vfd_iv6(2, calculate_segments_7(display_on ? data[2] : ' '));
	}
	else if (multiplex_counter == 3) {
		clear_display();
		write_vfd_iv6(3, calculate_segments_7(display_on ? data[3] : ' '));
	}
	else if (multiplex_counter == 4) {
		clear_display();
		write_vfd_iv6(4, calculate_segments_7(display_on ? data[4] : ' '));
	}
	else if (multiplex_counter == 5) {
		clear_display();
		write_vfd_iv6(5, calculate_segments_7(display_on ? data[5] : ' '));
	}
	else {
		clear_display();
	}
	
	multiplex_counter++;
	
	if (multiplex_counter == 6) multiplex_counter = 0;
}

// display multiplexing routine for IV6 shield: run once every 5us
void display_multiplex_iv18(void)
{
	if (multiplex_counter == 0) {
		clear_display();
		write_vfd_iv18(0, calculate_segments_7(display_on ? data[7] : ' '));
	}
	else if (multiplex_counter == 1) {
		clear_display();
		write_vfd_iv18(1, calculate_segments_7(display_on ? data[6] : ' '));
	}
	else if (multiplex_counter == 2) {
		clear_display();
		write_vfd_iv18(2, calculate_segments_7(display_on ? data[5] : ' '));
	}
	else if (multiplex_counter == 3) {
		clear_display();
		write_vfd_iv18(3, calculate_segments_7(display_on ? data[4] : ' '));
	}
	else if (multiplex_counter == 4) {
		clear_display();
		write_vfd_iv18(4, calculate_segments_7(display_on ? data[3] : ' '));
	}
	else if (multiplex_counter == 5) {
		clear_display();
		write_vfd_iv18(5, calculate_segments_7(display_on ? data[2] : ' '));
	}
	else if (multiplex_counter == 6) {
		clear_display();
		write_vfd_iv18(6, calculate_segments_7(display_on ? data[1] : ' '));
	}
	else if (multiplex_counter == 7) {
		clear_display();
		write_vfd_iv18(7, calculate_segments_7(display_on ? data[0] : ' '));
	}
	else if (multiplex_counter == 8) {
		clear_display();

                uint8_t seg = 0;

		if (g_alarm_switch)
                    seg |= (1<<7);
                if (gps_updated)
                    seg |= (1<<6);

		write_vfd_iv18(8, seg);
	}
	else {
		clear_display();
	}
	
	multiplex_counter++;
	
	if (multiplex_counter == 9) multiplex_counter = 0;
}

// display multiplexing routine for IV6 shield: run once every 5us
void display_multiplex_iv22(void)
{
	if (multiplex_counter == 0) {
		clear_display();
		write_vfd_iv6(0, calculate_segments_7(display_on ? data[0] : ' '));
	}
	else if (multiplex_counter == 1) {
		clear_display();
		write_vfd_iv6(1, calculate_segments_7(display_on ? data[1] : ' '));
	}
	else if (multiplex_counter == 2) {
		clear_display();
		write_vfd_iv6(2, calculate_segments_7(display_on ? data[2] : ' '));
	}
	else if (multiplex_counter == 3) {
		clear_display();
		write_vfd_iv6(3, calculate_segments_7(display_on ? data[3] : ' '));
	}
	else {
		clear_display();
	}

	multiplex_counter++;

	if (multiplex_counter == 4) multiplex_counter = 0;
}

volatile uint8_t nixie_multiplex_counter;

void display_multiplex(void)
{
    nixie_multiplex_counter = !nixie_multiplex_counter;
    
	if (shield == SHIELD_7SEG)
		display_multiplex_7seg();
	else if (shield == SHIELD_14SEG)
		display_multiplex_14seg();
	else if (shield == SHIELD_16SEG)
		display_multiplex_16seg();
	else if (shield == SHIELD_IV6)
		display_multiplex_iv6();
	else if (shield == SHIELD_IV17)
		display_multiplex_iv17();
	else if (shield == SHIELD_IV18)
		display_multiplex_iv18();
	else if (shield == SHIELD_IV22)
		display_multiplex_iv22();
	else if (shield == SHIELD_IN14 && nixie_multiplex_counter)
		display_multiplex_in14();
	else if (shield == SHIELD_IN8_2 && nixie_multiplex_counter)
		display_multiplex_in14();
}

void button_timer(void);
uint16_t interrupt_counter = 0;
uint16_t button_counter = 0;

//#define BUTTON_TIMER_MAX 71
#define BUTTON_TIMER_MAX 71*2

// 1 click = 1us. Overflow every 255 us
//ISR(TIMER3_COMPA_vect)

#ifdef HAVE_ATMEGA328
ISR(TIMER2_OVF_vect)
#elif defined (HAVE_LEONARDO)
ISR(TIMER3_OVF_vect)
#endif
{  
    // control blinking: on time is slightly longer than off time
    if (blink && display_on && ++blink_counter >= 0x900*5) {
        display_on = false;
        blink_counter = 0;
    }
    else if (blink && !display_on && ++blink_counter >= 0x750*5) {
        display_on = true;
        blink_counter = 0;
    }
	
    // button polling
    if (++button_counter == BUTTON_TIMER_MAX) {
        button_timer();
     button_counter = 0;
    }
	
    // display multiplex
    if (++interrupt_counter == 10) {
        display_multiplex();
        interrupt_counter = 0;
    }

#ifdef HAVE_GPS
	if (++gps_counter == 4) {  // every 0.001024 seconds
		GPSread();  // check for data on the serial port
		gps_counter = 0;
	}
#endif // HAVE_GPS

#ifdef HAVE_ATMEGA328
    TCNT2 = 0x0;
#elif defined (HAVE_LEONARDO)
    TCNT3 = 0xff00;
#endif
}

// utility functions
// fixme: generalize this to print any length of number
uint8_t print_digits(uint8_t num, uint8_t offset)
{
    uint8_t ret = (num >= 100) ? offset+3 : offset+2;
    
    if (num >= 100) {
        data[offset+2] = num % 10;
        num /= 10;
    }
    
    data[offset+1] = num % 10;
    num /= 10;
    data[offset] = num % 10;
    
    return ret;
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

extern uint8_t g_volume;

// set dots based on mode and seconds
void print_dots(uint8_t mode, uint8_t seconds)
{
	if (g_show_dots) {
  		if (digits == 10 && mode == 0) {
			sbi(dots, 3);
			sbi(dots, 5);
		}
  		else if (digits == 8 && mode == 0) {
			sbi(dots, 3);
			sbi(dots, 5);
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
	uint8_t hour = _24h_clock ? t->hour : t->twelveHour;

	print_dots(mode, t->sec);

	if (mode == 0) { // normal display mode
            nixie_print(hour, t->min, t->sec);

		if (digits == 10) { // "  HH.MM.SS  "
			offset = print_ch(' ', offset); 

			if (!_24h_clock && !t->am)
				offset = print_ch('P', offset);
			else
				offset = print_ch(' ', offset); 

			offset = print_digits(hour, offset);
			offset = print_digits(t->min, offset);
			offset = print_digits(t->sec, offset);
			offset = print_ch(' ', offset);
			offset = print_ch(' ', offset);
		}
		else if (digits == 8) { // " HH.MM.SS "
			if (!_24h_clock && !t->am)
				offset = print_ch('P', offset);
			else
				offset = print_ch(' ', offset); 
			offset = print_digits(hour, offset);
			offset = print_digits(t->min, offset);
			offset = print_digits(t->sec, offset);
			offset = print_ch(' ', offset);
		}
		else if (digits == 6) { // "HH.MM.SS"
			offset = print_digits(hour, offset);
			offset = print_digits(t->min, offset);
			offset = print_digits(t->sec, offset);			
		}
		else { // HH.MM
			offset = print_digits(hour, offset);
			offset = print_digits(t->min, offset);
		}
	}
	else if (mode == 1) { // extra display mode
            nixie_print_compact(hour, t->min, t->sec);


		if (digits == 10) { // " HH-MM-SS "
			offset = print_ch('-', offset);
			offset = print_digits(hour, offset);
			offset = print_ch('-', offset);
			offset = print_digits(t->min, offset);
			offset = print_ch('-', offset);
			offset = print_digits(t->sec, offset);
			offset = print_ch('-', offset);

		}
		else if (digits == 8) { // "HH-MM-SS"
			offset = print_digits(hour, offset);
			offset = print_ch('-', offset);
			offset = print_digits(t->min, offset);
			offset = print_ch('-', offset);
			offset = print_digits(t->sec, offset);
		}
		else if (digits == 6) { // " HH-MM"
			offset = print_digits(hour, offset);
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
	nixie_print_compact(hour, min, sec);

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

	nixie_print(0, t, f);
	
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
		offset = print_digits(f, offset);		
	}

  if (digits == 10) dots = (1<<3);
  else if (digits == 8) dots = (1<<3);
  else if (digits == 6) dots = (1<<2);
  else if (digits == 4) dots = (1<<1);
}

void show_humidity(uint8_t hum)
{
	dots = 0;
	uint8_t offset = 0;
	
	nixie_print(0, 0, hum);

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
		offset = print_ch('H', offset);
	}
}

void show_pressure(uint8_t pressure)
{
	dots = 0;
	uint8_t offset = 0;
	
	uint8_t temp  = pressure % 10;
	uint8_t temp2 = pressure/= 10;

	nixie_print(0, temp2, temp);

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
	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';
	
	for (int i = 0; i <= digits-1; i++) {
		if (!*str) break;
		data[i] = *(str++);
	}
}

// shows setting string
void show_setting_string(const char* short_str, const char* long_str, const char* value, bool show_setting)
{
    Serial.print("show_setting_string(");
    Serial.print(short_str);
    Serial.print(", ");
    Serial.print(long_str);
    Serial.print(", ");
    Serial.print(value);
    Serial.print(", ");
    Serial.print(show_setting ? "true" : "false");
    Serial.println(")");
    
	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';

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

void show_setting_int(const char* short_str, const char* long_str, int value, bool show_setting)
{
    Serial.print("show_setting_int(");
    Serial.print(short_str);
    Serial.print(", ");
    Serial.print(long_str);
    Serial.print(", ");
    Serial.print(value);
    Serial.print(", ");
    Serial.print(show_setting ? "true" : "false");
    Serial.println(")");
    

	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';

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

void clear_display(void)
{
	write_vfd_8bit(0);
	write_vfd_8bit(0);
	write_vfd_8bit(0);
	write_vfd_8bit(0);

	LATCH_DISABLE;
	LATCH_ENABLE;
}

void set_char_at(char c, uint8_t offset)
{
	data[offset] = c;
}

