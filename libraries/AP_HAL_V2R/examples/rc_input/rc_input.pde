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

void check_mavlink_endianess() {
    union {
        uint16_t two_bytes;
        uint8_t bytes[2];
    };

    bytes[0] = 0;
    bytes[1] = 1;

    bool is_big_endian_platform = (two_bytes == 0x0001);
    bool is_big_endian_mavlink = (MAVLINK_ENDIAN != MAVLINK_LITTLE_ENDIAN);

    printf("Endianess check: 0x%04X -> platform:%d mavlink:%d mavlink_swap_bytes:%d\n", two_bytes, is_big_endian_platform, is_big_endian_mavlink, MAVLINK_NEED_BYTE_SWAP);
};

void setup() {
    check_mavlink_endianess();
    
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

    hal.console->printf("%5u\r", (int)rssi);

//     hal.console->println();
#endif
}

AP_HAL_MAIN();
