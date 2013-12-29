/*
 * Auto DST support for VFD Modular Clock
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

#include "global.h"
#include "global_vars.h"

#ifdef HAVE_AUTO_DST

#include <avr/io.h>
#include <string.h>
//#include "Time.h"
#include <Wire.h>
#include <TWIDisplay.h>
#include <WireRtcLib.h>
#include "adst.h"

const uint8_t mDays[]={
  31,28,31,30,31,30,31,31,30,31,30,31};
const uint16_t tmDays[]={
  0,31,59,90,120,151,181,212,243,273,304,334}; // Number days at beginning of month if not leap year

long DSTstart, DSTend;  // start and end of DST for this year, in Year Seconds

// Calculate day of the week - Sunday=1, Saturday=7  (non ISO)
uint8_t dotw(uint16_t year, uint8_t month, uint8_t day)
{
  uint16_t m, y;
  m = month;
  y = year;
  if (m < 3)  {
    m += 12;
    y -= 1;
  }
  return (day + (2 * m) + (6 * (m+1)/10) + y + (y/4) - (y/100) + (y/400) + 1) % 7 + 1;
}

long yearSeconds(uint16_t yr, uint8_t mo, uint8_t da, uint8_t h, uint8_t m, uint8_t s)
{
  unsigned long dn = tmDays[(mo-1)]+da;  // # days so far if not leap year or (mo<3)
  if (mo>2) {
    if ((yr%4 == 0 && yr%100 != 0) || yr%400 == 0)  // if leap year
      dn ++;  // add 1 day
  }
  dn = dn*86400 + (long)h*3600 + (long)m*60 + s;
  return dn;
} 

long DSTseconds(uint16_t year, uint8_t month, uint8_t doftw, uint8_t week, uint8_t hour)
{
  uint8_t dom = mDays[month-1];
  if ( (month == 2) && (year%4 == 0) )
    dom ++;  // february has 29 days this year
  uint8_t dow = dotw(year, month, 1);  // DOW for 1st day of month for DST event
  int8_t day = doftw - dow;  // number of days until 1st dotw in given month
  if (day<1)  day += 7;  // make sure it's positive 
  if (doftw >= dow)
    day = doftw - dow;
  else
    day = doftw + 7 - dow;
  day = 1 + day + (week-1)*7;  // date of dotw for this year
  while (day > dom)  // handles "last DOW" case
    day -= 7;
  return yearSeconds(year,month,day,hour,0,0);  // seconds til DST event this year
}

void DSTinit(WireRtcLib::tm* te, uint8_t rules[9])
{
  uint16_t yr = 2000 + te->year;  // Year as 20yy; te.Year is not 1970 based
  // seconds til start of DST this year
  DSTstart = DSTseconds(yr, rules[0], rules[1], rules[2], rules[3]);  
	// seconds til end of DST this year
  DSTend = DSTseconds(yr, rules[4], rules[5], rules[6], rules[7]);  
}

// DST Rules: Start(month, dotw, n, hour), End(month, dotw, n, hour), Offset
// DOTW is Day of the Week.  1=Sunday, 7=Saturday
// N is which occurrence of DOTW
// Current US Rules: March, Sunday, 2nd, 2am, November, Sunday, 1st, 2 am, 1 hour
// 		3,1,2,2,  11,1,1,2,  1
uint8_t getDSToffset(WireRtcLib::tm* te, uint8_t rules[9])
{
  uint16_t yr = 2000 + te->year;  // Year as 20yy; te.Year is not 1970 based
  // if current time & date is at or past the first DST rule and before the second, return 1
  // otherwise return 0
  long seconds_now = yearSeconds(yr, te->mon, te->mday, te->hour, te->min, te->sec);
  if (DSTstart<DSTend) {  // northern hemisphere
    if ((seconds_now >= DSTstart) && (seconds_now < DSTend))  // spring ahead
      return(rules[8]);  // return Offset
    else  // fall back
      return(0);  // return 0
  }
  else {  // southern hemisphere
		if ((seconds_now >= DSTstart) || (seconds_now < DSTend))  // fall ahead
      return(rules[8]);  // return Offset
    else  // fall back
      return(0);  // return 0
  }
}

#endif // HAVE_AUTO_DST
