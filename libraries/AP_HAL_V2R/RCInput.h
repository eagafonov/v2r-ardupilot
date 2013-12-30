
#ifndef __AP_HAL_V2R_RCINPUT_H__
#define __AP_HAL_V2R_RCINPUT_H__

#include <AP_HAL_V2R.h>

class V2R::V2RRCInput : public AP_HAL::RCInput {
public:
    V2RRCInput();
    void init(void* machtnichts);
    uint8_t  valid_channels();
    uint16_t read(uint8_t ch);
    uint8_t read(uint16_t* periods, uint8_t len);

    bool set_overrides(int16_t *overrides, uint8_t len);
    bool set_override(uint8_t channel, int16_t override);
    void clear_overrides();
};

#endif // __AP_HAL_V2R_RCINPUT_H__
