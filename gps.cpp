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

#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>
#include "gps.h"
#include "display.h"
#include "Time.h"

#include <WireRtcLib.h>

// String buffer for processing GPS data:
char gpsBuffer[GPSBUFFERSIZE];
//volatile uint8_t gpsEnabled = 0;
#define gpsTimeoutLimit 5  // 5 seconds until we display the "no gps" message
uint16_t gpsTimeout = 0;  // how long since we received valid GPS data?
time_t tGPSupdate = 0;

// fixme: integrate with menu
int8_t g_TZ_hour = 9;
uint8_t g_TZ_minutes;
uint8_t g_DST;  // DST off, on, auto?
uint8_t g_DST_offset;  // DST offset in hours

uint8_t g_gps_updated;

char gps_setting_[4];
char* gps_setting(uint8_t gps)
{
	switch (gps) {
		case(0):
			strcpy(gps_setting_,"off");
			break;
		case(48):
			strcpy(gps_setting_," 48");
			break;
		case(96):
			strcpy(gps_setting_," 96");
			break;
		default:
			strcpy(gps_setting_," ??");
	}
	return gps_setting_;
}

// Set DST offset and save in EE prom

//Check to see if there is any serial data.
uint8_t gpsDataReady(void) {
#ifdef HAVE_LEONARDO
  //return Serial1.available();
  return (UCSR1A & _BV(RXC1));
#else
  return (UCSR0A & _BV(RXC0));
#endif
}

// get data from gps and update the clock (if possible)
//$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68\r\n
// 0         1         2         3         4         5         6         7
// 01234567890123456789012345678901234567890123456789012345678901234567890
//    0     1   2    3    4     5    6   7     8      9     10  11 12
void getGPSdata(void) {
#ifdef HAVE_LEONARDO
	//char charReceived = Serial1.read();  // get a byte from the port
	char charReceived = UDR1;  // get a byte from the port
#else
	char charReceived = UDR0;  // get a byte from the port
#endif

    //Serial.print("Polling ");
    //Serial.println(charReceived);


	uint8_t bufflen = strlen(gpsBuffer);
	//If the buffer has not been started, check for '$'
	if ( ( bufflen == 0 ) &&  ( '$' != charReceived ) )
		return;  // wait for start of next sentence from GPS
	if ( bufflen < (GPSBUFFERSIZE - 1) ) {  // is there room left? (allow room for null term)
		if ( '\r' != charReceived ) {  // end of sentence?
			strncat(gpsBuffer, &charReceived, 1);  // add char to buffer
			return;
		}
		strncat(gpsBuffer, "*", 1);  // mark end of buffer just in case
		//beep(1000, 1);  // debugging
		// end of sentence - is this the message we are looking for?
		parseGPSdata();  // check for GPRMC sentence and set clock
	}  // if space left in buffer
	// either buffer is full, or the message has been processed. reset buffer for next message
	memset( gpsBuffer, 0, GPSBUFFERSIZE );
}  // getGPSdata

extern WireRtcLib rtc;

void setRTCTime(time_t t)
{
    tmElements_t tm;
    
    breakTime(t, &tm);
    
    rtc.setTime_s(tm.Hour, tm.Minute, tm.Second);   
}

