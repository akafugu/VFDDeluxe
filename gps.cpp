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

