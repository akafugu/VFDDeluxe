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

#define A  0
#define B  1
#define C  2
#define D  3
#define E  4
#define F  5
#define G1 6
#define G2 7
#define H  8
#define I  9
#define J  10
#define K  11
#define L  12
#define M  13

#define  FEATURE_LOWERCASE

uint16_t calculate_segments_14(uint8_t character)
{
	uint16_t segments = 0;

	switch (character)
	{
		case 0:
		case '0':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<J)|(1<<M);
			break;
		case 1:
		case '1':
			//segments = (1<<I)|(1<<L);
			segments = (1<<B)|(1<<C)|(1<<J);
			break;
		case 2:
		case '2':
			segments = (1<<A)|(1<<B)|(1<<G1)|(1<<G2)|(1<<E)|(1<<D);
			break;
		case 3:
		case '3':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<G1)|(1<<G2);
			break;
		case 4:
		case '4':
			segments = (1<<B)|(1<<C)|(1<<G1)|(1<<G2)|(1<<F);
			break;
		case 5:
		case '5':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 6:
		case '6':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 7:
		case '7':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<F);
			break;
		case 8:
		case '8':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 9:
		case '9':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 10:
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<L)|(1<<I)|(1<<E)|(1<<F);
			break;
		case 11:
			segments = (1<<B)|(1<<C)|(1<<E)|(1<<F);
			break;
		case 12:
			segments = (1<<A)|(1<<B)|(1<<D)|(1<<L)|(1<<G2)|(1<<E)|(1<<F);
			break;
		case 13:
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<G2)|(1<<E)|(1<<F);
			break;
		case 14:
			segments = (1<<B)|(1<<C)|(1<<I)|(1<<G2)|(1<<E)|(1<<F);
			break;
		case 15:
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<I)|(1<<G2)|(1<<E)|(1<<F);
			break;
		case 16:
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<L)|(1<<I)|(1<<G2)|(1<<E)|(1<<F);
			break;
		case 17:
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<I)|(1<<E)|(1<<F);
			break;
		case 18:
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<L)|(1<<I)|(1<<G2)|(1<<E)|(1<<F);
			break;
		case 19:
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<I)|(1<<G2)|(1<<E)|(1<<F);	
			break;
		case 20:
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<L)|(1<<I)|(1<<H)|(1<<G1)|(1<<E);
			break;
		case 'A':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'B':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'C':
			segments = (1<<A)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 'D':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<I)|(1<<L);
			break;
		case 'E':
			segments = (1<<A)|(1<<D)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'F':
			segments = (1<<A)|(1<<E)|(1<<F)|(1<<G1);
			break;
		case 'G':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<G2);
			break;
		case 'H':
			segments = (1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'I':
			segments = (1<<A)|(1<<I)|(1<<L)|(1<<D);
			break;
		case 'J':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E);
			break;
		case 'K':
			segments = (1<<E)|(1<<F)|(1<<G1)|(1<<J)|(1<<K);
			break;
		case 'L':
			segments = (1<<D)|(1<<E)|(1<<F);
			break;
		case 'M':
			segments = (1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<H)|(1<<J);
			break;
		case 'N':
			segments = (1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<H)|(1<<K);
			break;
		case 'O':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 'P':
			segments = (1<<A)|(1<<B)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'Q':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F)|(1<<K);
			break;
		case 'R':
			segments = (1<<A)|(1<<B)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2)|(1<<K);
			break;
		case 'S':
			segments = (1<<A)|(1<<C)|(1<<D)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'T':
			segments = (1<<A)|(1<<I)|(1<<L);
			break;
		case 'U':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<F);
			break;
		case 'V':
			segments = (1<<E)|(1<<F)|(1<<J)|(1<<M);
			break;
		case 'W':
			segments = (1<<B)|(1<<C)|(1<<E)|(1<<F)|(1<<K)|(1<<M);
			break;
		case 'X':
		case 'x':
			segments = (1<<H)|(1<<J)|(1<<K)|(1<<M);
			break;
		case 'Y':
			segments = (1<<H)|(1<<J)|(1<<L);
			break;
		case 'Z':
			segments = (1<<A)|(1<<D)|(1<<J)|(1<<M);
			break;
#ifdef FEATURE_LOWERCASE
		case 'a':
			segments = (1<<D)|(1<<E)|(1<<G1)|(1<<L);
			break;
		case 'b':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<D)|(1<<I)|(1<<L)|(1<<G2);
			break;
		case 'c':
			segments = (1<<D)|(1<<E)|(1<<G1);
			break;
		case 'd':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E)|(1<<G1)|(1<<G2);
			break;
		case 'e':
			segments = (1<<D)|(1<<E)|(1<<G1)|(1<<M);
			break;
		case 'f':
			segments = (1<<A)|(1<<E)|(1<<F)|(1<<G1);
			break;
		case 'g':
			segments = (1<<C)|(1<<D)|(1<<K)|(1<<G2);
			break;
		case 'h':
			segments = (1<<C)|(1<<G2)|(1<<I)|(1<<L);
			break;
		case 'i':
			segments = (1<<L);
			break;
		case 'j':
			segments = (1<<B)|(1<<C)|(1<<D)|(1<<E);
			break;
		case 'k':
			segments = (1<<G2)|(1<<I)|(1<<K)|(1<<L);
			break;
		case 'l':
			segments = (1<<D)|(1<<E)|(1<<F);
			break;
		case 'm':
			segments = (1<<C)|(1<<E)|(1<<G1)|(1<<G2)|(1<<L);
			break;
		case 'n':
			segments = (1<<E)|(1<<G1)|(1<<L);
			break;
		case 'o':
			segments = (1<<C)|(1<<D)|(1<<E)|(1<<G1)|(1<<G2);
			break;
		case 'p':
			segments = (1<<A)|(1<<B)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'q':
			segments = (1<<A)|(1<<B)|(1<<C)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'r':
			segments = (1<<G2)|(1<<L);
			break;
		case 's':
			segments = (1<<D)|(1<<K)|(1<<G2);
			break;
		case 't':
			segments = (1<<D)|(1<<E)|(1<<F)|(1<<G1)|(1<<G2);
			break;
		case 'u':
			segments = (1<<C)|(1<<D)|(1<<E);
			break;
		case 'v':
			segments = (1<<E)|(1<<M);
			break;
		case 'w':
			segments = (1<<C)|(1<<E)|(1<<M)|(1<<K);
			break;
		case 'y':
			segments = (1<<C)|(1<<D)|(1<<K);
			break;
		case 'z':
			segments = (1<<D)|(1<<G1)|(1<<M);
			break;
#endif
		case '-':
			segments = (1<<G1)|(1<<G2);
			break;
		case '+':
			segments = (1<<G1)|(1<<G2)|(1<<I)|(1<<L);
			break;
	    case ' ':
		    segments = 0;
		    break;
		case '\'':
			segments = (1<<I);
			break;
		case '*':
		default:
			segments = (1<<H)|(1<<I)|(1<<J)|(1<<K)|(1<<L)|(1<<M)|(1<<G1)|(1<<G2);
			break;
	}
	
	return segments;
}
