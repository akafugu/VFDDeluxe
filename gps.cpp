/*
 * GPS support for VFD Modular Clock
 * (C) 2012 William B Phelps
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

#include "features.h"

#ifdef HAVE_GPS

#include "global.h"
#include "global_vars.h"

#include <avr/interrupt.h>
#include <string.h>
//#include <util/delay.h>
#include "gps.h"
#include "display.h"
#include "time.h"  // use ours, not Arduino's

#include <WireRtcLib.h>

unsigned long tGPSupdate;

// we double buffer: read into one line and leave one for the main program
volatile char gpsBuffer1[GPSBUFFERSIZE];
volatile char gpsBuffer2[GPSBUFFERSIZE];
// our index into filling the current line
volatile uint8_t gpsBufferPtr;
// pointers to the double buffers
volatile char *gpsNextBuffer;
volatile char *gpsLastBuffer;
volatile uint8_t gpsDataReady_;

extern int8_t g_gps_enabled;

// globals from main.c
extern enum shield_t shield;

//volatile uint8_t gpsEnabled = 0;
#define gpsTimeoutLimit 5  // 5 seconds until we display the "no gps" message
uint16_t gpsTimeout;  // how long since we received valid GPS data?
time_t tLast = 0;  // for checking GPS messages

extern WireRtcLib rtc;

void setRTCTime(time_t t)
{
    tmElements_t tm;
    breakTime(t, &tm);
    
    WireRtcLib::tm temp;

    temp.hour = tm.Hour;
    temp.min  = tm.Minute;
    temp.sec  = tm.Second;
    temp.mday = tm.Day;
    temp.mon  = tm.Month;
    temp.year = (1970+tm.Year)-2000;
    temp.wday = tm.Wday;
    
    rtc.setTime(&temp);
}

void GPSread(void) 
{
  char c = 0;
  
#ifdef HAVE_LEONARDO
  if ((g_gps_enabled) && (UCSR1A & _BV(RXC1))) {
		c=UDR1;  // get a byte from the port
#else
  if ((g_gps_enabled) && (UCSR0A & _BV(RXC0))) {
		c=UDR0;  // get a byte from the port
#endif
		if (c == '$') {
			gpsNextBuffer[gpsBufferPtr] = 0;
			gpsBufferPtr = 0;
		}
		if (c == '\n') {  // newline marks end of sentence
			gpsNextBuffer[gpsBufferPtr] = 0;  // terminate string
			if (gpsNextBuffer == gpsBuffer1) {  // switch buffers
				gpsNextBuffer = gpsBuffer2;
				gpsLastBuffer = gpsBuffer1;
			} else {
				gpsNextBuffer = gpsBuffer1;
				gpsLastBuffer = gpsBuffer2;
			}
			gpsBufferPtr = 0;
			gpsDataReady_ = true;  // signal data ready
		}
		gpsNextBuffer[gpsBufferPtr++] = c;  // add char to current buffer, then increment index
		if (gpsBufferPtr >= GPSBUFFERSIZE)  // if buffer full
			gpsBufferPtr = GPSBUFFERSIZE-1;  // decrement index to make room (overrun)
	}
//	return c;

}

uint8_t gpsDataReady(void) {
	return gpsDataReady_;
}

char *gpsNMEA(void) {
  gpsDataReady_ = false;
  return (char *)gpsLastBuffer;
}

uint32_t parsedecimal(char *str, uint8_t len) {
  uint32_t d = 0;
	for (uint8_t i=0; i<len; i++) {
   if ((str[i] > '9') || (str[0] < '0'))
     return d;  // no more digits
	 d = (d*10) + (str[i] - '0');
  }
  return d;
}

const char hex[17] = "0123456789ABCDEF";
uint8_t atoh(char x) {
  return (strchr(hex, x) - hex);
}
uint32_t hex2i(char *str, uint8_t len) {
  uint32_t d = 0;
	for (uint8_t i=0; i<len; i++) {
	 d = (d*10) + (strchr(hex, str[i]) - hex);
	}
	return d;
}

// find next token in GPS string - find next comma, then point to following char
char * ntok ( char *ptr ) {
	ptr = strchr(ptr, ',');  // Find the next comma
	if (ptr == NULL) return NULL;
	ptr++;  // point at next char after comma
	return ptr;
}

//  225446       Time of fix 22:54:46 UTC
//  A            Navigation receiver warning A = OK, V = warning
//  4916.45,N    Latitude 49 deg. 16.45 min North
//  12311.12,W   Longitude 123 deg. 11.12 min West
//  000.5        Speed over ground, Knots
//  054.7        Course Made Good, True
//  191194       Date of fix  19 November 1994
//  020.3,E      Magnetic variation 20.3 deg East
//  *68          mandatory checksum

//$GPRMC,225446.000,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68\r\n
// 0         1         2         3         4         5         6         7
// 0123456789012345678901234567890123456789012345678901234567890123456789012
//    0     1       2    3    4     5    6   7     8      9     10  11 12
void parseGPSdata(char *gpsBuffer) {
	time_t tNow, tDelta;
	tmElements_t tm;
	uint8_t gpsCheck1, gpsCheck2;  // checksums
//	char gpsTime[10];  // time including fraction hhmmss.fff
	char gpsFixStat;  // fix status
//	char gpsLat[7];  // ddmm.ff  (with decimal point)
//	char gpsLatH;  // hemisphere 
//	char gpsLong[8];  // dddmm.ff  (with decimal point)
//	char gpsLongH;  // hemisphere 
//	char gpsSpeed[5];  // speed over ground
//	char gpsCourse[5];  // Course
//	char gpsDate[6];  // Date
//	char gpsMagV[5];  // Magnetic variation 
//	char gpsMagD;  // Mag var E/W
//	char gpsCKS[2];  // Checksum without asterisk
	char *ptr;
	uint32_t tmp;
	if ( strncmp( gpsBuffer, "$GPRMC,", 7 ) == 0 ) {
		//beep(1000, 1);
		//Calculate checksum from the received data
		ptr = &gpsBuffer[1];  // start at the "G"
		gpsCheck1 = 0;  // init collector
		 /* Loop through entire string, XORing each character to the next */
		while (*ptr != '*') // count all the bytes up to the asterisk
		{
			gpsCheck1 ^= *ptr;
			ptr++;
			if (ptr>(gpsBuffer+GPSBUFFERSIZE)) goto GPSerror1;  // extra sanity check, can't hurt...
		}
		// now get the checksum from the string itself, which is in hex
    gpsCheck2 = atoh(*(ptr+1)) * 16 + atoh(*(ptr+2));
		if (gpsCheck1 == gpsCheck2) {  // if checksums match, process the data
			//beep(1000, 1);
			ptr = &gpsBuffer[1];  // start at beginning of buffer
			ptr = ntok(ptr);  // Find the time string
			if (ptr == NULL) goto GPSerror1;
			char *p2 = strchr(ptr, ',');  // find comma after Time
			if (p2 == NULL) goto GPSerror1;
			if (p2 < (ptr+6)) goto GPSerror1;  // Time must be at least 6 chars
//			strncpy(gpsTime, ptr, 10);  // copy time string hhmmss
			tmp = parsedecimal(ptr, 6);   // parse integer portion
			tm.Hour = tmp / 10000;
			tm.Minute = (tmp / 100) % 100;
			tm.Second = tmp % 100;
			ptr = ntok(ptr);  // Find the next token - Status
			if (ptr == NULL) goto GPSerror1;
			gpsFixStat = ptr[0];
			if (gpsFixStat == 'A') {  // if data valid, parse time & date
				gpsTimeout = 0;  // reset gps timeout counter
				for (uint8_t n=0; n<7; n++) { // skip 6 tokend, find date
					ptr = ntok(ptr);  // Find the next token
					if (ptr == NULL) goto GPSerror1; // error if not found
				}
				p2 = strchr(ptr, ',');  // find comma after Date
				if (p2 == NULL) goto GPSerror1;
				if (p2 != (ptr+6)) goto GPSerror1;  // check date length
				tmp = parsedecimal(ptr, 6); 
				tm.Day = tmp / 10000;
				tm.Month = (tmp / 100) % 100;
				tm.Year = tmp % 100;
				ptr = strchr(ptr, '*');  // Find Checksum
				if (ptr == NULL) goto GPSerror1;
//				strncpy(gpsCKS, ptr, 2);  // save checksum chars
				
				tm.Year = y2kYearToTm(tm.Year);  // convert yy year to (yyyy-1970) (add 30)
				tNow = makeTime(&tm);  // convert to time_t
				
//				if ((tGPSupdate>0) && (abs(tNow-tGPSupdate)>SECS_PER_DAY))  goto GPSerror2;  // GPS time jumped more than 1 day
				if ( (tLast>0) && (abs(tNow - tLast)>30) )  // if time jumps by more than a few seconds, 
				{
					tLast = tNow;  // save new time
					goto GPSerror2;  // it's probably an error
				}
				else {
					tLast = tNow;
					tDelta = tNow - tGPSupdate;
//					if ((tm.Second == 0) || ((tNow - tGPSupdate)>=60)) {  // update RTC once/minute or if it's been 60 seconds
					if (((tm.Second<5) && (tDelta>10)) || (tDelta>=60)) {  // update RTC once/minute or if it's been 30 seconds
						//beep(1000, 1);  // debugging
						g_gps_updating = true;
						tGPSupdate = tNow;  // remember time of this update
						tNow = tNow + (long)(g_TZ_hour + g_DST_offset) * SECS_PER_HOUR;  // add time zone hour offset & DST offset
						if (g_TZ_hour < 0)  // add or subtract time zone minute offset
							tNow = tNow - (long)g_TZ_minute * SECS_PER_HOUR;
						else
							tNow = tNow + (long)g_TZ_minute * SECS_PER_HOUR;
						setRTCTime(tNow);  // set RTC from adjusted GPS time & date
						if ((shield != SHIELD_IV18) && (shield != SHIELD_IV17))
							flash_display(100);  // flash display to show GPS update 28oct12/wbp - shorter blink
					}
					else
						g_gps_updating = false;
				}
			} // if fix status is A
		} // if checksums match
		else  // checksums do not match
			g_gps_cks_errors++;  // increment error count
		return;
