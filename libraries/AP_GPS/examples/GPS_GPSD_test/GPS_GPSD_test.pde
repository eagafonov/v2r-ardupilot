// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
//
// Test for AP_GPS_NMEA
//

#include <AP_Common.h>
#include <AP_Param.h>
#include <AP_Progmem.h>
#include <AP_Math.h>

#include <AP_HAL.h>
#include <AP_GPS.h>
#include <AP_HAL_AVR.h>
#include <AP_HAL_AVR_SITL.h>
#include <AP_HAL_Empty.h>
#include <AP_HAL_V2R.h>
#include <AP_Notify.h>
#include <GCS_MAVLink.h>
#include <DataFlash.h>
#include <AP_InertialSensor.h>
#include <AP_ADC.h>
#include <logger.h>

#include <GPSDClient.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

AP_GPS_GPSD GPSD_gps;
GPS *gps = &GPSD_gps;

#define T6 1000000
#define T7 10000000

void setup()
{
    log_dbg() << "Setup";
    gps->init(hal.uartB);
    log_dbg() << "Setup done";

//     log_dbg() << "GPS test";
//     GPSDClient client;
//     client.thread_proc();
}

void loop()
{
    hal.scheduler->delay(100);
//     return;

    gps->update();
    if (gps->new_data) {
        if (gps->fix) {
            hal.console->printf_P(
                PSTR("Lat: %.7f Lon: %.7f Alt: %.2fm GSP: %.2fm/s "
                    "CoG: %d SAT: %d TIM: %u/%lu\r\n"),
                (float)gps->latitude / T7,
                (float)gps->longitude / T7,
                (float)gps->altitude_cm / 100.0,
                (float)gps->ground_speed_cm / 100.0,
                (int)gps->ground_course_cd / 100,
                gps->num_sats,
                gps->time_week,
                gps->time_week_ms);
        } else {
            hal.console->printf_P(PSTR("No fix: SAT: %d TIM: %u/%lu\r\n"),
                gps->num_sats,
                gps->time_week,
                gps->time_week_ms);
        }
        gps->new_data = false;
    }
}

AP_HAL_MAIN();
