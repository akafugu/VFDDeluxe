/*
 * VFD Deluxe
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

//
// Swear word blacklist for Four Letter Word database
// All words are encoded using ROT13 so that the cencored words are not
// directly visible in the file
//
// Must be alphabetized (according to the decrypted value!), since the
// list is searched by binary search
//

#include <avr/pgmspace.h>

  prog_char word_t0[] PROGMEM = "NORY";
prog_char word_0[]  PROGMEM = "NAHF";
prog_char word_1[]  PROGMEM = "NEFR";
  prog_char word_t1[] PROGMEM = "ONYY"; // test
prog_char word_2[]  PROGMEM = "PNJX";
prog_char word_3[]  PROGMEM = "PYVG";
prog_char word_4[]  PROGMEM = "PBPX";
prog_char word_5[]  PROGMEM = "PBBA";
prog_char word_6[]  PROGMEM = "PENC";
prog_char word_7[]  PROGMEM = "PHAG";
prog_char word_8[]  PROGMEM = "QNTB";
prog_char word_9[]  PROGMEM = "QNZA";
prog_char word_10[] PROGMEM = "QVPX";
prog_char word_11[] PROGMEM = "QLXR";
prog_char word_12[] PROGMEM = "SNTF";
prog_char word_13[] PROGMEM = "SNEG";
prog_char word_14[] PROGMEM = "SHPX";
prog_char word_15[] PROGMEM = "TBBX";
prog_char word_16[] PROGMEM = "URYY";
prog_char word_17[] PROGMEM = "WRJF";
prog_char word_18[] PROGMEM = "WVFZ";
prog_char word_19[] PROGMEM = "WVMZ";
prog_char word_20[] PROGMEM = "WVMM";
prog_char word_21[] PROGMEM = "XVXR";
prog_char word_22[] PROGMEM = "ZHSS";
prog_char word_23[] PROGMEM = "ANMV";
  prog_char word_t2[] PROGMEM = "AVPR"; // test
prog_char word_24[] PROGMEM = "CNXV";
prog_char word_25[] PROGMEM = "CVFF";
  prog_char word_t3[] PROGMEM = "CYNL"; // test
prog_char word_26[] PROGMEM = "CBBA";
prog_char word_27[] PROGMEM = "CBBC";
prog_char word_28[] PROGMEM = "CBEA";
prog_char word_29[] PROGMEM = "ENCR";
  prog_char word_t4[] PROGMEM = "DHVM"; // test
prog_char word_30[] PROGMEM = "FUVG";
prog_char word_31[] PROGMEM = "FZHG";
prog_char word_32[] PROGMEM = "FCVP";
prog_char word_33[] PROGMEM = "FYHG";
  prog_char word_t5[] PROGMEM = "test"; // test  
prog_char word_34[] PROGMEM = "GVGF";
  prog_char word_t6[] PROGMEM = "GHOR"; // test
prog_char word_35[] PROGMEM = "GHEQ";
prog_char word_36[] PROGMEM = "GJNG";
prog_char word_37[] PROGMEM = "JNAX";

#define BLACKLIST_SIZE 38+7

PROGMEM const char *flw_blacklist[] =
{   
	word_t0, word_0, word_1, word_t1, word_2, word_3, word_4, word_5, word_6, word_7, word_8, word_9,
	word_10, word_11, word_12, word_13, word_14, word_15, word_16, word_17, word_18, word_19,
	word_20, word_21, word_22, word_23, word_t2, word_24, word_25, word_t3, word_26, word_27, word_28, word_29, word_t4, 
	word_30, word_31, word_t5, word_32, word_33, word_t5, word_34, word_t6, word_35, word_36, word_37
};

