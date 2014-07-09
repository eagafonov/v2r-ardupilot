
#include "GPIO.h"

#include <fstream>
#include <logger.h>

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
{
    // pin is threated as CON number
    // echo "set con 44 output 1" > /dev/v2r_pins

//     log_dbg() << "[GPIO] w " << (int) pin << " " << (int)value;

    value = value ? 1 : 0;

    _outputValues[pin] = value ;

    std::ofstream f("/dev/v2r_pins", std::ofstream::out);

    if (f.is_open()) {
        f << "set con " << (int)pin << " output " << (int)value << std::endl;
        f.close();
    } else {
        log_err() << "Failed to open /dev/v2r_pins to write";
    }
}

void V2RGPIO::toggle(uint8_t pin)
{
    OutputValues::iterator it = _outputValues.find(pin);

    if (it != _outputValues.end()) {
        write(pin, !it->second);
    } else {
        log_wrn() << "[GPIO] Failed to togle pin #" << (int)pin;
    }
}

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
