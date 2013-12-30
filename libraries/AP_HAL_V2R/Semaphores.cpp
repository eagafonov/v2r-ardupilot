#include <AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_V2R

#include "Semaphores.h"

extern const AP_HAL::HAL& hal;

using namespace V2R;

bool V2RSemaphore::give() 
{
    return pthread_mutex_unlock(&_lock) == 0;
}

bool V2RSemaphore::take(uint32_t timeout_ms) 
{
    if (timeout_ms == 0) {
        return pthread_mutex_lock(&_lock) == 0;
    }
    if (take_nonblocking()) {
        return true;
    }
    uint32_t start = hal.scheduler->micros();
    do {
        hal.scheduler->delay_microseconds(200);
        if (take_nonblocking()) {
            return true;
        }
    } while ((hal.scheduler->micros() - start) < timeout_ms*1000);
    return false;
}

bool V2RSemaphore::take_nonblocking() 
{
    return pthread_mutex_trylock(&_lock) == 0;
}

#endif // CONFIG_HAL_BOARD
