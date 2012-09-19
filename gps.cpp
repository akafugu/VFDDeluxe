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

/*

http://www.electronicsblog.net/arduino-gps-clock-using-nmea-protocol/

$GPGGA (fix information sentence)
http://aprs.gids.nl/nmea/#gga

$GPZDA (time info sentence)
http://aprs.gids.nl/nmea/#zda

UTC time offsets
http://en.wikipedia.org/wiki/List_of_time_zones_by_UTC_offset

*/

#include "gps.h"


uint32_t GPS::parsedecimal(char *str)
{
  uint32_t d = 0;
  
  while (str[0] != 0) {
   if ((str[0] > '9') || (str[0] < '0'))
     return d;
   d *= 10;
   d += str[0] - '0';
   str++;
  }
  return d;
}

void GPS::readline(void)
{
  char c;
  
  buffidx = 0; // start at begninning
  while (1) {
      c=Serial1.read();
      if (c == -1)
        continue;
      if (c == '\n')
        continue;
      if ((buffidx == BUFFSIZ-1) || (c == '\r')) {
        buffer[buffidx] = 0;
        return;
      }
      buffer[buffidx++]= c;
  }
}

#define GPSRATE 9600

void GPS::begin()
{
  Serial1.begin(GPSRATE);
}

GPS::gps_data* GPS::getData()
{
  return &gps;
}
  
void GPS::tick()
{
  uint32_t tmp;
  
  readline();
  
  // $GPRMC = global positioning fixed data
  if (strncmp(buffer, "$GPRMC",6) == 0) {
    Serial.println(buffer);

    // hhmmss time data
    parseptr = buffer+7;
    tmp = parsedecimal(parseptr);
    
    gps.tm.hour = tmp / 10000;
    gps.tm.min = (tmp / 100) % 100;
    gps.tm.sec = tmp % 100;
    
    parseptr = strchr(parseptr, ',') + 1;
    gps.status = parseptr[0];
    parseptr += 2;
    
    // grab latitude & long data
    // latitude
    gps.latitude = parsedecimal(parseptr);
    if (gps.latitude != 0) {
      gps.latitude *= 10000;
      parseptr = strchr(parseptr, '.')+1;
      gps.latitude += parsedecimal(parseptr);
    }
    parseptr = strchr(parseptr, ',') + 1;
    // read latitude N/S data
    if (parseptr[0] != ',') {
      gps.latdir = parseptr[0];
    }
    
    // longitude
    parseptr = strchr(parseptr, ',')+1;
    gps.longitude = parsedecimal(parseptr);
    if (gps.longitude != 0) {
      gps.longitude *= 10000;
      parseptr = strchr(parseptr, '.')+1;
      gps.longitude += parsedecimal(parseptr);
    }
    parseptr = strchr(parseptr, ',')+1;
    
    // read longitude E/W data
    if (parseptr[0] != ',') {
      gps.longdir = parseptr[0];
    }

    // groundspeed
    parseptr = strchr(parseptr, ',')+1;
    gps.groundspeed = parsedecimal(parseptr);

    // track angle
    parseptr = strchr(parseptr, ',')+1;
    gps.trackangle = parsedecimal(parseptr);

    // date
    parseptr = strchr(parseptr, ',')+1;
    tmp = parsedecimal(parseptr); 
    gps.tm.mday = tmp / 10000;
    gps.tm.mon = (tmp / 100) % 100;
    gps.tm.year = tmp % 100;
    
    Serial.print("Time: ");
    Serial.print(gps.tm.hour);
    Serial.print(":");
    Serial.print(gps.tm.min);
    Serial.print(":");
    Serial.println(gps.tm.sec);

    Serial.print("Date: ");
    Serial.print(gps.tm.year);
    Serial.print(".");
    Serial.print(gps.tm.mon);
    Serial.print(".");
    Serial.println(gps.tm.mday);

    //Serial.print("Status: ");
    //Serial.println(gps.status);

    /*
    Serial.print("Lat: "); 
    if (gps.latdir == 'N')
       Serial.print('+');
    else if (gps.latdir == 'S')
       Serial.print('-');

    Serial.print(gps.latitude/1000000, DEC); Serial.write('\°'); Serial.print(' ');
    Serial.print((gps.latitude/10000)%100, DEC); Serial.print('\''); Serial.print(' ');
    Serial.print((gps.latitude%10000)*6/1000, DEC); Serial.print('.');
    Serial.print(((gps.latitude%10000)*6/10)%100, DEC); Serial.println('"');
   
    Serial.print("Long: ");
    if (gps.longdir == 'E')
       Serial.print('+');
    else if (gps.longdir == 'W')
       Serial.print('-');
    Serial.print(gps.longitude/1000000, DEC); Serial.write('\°'); Serial.print(' ');
    Serial.print((gps.longitude/10000)%100, DEC); Serial.print('\''); Serial.print(' ');
    Serial.print((gps.longitude%10000)*6/1000, DEC); Serial.print('.');
    Serial.print(((gps.longitude%10000)*6/10)%100, DEC); Serial.println('"');
    */
  }
  // $GPGGA = Global Positioning System Fix Data
  else if (strncmp(buffer, "$GPGGA",6) == 0) {
    Serial.println(buffer);
    
    // skip past the initial data
    parseptr = strchr(buffer, ',') + 1;
    parseptr = strchr(parseptr, ',') + 1;
    parseptr = strchr(parseptr, ',') + 1;
    parseptr = strchr(parseptr, ',') + 1;
    parseptr = strchr(parseptr, ',') + 1;
    parseptr = strchr(parseptr, ',') + 1;

    // read fix data: 0 = no fix, 1 = GPS fix, 2 = DGPS fix
    if (*parseptr == 0)
      gps.fix = false;
    else
      gps.fix = true;  

    parseptr = strchr(parseptr, ',') + 1;
     
    char t1 = *parseptr++;
    char t2 = *parseptr;
    
    if (t2 == ',')
      gps.satellites = t1-'0';
    else
      gps.satellites = (t1-'0')*10+t2-'0';
    
    Serial.print("Fix data: ");
    Serial.print(gps.fix ? "FIX (" : "No fix (");
    Serial.print(gps.satellites);
    Serial.println(" satellites)");
  } 
}


