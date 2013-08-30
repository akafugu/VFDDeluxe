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
#include <WireRtcLib.h>
#include "adst.h"

uint8_t mDays[]={
  31,28,31,30,31,30,31,31,30,31,30,31};
uint16_t tmDays[]={
  0,31,59,90,120,151,181,212,243,273,304,334}; // Number days at beginning of month if not leap year

long DSTstart, DSTend;  // start and end of DST for this year, in Year Seconds

void breakTime(unsigned long time, WireRtcLib::tm* tm)
{
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!
  uint8_t year;
  uint8_t month, monthLength;
  unsigned long days;
  tm->sec = time % 60;
  time /= 60; // now it is minutes
  tm->min = time % 60;
  time /= 60; // now it is hours
  tm->hour = time % 24;
  time /= 24; // now it is days
  tm->wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm->year = year; // year is offset from 1970 
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = mDays[month];
    }
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm->mon = month + 1;  // jan is month 1  
  tm->mday = time + 1;     // day of month
}

unsigned long makeTime(WireRtcLib::tm* tm)
{   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  int i;
  unsigned long seconds;
  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds = tm->year*(SECS_PER_DAY * 365UL);
  for (i = 0; i < tm->year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  // add days for this year, months start from 1
  for (i = 1; i < tm->mon; i++) {
    if ( (i == 2) && LEAP_YEAR(tm->year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * mDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm->mday-1) * SECS_PER_DAY;
  seconds+= tm->hour * SECS_PER_HOUR;
  seconds+= tm->min * SECS_PER_MIN;
  seconds+= tm->sec;
  return seconds; 
}

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

void DSTinit(WireRtcLib::tm* te, int8_t rules[9])
{
  Serial.println("DSTinit");  // wbp debug
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
uint8_t getDSToffset(WireRtcLib::tm* te, int8_t rules[9])
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
    if ((seconds_now >= DSTend) || (seconds_now < DSTstart))  // spring ahead 14nov12/wbp
      return(rules[8]);  // return Offset
    else  // fall back
      return(0);  // return 0
  }
}

#endif // HAVE_AUTO_DST

