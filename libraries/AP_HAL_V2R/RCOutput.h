
#ifndef __AP_HAL_V2R_RCOUTPUT_H__
#define __AP_HAL_V2R_RCOUTPUT_H__

#include <AP_HAL_V2R.h>

#define V2R_NUM_OUTPUT_CHANNELS 4

class V2R::V2RRCOutput : public AP_HAL::RCOutput {
public:
    V2RRCOutput();
    void     init(void* machtnichts);
    void     set_freq(uint32_t chmask, uint16_t freq_hz);
    uint16_t get_freq(uint8_t ch);
    void     enable_ch(uint8_t ch);
    void     enable_mask(uint32_t chmask);
    void     disable_ch(uint8_t ch);
    void     disable_mask(uint32_t chmask);
    void     write(uint8_t ch, uint16_t period_us);
    void     write(uint8_t ch, uint16_t* period_us, uint8_t len);
    uint16_t read(uint8_t ch);
    void     read(uint16_t* period_us, uint8_t len);
private:
    struct pwm_info {
        uint16_t freq_hz;
        uint16_t period_us;
        int _duty_tiks;
        int _period_tiks;
        bool _dirty;

        void  _write_pwm(int channel);
    };

    void _write(uint8_t ch, uint16_t period_us, bool write_to_hardware);
    void _write_pwm();


    pwm_info _pwm[V2R_NUM_OUTPUT_CHANNELS];
};

#endif // __AP_HAL_V2R_RCOUTPUT_H__
