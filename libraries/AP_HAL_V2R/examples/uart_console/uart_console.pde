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
// #include <AP_InertialSensor.h>
// #include <AP_ADC.h>
// #include <GCS_MAVLink.h>
#include <AP_Param.h>
// #include <AP_Baro.h>
// #include <AP_Compass.h>
// #include <AP_Declination.h>
// #include <SITL.h>
#include <unistd.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;


static void delay_cb() {
    hal.console->println_P(PSTR("callback"));
};

void setup() {
//     hal.console->println_P(PSTR("Sleeping 5 sec"));
//     usleep(5 * 1000000);
    hal.console->println_P(PSTR("hello world"));
    hal.uartB->begin(9600);

//     hal.scheduler->register_delay_callback(delay_cb, 5);
}

void loop()
{
    hal.scheduler->delay(1000);
    hal.console->println("*");
    
    log_dbg() << hal.uartB->available();

    while (hal.uartB->available() > 0) {
        int16_t c = hal.uartB->read();
        hal.console->print((char)c);
    }
//     int16_t c = hal.uartB->read();
//     
//     if (c >	0) {
//         hal.console->print((char)c);
// //         log_dbg() << (char)c;
//     } else {
//         hal.scheduler->delay(10);
//     }
    
}

AP_HAL_MAIN();
