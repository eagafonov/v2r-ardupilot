
#ifndef __AP_HAL_V2R_ANALOGIN_H__
#define __AP_HAL_V2R_ANALOGIN_H__

#include <AP_HAL_V2R.h>

class V2R::V2RAnalogSource : public AP_HAL::AnalogSource {
public:
    V2RAnalogSource(float v);
    float read_average();
    float read_latest();
    void set_pin(uint8_t p);
    void set_stop_pin(uint8_t p);
    void set_settle_time(uint16_t settle_time_ms);
    float voltage_average();
    float voltage_latest();
    float voltage_average_ratiometric() { return voltage_average(); }
private:
    float _v;
};

class V2R::V2RAnalogIn : public AP_HAL::AnalogIn {
public:
    V2RAnalogIn();
    void init(void* implspecific);
    AP_HAL::AnalogSource* channel(int16_t n);
};
#endif // __AP_HAL_V2R_ANALOGIN_H__
