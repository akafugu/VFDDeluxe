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

#ifndef ROTARY_H_
#define ROTARY_H_

#include "features.h"

#ifdef HAVE_ROTARY

#include <stdbool.h>

#include <avr/io.h>

class Rotary {
public:
  Rotary() 
  : m_rotary_raw_pos(0)
  , m_min(0)
  , m_max(12*4)
  , m_rotary_pos(0)
  , m_divider(4)
  {}
  
  // fixme: set pins properly using arduino functions
  void begin();
  
  void setDivider(uint8_t divider);
  void setRange(uint8_t from, uint8_t to);
  void setPosition(uint8_t value);
  
  void wrap();

  void save();
  void restore();

  uint8_t getPosition();
  uint8_t getRawPosition();

private:
  //uint8_t m_encoder_state;
  
  // Raw position of rotary encoder (4 ticks per click)
  uint8_t m_rotary_raw_pos;
  // max and min value
  uint8_t m_min, m_max;
  // Current position of rotary encoder
  uint8_t m_rotary_pos;
  // Divider for rotary encoder position
  uint8_t m_divider;
  
  // saved values
  uint8_t m_saved_pos, m_saved_min, m_saved_max;
};

#endif // HAVE_ROTARY
#endif // ROTARY_H_

