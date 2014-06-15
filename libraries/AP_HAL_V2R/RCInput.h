
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

    void process_input();

private:
    static const int MAX_CHANNELS = 8;
    static const uint16_t MIN_CHANNEL_VALUE = 1000;
    static const uint16_t MAX_CHANNEL_VALUE = 2000;
    static const uint16_t MID_CHANNEL_VALUE = (MIN_CHANNEL_VALUE + MAX_CHANNEL_VALUE) / 2;

    static const uint8_t CHANNEL_THR = 3; // channel #3 is throttle

    uint16_t _periods[MAX_CHANNELS];

    int mavlink_fd;

    uint8_t _valid_channels;
};

#endif // __AP_HAL_V2R_RCINPUT_H__
