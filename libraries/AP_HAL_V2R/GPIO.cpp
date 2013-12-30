
#include "GPIO.h"

using namespace V2R;

V2RGPIO::V2RGPIO()
{}

void V2RGPIO::init()
{}

void V2RGPIO::pinMode(uint8_t pin, uint8_t output)
{}

int8_t V2RGPIO::analogPinToDigitalPin(uint8_t pin)
{
	return -1;
}


uint8_t V2RGPIO::read(uint8_t pin) {
    return 0;
}

void V2RGPIO::write(uint8_t pin, uint8_t value)
{}

void V2RGPIO::toggle(uint8_t pin)
{}

/* Alternative interface: */
AP_HAL::DigitalSource* V2RGPIO::channel(uint16_t n) {
    return new V2RDigitalSource(0);
}

/* Interrupt interface: */
bool V2RGPIO::attach_interrupt(uint8_t interrupt_num, AP_HAL::Proc p,
        uint8_t mode) {
    return true;
}

bool V2RGPIO::usb_connected(void)
{
    return false;
}

V2RDigitalSource::V2RDigitalSource(uint8_t v) :
    _v(v)
{}

void V2RDigitalSource::mode(uint8_t output)
{}

uint8_t V2RDigitalSource::read() {
    return _v;
}

void V2RDigitalSource::write(uint8_t value) {
    _v = value;
}

void V2RDigitalSource::toggle() {
    _v = !_v;
}
