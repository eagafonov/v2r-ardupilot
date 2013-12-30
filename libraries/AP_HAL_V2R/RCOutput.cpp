
#include "RCOutput.h"

using namespace V2R;

void V2RRCOutput::init(void* machtnichts) {}

void V2RRCOutput::set_freq(uint32_t chmask, uint16_t freq_hz) {}

uint16_t V2RRCOutput::get_freq(uint8_t ch) {
    return 50;
}

void V2RRCOutput::enable_ch(uint8_t ch)
{}

void V2RRCOutput::enable_mask(uint32_t chmask)
{}

void V2RRCOutput::disable_ch(uint8_t ch)
{}

void V2RRCOutput::disable_mask(uint32_t chmask)
{}

void V2RRCOutput::write(uint8_t ch, uint16_t period_us)
{}

void V2RRCOutput::write(uint8_t ch, uint16_t* period_us, uint8_t len)
{}

uint16_t V2RRCOutput::read(uint8_t ch) {
    return 900;
}

void V2RRCOutput::read(uint16_t* period_us, uint8_t len)
{}

