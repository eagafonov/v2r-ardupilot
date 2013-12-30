
#ifndef __AP_HAL_V2R_NAMESPACE_H__
#define __AP_HAL_V2R_NAMESPACE_H__

/* While not strictly required, names inside the V2R namespace are prefixed
 * with V2R for clarity. (Some of our users aren't familiar with all of the
 * C++ namespace rules.)
 */

namespace V2R {
    class V2RUARTDriver;
    class V2RI2CDriver;
    class V2RSPIDeviceManager;
    class V2RSPIDeviceDriver;
    class V2RAnalogSource;
    class V2RAnalogIn;
    class V2RStorage;
    class V2RGPIO;
    class V2RDigitalSource;
    class V2RRCInput;
    class V2RRCOutput;
    class V2RSemaphore;
    class V2RScheduler;
    class V2RUtil;
}

#endif // __AP_HAL_V2R_NAMESPACE_H__

