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

#define A	0
#define B	1
#define C	2
#define D	3
#define E	4
#define F	5
#define	G	6
#define	DP	7

uint8_t calculate_segments_7(uint8_t character)
{
	uint8_t segments = 0;
	
	switch (character)
	{
		case 0:
		case '0':
		case 'O':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 1:
		case '1':
		case 'l':
			segments = (1<<B)|(1<<C);
			break;
		case 2:
		case '2':
			segments = (1<<A)|(1<<B)|(1<<D)|(1<<E)|(1<<G);
			break;
		case 3:
		case '3':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<G);
			break;
		case 4:
		case '4':
			segments = (1<<B)|(1<<C)|(1<<F)|(1<<G);
			break;
		case 5:
		case '5':
		case 'S':
		case 's':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<F)|(1<<G);
			break;
		case 6:
		case '6':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 7:
		case '7':
			segments = (1<<A)|(1<<B)|(1<<C);
			break;
		case 8:
		case '8':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 9:
		case '9':
		case 'g':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<F)|(1<<G);
			break;
		case 10:
		case 'A':
		case 'a':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 11:
		case 'B':
		case 'b':
			segments = (1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 12:
		case 'C':
			segments = (1<<A)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 'c':
			segments = (1<<D)|(1<<E)|(1<<G);
			break;
		case 13:
		case 'D':
		case 'd':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<G);
			break;
		case 14:
		case 'E':
			segments = (1<<A)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 'e':
			segments = (1<<A)|(1<<B)|(1<<D)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 15:
		case 'F':
		case 'f':
			segments = (1<<A)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 'G':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 'H':
			segments = (1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 'h':
			segments = (1<<C)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 'I':
		case 'i':
			segments = (1<<B)|(1<<C);
			break;
		case 'J':
		case 'j':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E);
			break;
		case 'L':
			segments = (1<<D)|(1<<E)|(1<<F);
			break;
		case 'M':
		case 'm':
			segments = (1<<A)|(1<<C)|(1<<E)|(1<<G);
			break;
		case 'N':
		case 'n':
			segments = (1<<C)|(1<<E)|(1<<G);
			break;
		case 'o':
			segments = (1<<C)|(1<<D)|(1<<E)|(1<<G);
			break;
		case 'P':
		case 'p':
			segments = (1<<A)|(1<<B)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 'Q':
		case 'q':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<F)|(1<<G);
			break;
		case 'R':
		case 'r':
			segments = (1<<E)|(1<<G);
			break;
		case 'T':
		case 't':
			segments = (1<<D)|(1<<E)|(1<<F)|(1<<G);
			break;
		case 'U':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 'u':
			segments = (1<<C)|(1<<D)|(1<<E);
			break;
		case 'V':
		case 'v':
			segments = (1<<C)|(1<<D)|(1<<E);
			break;
		case 'W':
		case 'w':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<E);
			break;
		case 'Y':
		case 'y':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<F)|(1<<G);
			break;
		case '-':
			segments = (1<<G);
			break;
		case '"':
			segments = (1<<B)|(1<<F);
			break;
		case 0x27:	// "'"
			segments = (1<<B);
			break;
		case '_':
			segments = (1<<D);
			break;
		case ' ':
		default:
			segments = 0;
			break;
	}
	
	return segments;
}
