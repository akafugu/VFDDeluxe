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

#include "rotary.h"

#ifdef HAVE_ROTARY

#include <avr/interrupt.h>

/*
extern volatile bool g_update_rtc;
extern volatile bool g_update_rgb;
extern volatile bool g_blink_on;
*/
extern volatile uint16_t g_rotary_moved_timer;

// rotary encoder
#define ROTARY_1_DDR  DDRE
#define ROTARY_1_PORT PORTE
#define ROTARY_1 PORTE6 // INT6
#define ROTARY_1_BIT PINE6

#define ROTARY_2_DDR  DDRD
#define ROTARY_2_PORT PORTD
#define ROTARY_2 PORTD7 // no interrupt!!!
#define ROTARY_2_BIT PIND7

#define BUTTON_DDR  DDRD
#define BUTTON_PORT PORTD
#define BUTTON PORTD4
#define BUTTON_BIT PIND4

// Rotary encoder singleton
Rotary rotary;

uint8_t s_encoder_state;

// Raw position of rotary encoder (4 ticks per click)
volatile uint8_t s_rotary_raw_pos;
// Current position of rotary encoder
volatile uint8_t s_rotary_pos;

// fixme: set flag to see if rotary encoder is moving or not
// Rotary encoder interrupt
// Based on code in this library http://www.pjrc.com/teensy/td_libs_Encoder.html
//ISR( PCINT0_vect )
//ISR( INT6_vect )
static void isr6(void)
{
  uint8_t s = s_encoder_state & 3;
  if (PINB & _BV(ROTARY_1_BIT)) s |= 4;
  if (PINB & _BV(ROTARY_2_BIT)) s |= 8;

  switch (s) {
    case 0: case 5: case 10: case 15:
      break;
    case 1: case 7: case 8: case 14:
      s_rotary_raw_pos++; break;
    case 2: case 4: case 11: case 13:
      s_rotary_raw_pos--; break;
    case 3: case 12:
      s_rotary_raw_pos += 2; break;
    default:
      s_rotary_raw_pos -= 2; break;
    }

  s_rotary_pos = s_rotary_raw_pos;
  s_encoder_state = (s >> 2);
  
  // wrap position
  rotary.wrap();
    
  //g_update_rgb = true;
  //g_update_rtc = true;
  
  // set cooldown timer to disable blink for a small time frame after
  // the rotary encoder moves
  g_rotary_moved_timer = 100;
  //g_blink_on = false;
}

#include <Arduino.h>

void Rotary::begin()
{
  //MCUCR |= (1 << ISC61)|(1 << ISC60); // Turn on INT6
  //GIMSK |= (1 << INT6);
    //EICRB |= (1 << ISC61)|(1 << ISC60);

    attachInterrupt(6, isr6, CHANGE);

  
  // rotary encoder ports as input
  ROTARY_1_DDR &= ~(_BV(ROTARY_1));
  ROTARY_2_DDR &= ~(_BV(ROTARY_2));

  // rotary encoder button
  BUTTON_DDR &= ~(_BV(BUTTON)); // button as input

  // enable pullups for all rotary encoder pins
  ROTARY_1_PORT |= _BV(ROTARY_1); // enable pullup  
  ROTARY_2_PORT |= _BV(BUTTON) | _BV(ROTARY_2);
  
  // set up interrupt for rotary encoder pins
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT6);
  PCMSK0 |= (1 << PCINT7);

  // Initialize rotary encoder
  uint8_t s = 0;
  if (PINB & _BV(ROTARY_1_BIT)) s |= 1;
  if (PINB & _BV(ROTARY_2_BIT)) s |= 2;
  s_encoder_state = s;
}
  
void Rotary::setDivider(uint8_t divider)
{
  m_divider = divider;
}

void Rotary::setRange(uint8_t from, uint8_t to)
{
  m_min = from * m_divider;
  m_max = to   * m_divider + (m_divider-1);
}

void Rotary::setPosition(uint8_t value)
{
  s_rotary_raw_pos = s_rotary_pos = value * m_divider;
}

void Rotary::wrap()
{
  if (s_rotary_pos == 0xFF)
    s_rotary_pos = s_rotary_raw_pos = m_max;
  else if (s_rotary_pos > m_max)
    s_rotary_pos = s_rotary_raw_pos = m_min;
}

void Rotary::save()
{
  m_saved_pos = s_rotary_raw_pos;
  m_saved_min = m_min;
  m_saved_max = m_max;
}

void Rotary::restore()
{
  s_rotary_pos = s_rotary_raw_pos = m_saved_pos;
  m_min = m_saved_min;
  m_max = m_saved_max;  
}

uint8_t Rotary::getPosition()
{
  return s_rotary_pos / m_divider;
}

uint8_t Rotary::getRawPosition()
{
  return s_rotary_pos;
}

#endif // HAVE_ROTARY

