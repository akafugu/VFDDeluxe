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

#include <avr/io.h>

uint8_t calculate_segments_7(uint8_t character);

#define segA 0b00000001
#define segB 0b00000010
#define segC 0b00000100
#define segD 0b00001000
#define segE 0b00010000
#define segF 0b00100000
#define segG 0b01000000
#define segDP 0b10000000

char segments_7n[] = {
	segA + segB + segC + segD + segE + segF,  //0
	segB + segC, //1
	segA + segB + segD + segE + segG, //2
	segA + segB + segC + segD + segG, //3
	segB + segC + segF + segG, //4
	segA + segC + segD + segF + segG, //5 S
	segA + segC + segD + segE + segF + segG, //6
	segA + segB + segC, //7
	segA + segB + segC + segD + segE + segF + segG, //8
	segA + segB + segC + segD + segF + segG, //9
	};
char segments_7a[] = {
	segA + segB + segC + segE + segF + segG, //10 A
	segC + segD + segE + segF + segG, //11 B
	segA + segD + segE + segF, //12 C
	segB + segC + segD + segE + segG, //13 D
	segA + segD + segE + segF + segG, //14 E
	segA + segE + segF + segG, //15 F
	segA + segC + segD + segE + segF, //G
	segB + segC + segE + segF + segG, //H
	segB + segC, // I
	segB + segC + segD + segE, //J
	segB + segE + segF + segG, //K
	segD + segE + segF, //L
	segA + segC + segE + segG, //M
	segC + segE + segG, //N
	segA + segB + segC + segD + segE + segF,  //0
	segA + segB + segE + segF + segG, //P
	segA + segB + segC + segF + segG, //Q
	segE + segG, //R
	segA + segC + segD + segF + segG, //5 S
	segD + segE + segF + segG, //T
	segB + segC + segD + segE + segF, //U
	segC + segD + segE, //V
	segA + segD + segG, //W
	segA + segD + segG, //X
	segB + segC + segD + segF + segG, //Y
	segD + segG //Z
};

uint8_t calculate_segments_7(uint8_t ch)
{
	uint8_t segs = 0;
	if ((ch >= 0) && (ch <= 9))
		segs = segments_7n[ch];
	else if ((ch >= '0') && (ch <= '9'))
		segs = segments_7n[ch-48];
	else if ((ch >= 'A') && (ch <= 'Z'))  // A-Z
		segs = segments_7a[ch-'A'];
	else if ((ch >= 'a') && (ch <= 'z'))  // a-z
		segs = segments_7a[ch-'a'];
	else if (ch == '-')
		segs = segG;
	else if (ch == '"')
		segs = segB+segF;
	else if (ch == 0x27)  // "'"
		segs = segB;
	else if (ch == '_')
		segs = segD;
	else
		segs = 0;
	return segs;
}

