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

#ifndef FLW_H__
#define FLW_H__

#include <inttypes.h>
#include <stdbool.h>

class FourLetterWord
{
private:
    unsigned long m_offset;
    char m_current_word[6];
    uint32_t m_lfsr;
    
    uint32_t randomize();
    
    uint8_t read_byte(int device, unsigned int addr);
    void read_buffer(int device, unsigned int addr, uint8_t *buffer, int length);
public:
    FourLetterWord() :
      m_offset(0),
      m_lfsr(0xbeefcace) {}

    void begin(uint32_t seed = 0xbeefcace);
    bool has_eeprom();
    char* get_word();
};


void seed_random(uint32_t seed);
bool has_eeprom(void);
unsigned long get_word(unsigned long offset, char* word);

#endif
