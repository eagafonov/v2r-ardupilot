
#include "RCOutput.h"
#include <logger.h>
#include <fstream>

using namespace V2R;

// v2r hardware PWM operates on 24MHz

// Tics per sec
#define PWM_TICKS_PER_SEC 24000000

// Ticks per usec
#define PWM_TICKS_PER_MICROSEC 24

// static int freq_to_pwm_ticks(uint16_t freq_hz) {
//     int rv(PWM_TICKS_PER_SEC / freq_hz);
//     return rv;
// };

V2RRCOutput::V2RRCOutput()
{
    memset(_pwm, 0, sizeof(_pwm));
}


void V2RRCOutput::init(void* machtnichts)
{
    log_dbg() << __PRETTY_FUNCTION__;

    std::ofstream f("/dev/v2r_pins", std::ofstream::out);

    if (f.is_open()) {
        f << "set con43 pwm0" << std::endl
          << "set con44 pwm1" << std::endl
          << "set con16 pwm2" << std::endl
          << "set con17 pwm2" << std::endl
          << "set con42 pwm3" << std::endl;

        f.close();
    } else {
        log_err() << "Failed to open /dev/v2r_pins to write";
    }

}

void V2RRCOutput::set_freq(uint32_t chmask, uint16_t freq_hz) {
    log_dbg() << __PRETTY_FUNCTION__ <<	"chmask:0x" << std::hex << chmask <<	" freq:" << std::dec << freq_hz;

    int m = 1;

    for (int i=0; i < V2R_NUM_OUTPUT_CHANNELS; i++, m <<= 1) {
        if (chmask & m) {
            _pwm[i].freq_hz = freq_hz;
            _pwm[i]._period_tiks = PWM_TICKS_PER_SEC / freq_hz;
            _pwm[i]._dirty = true;
        }
    }
}

uint16_t V2RRCOutput::get_freq(uint8_t ch) {
    if (ch >= V2R_NUM_OUTPUT_CHANNELS) {
        log_err() << "V2RRCOutput::get_freq invalid channel " << ch;
        return 0;
    }

    log_dbg() << __PRETTY_FUNCTION__ << " ch:" << (int)ch;

    return _pwm[ch].freq_hz;
}

void V2RRCOutput::enable_ch(uint8_t ch)
{
    // channels are always enabled ...
}

void V2RRCOutput::enable_mask(uint32_t chmask)
{
    // channels are always enabled ...
}

void V2RRCOutput::disable_ch(uint8_t ch)
{
    // channels are always enabled ...
}

void V2RRCOutput::disable_mask(uint32_t chmask)
{
    // channels are always enabled ...
}


void V2RRCOutput::write(uint8_t ch, uint16_t period_us)
{
    _write(ch, period_us, true);
}

void V2RRCOutput::_write(uint8_t ch, uint16_t period_us, bool write_to_hardware)
{
    log_dbg() << __PRETTY_FUNCTION__ <<	" ch:" << (int)ch << " period:" << period_us;

    if (ch >= V2R_NUM_OUTPUT_CHANNELS) {
        log_err() << "V2RRCOutput::write invalid channel " << ch;
        return;
    }

    _pwm[ch].period_us = period_us;
    _pwm[ch]._duty_tiks = period_us * PWM_TICKS_PER_MICROSEC;
    _pwm[ch]._dirty = true;

    if (write_to_hardware) {
        _pwm[ch]._write_pwm(ch);
    }
}

void V2RRCOutput::write(uint8_t ch, uint16_t* period_us, uint8_t len)
{
    log_dbg() << __PRETTY_FUNCTION__ <<	"channel:" << (int)ch;

    for (uint8_t i=0; i<len; i++) {
        _write(i, period_us[i], false);
    }

    _write_pwm();
}

uint16_t V2RRCOutput::read(uint8_t ch) {
    log_dbg() << __PRETTY_FUNCTION__;

    if (ch >= V2R_NUM_OUTPUT_CHANNELS) {
        log_err() << "V2RRCOutput::read invalid channel " << ch;
        return 0;
    }

    return _pwm[ch].period_us;
}

void V2RRCOutput::read(uint16_t* period_us, uint8_t len)
{
    log_dbg() << __PRETTY_FUNCTION__;
}



void V2RRCOutput::_write_pwm()
{
    for (size_t i = 0; i < sizeof(_pwm) / sizeof(_pwm[0]); i++) {
        if (_pwm[i]._dirty) {
            _pwm[i]._write_pwm(i);
        }
    }
}

void V2RRCOutput::pwm_info::_write_pwm(int channel)
{
    log_dbg() << "PWM ch:" << channel << " duty:" << _duty_tiks << " period:"<< _period_tiks;

    std::ofstream f("/dev/v2r_pins", std::ofstream::out);

    if (f.is_open()) {
        f << "set pwm" << channel << " duty:" << _duty_tiks	<< " period:" << _period_tiks;
        f.close();
    } else {
        log_err() << "Failed to open /dev/v2r_pins to write";
    }

    _dirty = false;
}