void parseGPSdata() {
	char gpsCheck1, gpsCheck2;
	char gpsTime[7];
	char gpsDate[7];
	char gpsFixStat[1];  // fix status
	char gpsLat[6];  // ddmmff  (without decimal point)
	char gpsLatH[1];  // hemisphere 
	char gpsLong[7];  // ddddmmff  (without decimal point)
	char gpsLongH[1];  // hemisphere 
	char *gpsPtr;
	if ( strncmp( gpsBuffer, "$GPRMC,", 7 ) == 0 ) {
		//beep(1000, 1);
		//Calculate checksum from the received data
		gpsPtr = &gpsBuffer[1];  // start at the "G"
		gpsCheck1 = 0;  // init collector
		 /* Loop through entire string, XORing each character to the next */
		while (*gpsPtr != '*')  // count all the bytes up to the asterisk
		{
			gpsCheck1 ^= *gpsPtr;
			gpsPtr++;
		}
		// now get the checksum from the string itself, which is in hex
		uint8_t chk1, chk2;
		chk1 = *(gpsPtr+1);
		chk2 = *(gpsPtr+2);
		if (chk1 > '9') 
			chk1 = chk1 - 55;  // convert 'A-F' to 10-15
		else
			chk1 = chk1 - 48;  // convert '0-9' to 0-9
		if (chk2 > '9') 
			chk2 = chk2 - 55;  // convert 'A-F' to 10-15
		else
			chk2 = chk2 - 48;  // convert '0-9' to 0-9
		gpsCheck2 = (chk1 * 16)  + chk2;
		if (gpsCheck1 == gpsCheck2) {  // if checksums match, process the data

                        Serial.println(gpsBuffer);

			//beep(1000, 1);
			//Find the first comma:
			gpsPtr = strchr( gpsBuffer, ',');
			//Copy the section of memory in the buffer that contains the time.
			memcpy( gpsTime, gpsPtr + 1, 6 );
			gpsTime[6] = 0;  //add a null character to the end of the time string.
			gpsPtr = strchr( gpsPtr + 1, ',');  // find the next comma
			memcpy( gpsFixStat, gpsPtr + 1, 1 );  // copy fix status
			if (gpsFixStat[0] == 'A') {  // if data valid, parse time & date
				gpsTimeout = 0;  // reset gps timeout counter
				gpsPtr = strchr( gpsPtr + 1, ',');  // find the next comma
				memcpy( gpsLat, gpsPtr + 1, 4 );  // copy Latitude ddmm
				memcpy( gpsLat + 4, gpsPtr + 6, 2 );  // copy Latitude ff
				gpsPtr = strchr( gpsPtr + 1, ',');  // find the next comma
				memcpy( gpsLatH, gpsPtr + 1, 1 );  // copy Latitude Hemisphere
				gpsPtr = strchr( gpsPtr + 1, ',');  // find the next comma
				memcpy( gpsLong, gpsPtr + 1 , 5 );  // copy Longitude dddmm
				memcpy( gpsLong + 5, gpsPtr + 7 , 2 );  // copy Longitude ff
				gpsPtr = strchr( gpsPtr + 1, ',');  // find the next comma
				memcpy( gpsLongH, gpsPtr + 1 ,1 );  // copy Longitude Hemisphere
				//Find three more commas to get the date:
				for ( int i = 0; i < 3; i++ ) {
					gpsPtr = strchr( gpsPtr + 1, ',');
				}
				//Copy the section of memory in the buffer that contains the date.
				memcpy( gpsDate, gpsPtr + 1, 6 );
				gpsDate[6] = 0;  //add a null character to the end of the date string.
				time_t tNow;
				tmElements_t tm;
				tm.Hour = (gpsTime[0] - '0') * 10;
				tm.Hour = tm.Hour + (gpsTime[1] - '0');
				tm.Minute = (gpsTime[2] - '0') * 10;
				tm.Minute = tm.Minute + (gpsTime[3] - '0');
				tm.Second = (gpsTime[4] - '0') * 10;
				tm.Second = tm.Second + (gpsTime[5] - '0');
				tm.Day = (gpsDate[0] - '0') * 10;
				tm.Day = tm.Day + (gpsDate[1] - '0');
				tm.Month = (gpsDate[2] - '0') * 10;
				tm.Month = tm.Month + (gpsDate[3] - '0');
				tm.Year = (gpsDate[4] - '0') * 10;
				tm.Year = tm.Year + (gpsDate[5] - '0');
				tm.Year = y2kYearToTm(tm.Year);  // convert yy year to (yyyy-1970) (add 30)
				tNow = makeTime(&tm);  // convert to time_t

				if ((tm.Second == 0) || ((tNow - tGPSupdate)>=60)) {  // update RTC once/minute or if it's been 60 seconds
					set_gps_updated(true); // enable visual indication that the time is being updateds

					tGPSupdate = tNow;
					tNow = tNow + (g_TZ_hour + g_DST_offset) * SECS_PER_HOUR;  // add time zone hour offset & DST offset
					if (g_TZ_hour < 0)  // add or subtract time zone minute offset
						tNow = tNow - g_TZ_minutes * SECS_PER_HOUR;
					else
						tNow = tNow + g_TZ_minutes * SECS_PER_HOUR;

					setRTCTime(tNow);
					//fixme: rtc.setTime_t(tNow);  // set RTC from adjusted GPS time & date
					_delay_ms(100);  // pause long enough to make blink visible

					set_gps_updated(false);
				}

			} // if fix status is A
                        else {
                            Serial.println("No fix");
                        }
		} // if checksums match
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
    Serial.println("Init serial");
    
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
}

#endif // HAVE_GPS

