/*
 *       Example of AC_Notify library .
 *       DIYDrones.com
 */

#include <AP_Common.h>
#include <AP_Progmem.h>
#include <AP_Math.h>            // ArduPilot Mega Vector/Matrix math Library
#include <AP_Param.h>
#include <AP_HAL.h>
#include <AP_HAL_AVR.h>
#include <AP_HAL_Linux.h>
#include <AP_HAL_PX4.h>
#include <AP_HAL_Empty.h>
#include <AP_HAL_V2R.h>
#include <AP_Notify.h>          // Notify library
#include <AP_BoardLED.h>        // Board LED library
#include <logger.h>
#include <GCS_MAVLink.h>
#include <DataFlash.h>
#include <AP_GPS.h>
#include <AP_InertialSensor.h>
#include <AP_ADC.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

// create board led object
AP_BoardLED board_led;

void setup()
{
    hal.console->println("AP_Notify library test");

    // initialise the board leds
    board_led.init();

    // turn on initialising notification
    AP_Notify::flags.initialising = true;
    AP_Notify::flags.gps_status = 1;
    AP_Notify::flags.armed = 1;
    AP_Notify::flags.pre_arm_check = 1;
}

void loop()
{
    hal.scheduler->delay(20);

    AP_Notify::flags.rc_input = 1;

    while (hal.console->available()) {
        char c;

        c = hal.console->read();

//         if (c < '0' || c > '9') {
//             continue;
//         }

//         log_dbg() << "[Key] " << c;
        
//     struct notify_type {
//         uint16_t initialising       : 1;    // 1 if initialising and copter should not be moved
//         uint16_t gps_status         : 2;    // 0 = no gps, 1 = no lock, 2 = 2d lock, 3 = 3d lock
//         uint16_t gps_glitching      : 1;    // 1 if gps position is not good
//         uint16_t armed              : 1;    // 0 = disarmed, 1 = armed
//         uint16_t pre_arm_check      : 1;    // 0 = failing checks, 1 = passed
//         uint16_t save_trim          : 1;    // 1 if gathering trim data
//         uint16_t esc_calibration    : 1;    // 1 if calibrating escs
//         uint16_t failsafe_radio     : 1;    // 1 if radio failsafe
//         uint16_t failsafe_battery   : 1;    // 1 if battery failsafe
//         uint16_t failsafe_gps       : 1;    // 1 if gps failsafe
// 
//         // additional flags
//         uint16_t external_leds      : 1;    // 1 if external LEDs are enabled (normally only used for copter)
//     };
#define HANDLE_KEY(KEY, FIELD)  \
    case KEY: \
        AP_Notify::flags.FIELD++; \
            log_inf() << #FIELD" " << AP_Notify::flags.FIELD; \
                break;
    
#define HANDLE_KEY_HELP(KEY, FIELD)  \
        log_inf() << KEY << " " << #FIELD;

        switch (c) {
            HANDLE_KEY('1', initialising);
            HANDLE_KEY('2', gps_status);
            HANDLE_KEY('3', gps_glitching);
            HANDLE_KEY('4', armed);
            HANDLE_KEY('5', pre_arm_check);
            HANDLE_KEY('6', save_trim);
            HANDLE_KEY('7', esc_calibration);
            HANDLE_KEY('8', failsafe_radio);
            HANDLE_KEY('9', failsafe_battery);
            HANDLE_KEY('0', failsafe_gps);

            case 'h':
            case 'H':
                HANDLE_KEY_HELP('1', initialising);
                HANDLE_KEY_HELP('2', gps_status);
                HANDLE_KEY_HELP('3', gps_glitching);
                HANDLE_KEY_HELP('4', armed);
                HANDLE_KEY_HELP('5', pre_arm_check);
                HANDLE_KEY_HELP('6', save_trim);
                HANDLE_KEY_HELP('7', esc_calibration);
                HANDLE_KEY_HELP('8', failsafe_radio);
                HANDLE_KEY_HELP('9', failsafe_battery);
                HANDLE_KEY_HELP('0', failsafe_gps);
                break;
        }
    }

    board_led.update();
}

AP_HAL_MAIN();
