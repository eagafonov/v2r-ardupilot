
#ifndef __AP_HAL_V2R_SEMAPHORE_H__
#define __AP_HAL_V2R_SEMAPHORE_H__

#include <AP_HAL_Boards.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_V2R
#include <AP_HAL_V2R.h>
#include <pthread.h>

class V2R::V2RSemaphore : public AP_HAL::Semaphore {
public:
    V2RSemaphore() {
        pthread_mutex_init(&_lock, NULL);
    }
    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();
private:
    pthread_mutex_t _lock;
};
#endif // CONFIG_HAL_BOARD

#endif // __AP_HAL_V2R_SEMAPHORE_H__
