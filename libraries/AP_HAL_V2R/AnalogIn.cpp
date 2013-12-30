#include "AnalogIn.h"

using namespace V2R;

V2RAnalogSource::V2RAnalogSource(float v) :
    _v(v)
{}

float V2RAnalogSource::read_average() {
    return _v;
}

float V2RAnalogSource::voltage_average() {
    return 5.0 * _v / 1024.0;
}

float V2RAnalogSource::voltage_latest() {
    return 5.0 * _v / 1024.0;
}

float V2RAnalogSource::read_latest() {
    return _v;
}

void V2RAnalogSource::set_pin(uint8_t p)
{}

void V2RAnalogSource::set_stop_pin(uint8_t p)
{}

void V2RAnalogSource::set_settle_time(uint16_t settle_time_ms)
{}

V2RAnalogIn::V2RAnalogIn()
{}

void V2RAnalogIn::init(void* machtnichts)
{}

AP_HAL::AnalogSource* V2RAnalogIn::channel(int16_t n) {
    return new V2RAnalogSource(1.11);
}


