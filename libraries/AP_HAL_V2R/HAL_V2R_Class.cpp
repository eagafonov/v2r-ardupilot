#include <AP_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_V2R

#include "HAL_V2R_Class.h"
#include "AP_HAL_V2R_Private.h"

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

using namespace V2R;

V2RUARTDriver uartConsole(true);

// 3 serial ports on V2R for now
static V2RUARTDriver uartADriver(false);
static V2RUARTDriver uartBDriver(false);
static V2RUARTDriver uartCDriver(false);

static V2RSemaphore  i2cSemaphore;
static V2RI2CDriver  i2cDriver(&i2cSemaphore, "/dev/i2c-1");
static V2RSPIDeviceManager spiDeviceManager;
static V2RAnalogIn analogIn;
static V2RStorage storageDriver;
static V2RGPIO gpioDriver;
/*static*/ V2RRCInput rcinDriver; // Non-static to be called by Scheduler in rc_thread
static V2RRCOutput rcoutDriver;
static V2RScheduler schedulerInstance;
static V2RUtil utilInstance;

HAL_V2R::HAL_V2R() :
    AP_HAL::HAL(
        &uartADriver,
        &uartBDriver,
        &uartCDriver,
        NULL,            /* no uartD */
        &i2cDriver,
        &spiDeviceManager,
        &analogIn,
        &storageDriver,
        &uartConsole,
        &gpioDriver,
        &rcinDriver,
        &rcoutDriver,
        &schedulerInstance,
        &utilInstance)
{}

void HAL_V2R::init(int argc,char* const argv[]) const
{
    int opt;
    /*
      parse command line options
     */
    while ((opt = getopt(argc, argv, "A:B:C:h")) != -1) {
        switch (opt) {
        case 'A':
            uartADriver.set_device_path(optarg);
            break;
// Port B is always UART1
#if 0
         case 'B':
            uartBDriver.set_device_path(optarg);
            break;
#endif
        case 'C':
            uartCDriver.set_device_path(optarg);
            break;
        case 'h':
            printf("Usage: -A uartAPath -B uartBPath -C uartCPath\n");
            exit(0);
        default:
            printf("Unknown option '%c'\n", (char)opt);
            exit(1);
        }
    }


    scheduler->init(NULL);
    uartA->begin(115200);
    uartB->begin(115200);
    uartC->begin(115200);
    uartConsole.begin(115200);
    uartConsole.set_blocking_writes(false); // non-blocking for console
    i2c->begin();
    spi->init(NULL);
    rcin->init(NULL);
    rcout->init(NULL);
}

const HAL_V2R AP_HAL_V2R;

#endif
