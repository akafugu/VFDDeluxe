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
#include <avr/io.h>

uint16_t calculate_segments_16(uint8_t character);

#define segA1 0b0000000000000001
#define segA2 0b0000000000000010
#define segB  0b0000000000000100
#define segC  0b0000000000001000
#define segD1 0b0000000000010000
#define segD2 0b0000000000100000
#define segE  0b0000000001000000
#define segF  0b0000000010000000
#define segG1 0b0000000100000000
#define segG2 0b0000001000000000
#define segH  0b0000010000000000
#define segI  0b0000100000000000
#define segJ  0b0001000000000000
#define segK  0b0010000000000000
#define segL  0b0100000000000000
#define segM  0b1000000000000000

#ifdef HAVE_ALTERNATE_FONT
uint16_t segments_16_numerals[] = {
	segA1+segA2+segB+segC+segD1+segD2+segE+segF+segJ+segM,
	segB+segC+segJ,
	segA1+segA2+segB+segG2+segM+segD1+segD2,
	segA1+segA2+segC+segD1+segD2+segJ+segG2,
	segB+segC+segG2+segH,
	segA1+segA2+segC+segD1+segD2+segH+segG2,
	segA1+segA2+segK+segD1+segD2+segE+segF+segG1,
	segA1+segA2+segJ+segL,
	segA1+segA2+segB+segC+segD1+segD2+segE+segH+segG1+segG2,
	segA1+segA2+segB+segC+segD1+segD2+segH+segG2,
	segA2+segB+segC+segD1+segL+segI+segE+segF,
	segB+segC+segE+segF,
	segA2+segB+segD1+segL+segG2+segE+segF,
	segA2+segB+segC+segD1+segG2+segE+segF,
	segB+segC+segI+segG2+segE+segF,
	segA2+segC+segD1+segI+segG2+segE+segF,
	segA2+segC+segD1+segL+segI+segG2+segE+segF,
	segA2+segB+segC+segI+segE+segF,
	segA2+segB+segC+segD1+segL+segI+segG2+segE+segF,
	segA2+segB+segC+segD1+segI+segG2+segE+segF,
	segA2+segB+segC+segD1+segL+segI+segH+segG1+segE+segD2
};
#else
uint16_t segments_16_numerals[] = {
	segA1+segA2+segB+segC+segD1+segD2+segE+segF+segJ+segM,
	segB+segC+segJ,
	segA1+segA2+segB+segG1+segG2+segE+segD1+segD2,
	segA1+segA2+segB+segC+segD1+segD2+segG1+segG2,
	segB+segC+segG1+segG2+segF,
	segA1+segA2+segC+segD1+segD2+segF+segG1+segG2,
	segA1+segA2+segC+segD1+segD2+segE+segF+segG1+segG2,
	segA1+segA2+segB+segC+segF,
	segA1+segA2+segB+segC+segD1+segD2+segE+segF+segG1+segG2,
	segA1+segA2+segB+segC+segD1+segD2+segF+segG1+segG2,
	segA2+segB+segC+segD1+segL+segI+segE+segF,
	segB+segC+segE+segF,
	segA2+segB+segD1+segL+segG2+segE+segF,
	segA2+segB+segC+segD1+segG2+segE+segF,
	segB+segC+segI+segG2+segE+segF,
	segA2+segC+segD1+segI+segG2+segE+segF,
	segA2+segC+segD1+segL+segI+segG2+segE+segF,
	segA2+segB+segC+segI+segE+segF,
	segA2+segB+segC+segD1+segL+segI+segG2+segE+segF,
	segA2+segB+segC+segD1+segI+segG2+segE+segF,
	segA2+segB+segC+segD1+segL+segI+segH+segG1+segE+segD2
};
#endif // HAVE_ALTERNATE_FONT

uint16_t segments_16_alpha_upper[] = {
	segA1+segA2+segB+segC+segE+segF+segG1+segG2,
	segA1+segA2+segB+segC+segD1+segD2+segG2+segI+segL,
	segA1+segA2+segD1+segD2+segE+segF,
	segA1+segA2+segB+segC+segD1+segD2+segI+segL,
	segA1+segA2+segD1+segD2+segE+segF+segG1+segG2,
	segA1+segA2+segE+segF+segG1,
	segA1+segA2+segC+segD1+segD2+segE+segF+segG2,
	segB+segC+segE+segF+segG1+segG2,
	segA1+segA2+segI+segL+segD1+segD2,
	segB+segC+segD1+segD2+segE,
	segE+segF+segG1+segJ+segK,
	segD1+segD2+segE+segF,
	segB+segC+segE+segF+segH+segJ,
	segB+segC+segE+segF+segH+segK,
	segA1+segA2+segB+segC+segD1+segD2+segE+segF,
	segA1+segA2+segB+segE+segF+segG1+segG2,
	segA1+segA2+segB+segC+segD1+segD2+segE+segF+segK,
	segA1+segA2+segB+segE+segF+segG1+segG2+segK,
	segA1+segA2+segC+segD1+segD2+segF+segG1+segG2,
	segA1+segA2+segI+segL,
	segB+segC+segD1+segD2+segE+segF,
	segE+segF+segJ+segM,
	segB+segC+segE+segF+segK+segM,
	segH+segJ+segK+segM,
	segH+segJ+segL,
	segA1+segA2+segD1+segD2+segJ+segM
};