GPSerror1:
		g_gps_parse_errors++;  // increment error count
//	  tone(PinMap::piezo, 2093, 100);  // test tone
		goto GPSerror2a;
GPSerror2:
		g_gps_time_errors++;  // increment error count
GPSerror2a:
		//beep(2093,1);  // error signal - I'm leaving this in for now /wm
		flash_display(200);  // flash display to show GPS error
	  tone(PinMap::piezo, 2093, 100);  // test tone
		strcpy(gpsBuffer, "");  // wipe GPS buffer
	}  // if "$GPRMC"
}

void uart_init(uint16_t BRR) {
#ifdef HAVE_LEONARDO
  UBRR1 = BRR;               // set baudrate counter

  UCSR1B = _BV(RXEN1) | _BV(TXEN1);
  UCSR1C = _BV(USBS1) | (3<<UCSZ12);
  DDRD |= _BV(PORTD2);
  DDRD &= ~_BV(PORTD);
//    Serial1.begin(9600); // fixme: adapt uart rate for serial library
#else
  /* setup the main UART */
  UBRR0 = BRR;               // set baudrate counter

  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(USBS0) | (3<<UCSZ00);
  DDRD |= _BV(PORTD1);
  DDRD &= ~_BV(PORTD0);
#endif
}

void gps_init(uint8_t gps) {
	switch (gps) {
		case(0):  // no GPS
			break;
		case(48):
			uart_init(BRRL_4800);
			break;
		case(96):
			uart_init(BRRL_9600);
			break;
	}
	tGPSupdate = 0;  // reset GPS last update time
	gpsDataReady_ = false;
  gpsBufferPtr = 0;
  gpsNextBuffer = gpsBuffer1;
  gpsLastBuffer = gpsBuffer2;
}

#endif // HAVE_GPS

