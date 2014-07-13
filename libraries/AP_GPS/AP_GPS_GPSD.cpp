// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// NMEA parser, adapted by Michael Smith from TinyGPS v9:
//
// TinyGPS - a small GPS library for Arduino providing basic NMEA parsing
// Copyright (C) 2008-9 Mikal Hart
// All rights reserved.
//

/// @file	AP_GPS_GPSD.cpp
/// @brief	NMEA protocol parser
///
/// This is a lightweight NMEA parser, derived originally from the
/// TinyGPS parser by Mikal Hart.
///

#include <AP_Common.h>

#include <AP_Progmem.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#include "AP_GPS_GPSD.h"

#include "gps.h"
#include <errno.h>
#include <logger.h>

extern const AP_HAL::HAL& hal;

AP_GPS_GPSD::AP_GPS_GPSD(void):
    GPS(),
    _gps_data_init(false)
{
}

// Public Methods //////////////////////////////////////////////////////////////
void AP_GPS_GPSD::init(AP_HAL::UARTDriver *s, enum GPS_Engine_Setting nav_setting)
{
// 	_port = s;
    _port = 0; // we do not use UART so make sure the driver does  not check it as well

    if (_gps_data_init) {
        log_wrn() << "[GPSD] Already initialized!";
        return;
    };

    int ret;

    ret = gps_open("127.0.0.1", "2947", &gps_data);

    log_dbg() << "[GPSD] open result " << ret;

    ret = gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);
    log_dbg() << "[GPSD] stream result " << ret;

    _gps_data_init = true;
}

bool AP_GPS_GPSD::read(void)
{
    int16_t numc;
    bool parsed = false;

    if (gps_waiting (&gps_data, 0)) {
        errno = 0;
        if (gps_read (&gps_data) == -1) {
            log_err() << "[GPSD] read error";
        } else {
            /* Display data from the GPS receiver. */
            // if (gps_data.set & ..

            if (gps_data.set & SATELLITE_SET) {
                num_sats = gps_data.satellites_visible;
            }

            if (gps_data.status == STATUS_NO_FIX) {
                fix = FIX_NONE;
                return true; // Nothing to do anymore
            }

            switch (gps_data.fix.mode) {
                case MODE_NOT_SEEN:
                case MODE_NO_FIX:
                    fix = FIX_NONE;
                    break;

                case MODE_2D:
                    fix = FIX_2D;
                    break;

                case MODE_3D:
                    fix = FIX_3D;
                    break;
            };

            longitude = gps_data.fix.longitude * 10000000;
            latitude = gps_data.fix.latitude * 10000000;
            altitude_cm = gps_data.fix.altitude * 100;
            ground_speed_cm = gps_data.fix.speed * 100;
            ground_course_cd = gps_data.fix.track * 100;

            long time_sec = (long)gps_data.fix.time;

            double integral, fractional;
            fractional = modf(gps_data.fix.time, &integral);
            time_t intfixtime = (time_t) integral;
            uint32_t intfixtime_ms = (uint32_t)(fractional * 1000);

            // convert to time since GPS epoch
            intfixtime -= 315964800;

            // get GPS week and time
            time_week = intfixtime / (7 * 86400); // Weeks

            time_week_ms = (uint32_t)(intfixtime % (7 * 86400)) * 1000 + intfixtime_ms;

//             printf("status: %d satelites:%d/%d fix_mode:%d long:%lf(+-%lf) lat:%lf(+-%lf) alt:%lf(+-%lf) time:%lf\n",
//                     gps_data.status,
//                     gps_data.satellites_used,
//                     gps_data.satellites_visible,
//                     gps_data.fix.mode,
//                     gps_data.fix.longitude, gps_data.fix.epx,
//                     gps_data.fix.latitude, gps_data.fix.epy,
//                     gps_data.fix.altitude, gps_data.fix.epv,
//                     gps_data.fix.time
//                     );

            parsed = true;
            
//             new_data = true;
        }
    }/* else {
        log_dbg() << "[GPSD] No data yet";
    }*/

    return parsed;
}


