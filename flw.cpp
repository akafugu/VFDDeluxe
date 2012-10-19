/*
 * Four Letter Word Generator
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

/*
 * To use this Four Letter Word generator you will need the following:
 *
 * - A 512kbit/64kb I2C EEPROM
 * - A database file, generated from this Processing application:
 *   https://github.com/perjg/fourletterword
 * - A method for uploading the data file to the EEPROM
 *   (Either an Arduino Mega, or a normal Arduino with a micro SD card)
 *
 *
 * TODO:
 * - Allow changing random seed (currently the same sequence is always used)
 */

#include <Wire.h>

#include "flw.h"

#define EEPROM_ADDR 0b1010000
//unsigned long offset = 0;
//char current_word[6];

// random number seed
volatile uint32_t lfsr = 0xbeefcacc;

void seed_random(uint32_t seed)
{
	lfsr = seed;
}

uint32_t g_rand(void)
{
	// http://en.wikipedia.org/wiki/Linear_feedback_shift_register
	// Galois LFSR: taps: 32 31 29 1; characteristic polynomial: x^32 + x^31 + x^29 + x + 1 */
  	lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u);
	return lfsr;
}

uint8_t read_byte(int device, unsigned int addr) {
  uint8_t rdata = 0xFF;
  
  Wire.beginTransmission(device);
  Wire.write((int)(addr >> 8)); // MSB
  Wire.write((int)(addr & 0xFF)); // LSB
  Wire.endTransmission();
  
  Wire.requestFrom(device,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

void read_buffer(int device, unsigned int addr, uint8_t *buffer, int length) {
  Wire.beginTransmission(device);
  Wire.write((int)(addr >> 8)); // MSB
  Wire.write((int)(addr & 0xFF)); // LSB
  Wire.endTransmission();
  
  Wire.requestFrom(device,length);

  int c = 0;
  for ( c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}

// check if there is an eeprom installed with the fourletterword database
bool has_eeprom(void)
{
   uint8_t b1 = read_byte(EEPROM_ADDR, 0); 
   uint8_t b2 = read_byte(EEPROM_ADDR, 0); 
   
   if (b1 == 65 && b2 == 65)
     return true;
   return false;
}

// gets word at offset and picks a random one to go next
unsigned long get_word(unsigned long offset, char* word) {
	unsigned char low = 0xFF, high = 0xFF;
	unsigned int next_offset;
	unsigned char count = 0;
	int next = 0;

	read_buffer(EEPROM_ADDR, offset, (uint8_t*)word, 5);
	count = word[4];
	word[4] = '\0';

	next = g_rand() % count;
	//next = random(0, count-1);
	offset += 5 + next * 2;

	high = read_byte(EEPROM_ADDR, offset++);
	low  = read_byte(EEPROM_ADDR, offset++);
	
	next_offset = (high << 8) | low;
	
	return next_offset;
}