/*
#ifdef HAVE_LOWERCASE
		case 'a':
			segments = (1<<A2)|(1<<B)|(1<<C)|(1<<D2)|(1<<G2)|(1<<L);
			break;
		case 'b':
			segments = (1<<C)|(1<<D2)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'c':
			segments = (1<<D2)|(1<<G2)|(1<<L);
			break;
		case 'd':
			segments = (1<<B)|(1<<C)|(1<<D2)|(1<<G2)|(1<<L);
			break;
		case 'e':
			//segments = (1<<G2)|(1<<J)|(1<<K);
			segments = (1<<D1)|(1<<E)|(1<<G1)|(1<<M);
			break;
		case 'f':
			segments = (1<<A2)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'g':
			segments = (1<<A2)|(1<<B)|(1<<C)|(1<<D2)|(1<<G2)|(1<<I);
			break;
		case 'h':
			segments = (1<<C)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'i':
			segments = (1<<L);
			break;
		case 'j':
			segments = (1<<B)|(1<<C)|(1<<D2);
			break;
		case 'k':
			segments = (1<<G2)|(1<<I)|(1<<K)|(1<<L);
			break;
		case 'l':
			segments = (1<<D2)|(1<<I)|(1<<L);
			break;
		case 'm':
			segments = (1<<C)|(1<<E)|(1<<G1)|(1<<G2)|(1<<L);
			break;
		case 'n':
			segments = (1<<E)|(1<<G1)|(1<<L);
			break;
		case 'o':
			segments = (1<<C)|(1<<D2)|(1<<G2)|(1<<L);
			break;
		case 'p':
			segments = (1<<A2)|(1<<B)|(1<<G2)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'q':
			segments = (1<<A2)|(1<<B)|(1<<C)|(1<<G2)|(1<<I);
			break;
		case 'r':
			segments = (1<<G2)|(1<<L);
			break;
		case 's':
			segments = (1<<A2)|(1<<C)|(1<<D2)|(1<<G2)|(1<<I);
			break;
		case 't':
			segments = (1<<D2)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'u':
			segments = (1<<C)|(1<<D2)|(1<<L);
			break;
		case 'v':
			segments = (1<<E)|(1<<M);
			break;
		case 'w':
			segments = (1<<C)|(1<<D2)|(1<<E)|(1<<M)|(1<<L);
			break;
		case 'x':
			segments = (1<<H)|(1<<J)|(1<<K)|(1<<M);
			break;
		case 'y':
			segments = (1<<H)|(1<<I)|(1<<L);
			break;
		case 'z':
			segments = (1<<D1)|(1<<G1)|(1<<M);
			break;

		case '/':
			segments = (1<<J)|(1<<M);
			break;

*/

uint16_t calculate_segments_16(uint8_t ch)
{
	uint16_t segs = 0;
	if ((ch >= 'A') && (ch <= 'Z'))
		segs = segments_16_alpha_upper[ch-'A'];  // A-Z
	else if ((ch >= 'a') && (ch <= 'z'))
		segs = segments_16_alpha_upper[ch-'a'];  // a-z
	else if ((ch >= '0') && (ch <= '9'))
		segs = segments_16_numerals[ch-48];  // 0-9
	else if ((ch >= 0) && (ch <= 20))
		segs = segments_16_numerals[ch];  // 0-20
	else if (ch == ' ')
		segs = 0;
	else if (ch == '-')
		segs = segG1+segG2;
	else if (ch == '+')
		segs = segG1+segG2+segI+segL;
	else if (ch == '<')
		segs = segJ+segK;
	else if (ch == '>')
		segs = segH+segM;
	else if (ch == '/')
		segs = segJ+segM;
	else
		segs = segH+segI+segJ+segK+segL+segM+segG1+segG2;  // asterisk
	return segs;
}

