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

#ifndef GPS_H_
#define GPS_H_

#include <Wire.h>
#include <WireRtcLib.h>

#define BUFFSIZ 90 // plenty big

class GPS
{
public:
  class gps_data {
    public:
      WireRtcLib::tm tm;
      int8_t timezone;

     uint32_t latitude, longitude;
     uint8_t groundspeed, trackangle;

      char latdir, longdir;
      char status;
  
      bool fix;
      uint8_t satellites;
  };

private:
  char buffer[BUFFSIZ];
  char *parseptr;
  char buffidx;
  
  GPS::gps_data gps;
  
  uint32_t parsedecimal(char *str);
  void readline(void);

public:
  void begin();
  GPS::gps_data* getData();
  
  void tick();
};


#endif // GPS_H_


