/*
 * The Akafugu Nixie Clock
 * (C) 2012 Akafugu Corporation
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

#ifdef HAVE_RGB_BACKLIGHT

#include "rgbled.h"
#include <Wire.h>

#define PCA9685_ADDR 0x40
uint8_t _pca9685_address = PCA9685_ADDR;

#define highByte(w) ((uint8_t) ((w) >> 8))
#define lowByte(w) ((uint8_t) ((w) & 0xff))

unsigned int makeWord(unsigned char h, unsigned char l) { return (h << 8) | l; }
#define word(...) makeWord(__VA_ARGS__)

//Wake PCA9685 oscillator and enable auto increment
void pca9685_wake(void)
{
  Wire.beginTransmission(_pca9685_address);
  Wire.write((uint8_t)0x00);
  Wire.write(0B00100001);
  
  // totem-pole
  //Wire.write(0b00010100);
  // open-drain
  Wire.write(0b00010000);
  
  Wire.endTransmission();
}

// PCA9685 PWM frequency prescale
void pca9685_PWM_precale(uint8_t prescale)
{
  Wire.beginTransmission(_pca9685_address);
  Wire.write(0xfe);
  Wire.write(prescale);
  Wire.endTransmission();
}

// Set single channel PWM value (non-incremental)
void pca9685_set_channel(uint8_t channel, uint16_t value)
{
  channel = (channel * 4) + 8;
  Wire.beginTransmission(_pca9685_address);
  Wire.write(channel); 
  Wire.write(lowByte(value));
  Wire.endTransmission();
  Wire.beginTransmission(_pca9685_address);
  Wire.write(channel + 1);
  Wire.write(highByte(value));
  Wire.endTransmission();
}

void set_rgb_all(uint16_t r, uint16_t g, uint16_t b)
{
	set_rgb_ch(0, r, g, b);
	set_rgb_ch(1, r, g, b);
	set_rgb_ch(2, r, g, b);
	set_rgb_ch(3, r, g, b);
}

void set_rgb_ch(uint8_t channel, uint16_t r, uint16_t g, uint16_t b)
{
	pca9685_set_channel(   channel*3, b);
	pca9685_set_channel(1+ channel*3, g);
	pca9685_set_channel(2+ channel*3, r);
}

volatile uint8_t g_mode = 0;

void set_rgb_mode(uint8_t mode)
{
	if (mode == 0) {
		set_rgb_all(0, 0, 0); // OFF
	}
	else if (mode == 1) {
		set_rgb_all(2500, 2500, 2500); // WHITE
	}
	else if (mode == 2) {
		set_rgb_all(0, 0, 2500); // BLUE
	}
	else if (mode == 3) {
		set_rgb_all(0, 2500, 0); // GREEN
	}
	else if (mode == 4) {
		set_rgb_all(2500, 1000, 0); // NEON ORANGE
	}
	else if (mode == 5) {
		set_rgb_all(2500, 0, 0); // RED
	}
	else if (mode == 6) {
		set_rgb_all(2500, 0, 2500); // PURPLE
	}
	else if (mode == 7) {
		// pulse white
	}
	else if (mode == 8) {
		// pulse blue
	}
	else if (mode == 9) {
		// pulse blue
	}
	else if (mode == 10) {
		// pulse green
	}

	g_mode = mode;
}

extern volatile int8_t g_pulse_direction;
extern volatile uint16_t g_pulse_value;

/* fixme: already defined in flw.cpp - move to separate utility file
// random number seed
volatile uint32_t lfsr = 0xbeefcacc;

void seed_random(uint32_t seed)
{
	lfsr = seed;
}

uint32_t rnd(void)
{
	// http://en.wikipedia.org/wiki/Linear_feedback_shift_register
	// Galois LFSR: taps: 32 31 29 1; characteristic polynomial: x^32 + x^31 + x^29 + x + 1
  	lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u);
	return lfsr;
}
*/

void seed_random(uint32_t seed);
uint32_t rnd(void);

// pseudo-random pure-color fader
void solid_color_fade()
{
  static uint8_t fade_color = 0;
  static uint16_t red = 1000;
  static uint16_t green = 0;
  static uint16_t blue = 0;
  
  if (fade_color == 0) {
    seed_random(g_pulse_value);
  }
  

  set_rgb_all(red, green, blue);
  
  switch (fade_color) {
  case 0: // increase red
    if (red >= 3000) fade_color = rnd() % 5;
    else red+=4;
    break;
  case 1: // decrease red
    if (red <= 0) fade_color = rnd() % 5;
    else red-=4;
    break;
  case 2: // increase green
    if (green >= 3000) fade_color = rnd() % 5;
    else green+=4;
    break;
  case 3: // decrease green
    if (green <= 0) fade_color = rnd() % 5;
    else green-=4;
    break;
  case 4: // increase blue
    if (blue >= 3000) fade_color = rnd() % 5;
    else blue+=4;
    break;
  case 5: // decrease blue
    if (blue <= 0) fade_color = rnd() % 5;
    else blue-=4;
    break;
  }
}

#define OFFSET 1900
int16_t counter = 0;

void rgb_tick(void)
{
	g_pulse_value += (32*g_pulse_direction);
	if (g_mode < 7) return;

	if (g_mode == 7) { // pulse white
		set_rgb_all(g_pulse_value+ OFFSET, g_pulse_value+ OFFSET, g_pulse_value + OFFSET);
	}
	else if (g_mode == 8) { // pulse red
		set_rgb_all(g_pulse_value + OFFSET, 0, 0);
	}
	else if (g_mode == 9) { // pulse green
		set_rgb_all(0, g_pulse_value + OFFSET, 0);
	}
        else if (g_mode == 10) { // pulse blue
		set_rgb_all(0, 0, g_pulse_value + OFFSET);        
        }
	else if (g_mode == 11) { // pulse orange
		set_rgb_all(g_pulse_value + OFFSET + 500, 1200, 0);
	}
        else { // solid color fade
		solid_color_fade();
        }
}

#endif // HAVE_RGB_BACKLIGHT


