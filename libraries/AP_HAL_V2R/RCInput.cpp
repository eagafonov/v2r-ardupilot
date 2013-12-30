
#include "RCInput.h"

using namespace V2R;
V2RRCInput::V2RRCInput()
{}

void V2RRCInput::init(void* machtnichts)
{}

uint8_t V2RRCInput::valid_channels() {
    return 0;
}

uint16_t V2RRCInput::read(uint8_t ch) {
    if (ch == 2) return 900; /* throttle should be low, for safety */
    else return 1500;
}

uint8_t V2RRCInput::read(uint16_t* periods, uint8_t len) {
    for (uint8_t i = 0; i < len; i++){
        if (i == 2) periods[i] = 900;
        else periods[i] = 1500;
    }
    return len;
}

bool V2RRCInput::set_overrides(int16_t *overrides, uint8_t len) {
    return true;
}

bool V2RRCInput::set_override(uint8_t channel, int16_t override) {
    return true;
}

void V2RRCInput::clear_overrides()
{}

