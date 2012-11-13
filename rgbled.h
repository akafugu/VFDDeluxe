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

#ifndef RGBLED_H_
#define RGBLED_H_

#include <stdbool.h>

#include <avr/io.h>

#define PCA9685_SLAVE_ADDR 0x40

void pca9685_wake(void);
void pca9685_PWM_precale(uint8_t prescale);
void pca9685_set_channel(uint8_t channel, uint16_t value);

void set_rgb_all(uint16_t r, uint16_t g, uint16_t b);
void set_rgb_ch(uint8_t channel, uint16_t r, uint16_t g, uint16_t b);

// sets backlight mode based preset
void set_rgb_mode(uint8_t mode);

void rgb_tick(void);

#endif // RGBLED_H_
