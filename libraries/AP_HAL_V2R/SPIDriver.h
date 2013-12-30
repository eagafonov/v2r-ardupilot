
#ifndef __AP_HAL_EMPTY_SPIDRIVER_H__
#define __AP_HAL_EMPTY_SPIDRIVER_H__

#include <AP_HAL_V2R.h>
#include "Semaphores.h"

class V2R::V2RSPIDeviceDriver : public AP_HAL::SPIDeviceDriver {
public:
    V2RSPIDeviceDriver(const char *spipath, uint8_t mode, uint8_t bitsPerWord, uint32_t speed);
    void init();
    AP_HAL::Semaphore* get_semaphore();
    void transaction(const uint8_t *tx, uint8_t *rx, uint16_t len);

    void cs_assert();
    void cs_release();
    uint8_t transfer (uint8_t data);
    void transfer (const uint8_t *data, uint16_t len);
private:
    V2RSemaphore _semaphore;
    const char *_spipath;
    int _fd;
    uint8_t _mode;
    uint8_t _bitsPerWord;
    uint32_t _speed;
};

class V2R::V2RSPIDeviceManager : public AP_HAL::SPIDeviceManager {
public:
    V2RSPIDeviceManager();
    void init(void *);
    AP_HAL::SPIDeviceDriver* device(enum AP_HAL::SPIDevice);
private:
    V2RSPIDeviceDriver _device_cs0;
    V2RSPIDeviceDriver _device_cs1;
};

#endif // __AP_HAL_V2R_SPIDRIVER_H__
