
#include "RCInput.h"

using namespace V2R;

V2RRCInput::V2RRCInput()
{
    for(int i = 0; i < sizeof(_periods) / sizeof(_periods[0]); i++) {
        _periods[i] = MID_CHANNEL_VALUE;
    }

    _periods[CHANNEL_THR] = MIN_CHANNEL_VALUE;
}

void V2RRCInput::init(void* machtnichts)
{}

uint8_t V2RRCInput::valid_channels() {
    return 0xFF; // All 8 channels are supported
}

uint16_t V2RRCInput::read(uint8_t ch) {
    if (ch > MAX_CHANNELS) {
        return 0;
    }

    return _periods[ch];
}

uint8_t V2RRCInput::read(uint16_t* periods, uint8_t len) {
    if (len > MAX_CHANNELS) {
        len = MAX_CHANNELS;
    }
    for (uint8_t i = 0; i < len; i++){
        periods[i] = _periods[i];
    }
    return len;
}

bool V2RRCInput::set_overrides(int16_t *overrides, uint8_t len) {
    if (len > MAX_CHANNELS) {
        len = MAX_CHANNELS;
    }

    for (int i = 0; i < len; i++) {
        set_override(i, overrides[i]);
    }

    return true;
}

bool V2RRCInput::set_override(uint8_t channel, int16_t override) {
    if (channel < MAX_CHANNELS) {
        if ((override >= MIN_CHANNEL_VALUE) && (override <= MAX_CHANNEL_VALUE)) {
            _periods[channel] = override;
        }
        return true;
    } else {
        return false;
    }
}

void V2RRCInput::clear_overrides()
{
    // Overrides is the only supporeted fro V2R yet
}

