/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
  simple hello world sketch
  Andrew Tridgell September 2011
*/

#include <AP_Common.h>
#include <AP_Progmem.h>
#include <AP_HAL.h>
#include <AP_HAL_V2R.h>
#include <AP_Math.h>
#include <logger.h>
// #include <Filter.h>
#include <AP_InertialSensor.h>
#include <AP_ADC.h>
#include <AP_Notify.h>
#include <GCS_MAVLink.h>
#include <AP_Param.h>
#include <DataFlash.h>
#include <AP_GPS.h>
#include <RSSI_V2R.h>
// #include <AP_Baro.h>
// #include <AP_Compass.h>
// #include <AP_Declination.h>
// #include <SITL.h>
#include <unistd.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

void setup() {
    hal.console->printf("Process RC Input\n");
}

void loop()
{
    hal.scheduler->delay(20);
    uint16_t rc[8];

    int channels = hal.rcin->read(rc, 8);

    uint8_t rssi = read_rssi_v2r();

#if 1
    for (int i = 0; i < channels; i++) {
        hal.console->printf("%5u", (rc[i]));
    }

    hal.console->printf("%5u", (int)rssi);

    hal.console->println();
#endif
}

AP_HAL_MAIN();
