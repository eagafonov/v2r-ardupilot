/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef __AP_INERTIAL_SENSOR_INV_MPU_IIO_H__
#define __AP_INERTIAL_SENSOR_INV_MPU_IIO_H__

#include <inv_mpu_iio.h>
#include <stdint.h>
#include <AP_HAL.h>
#include <AP_Math.h>
#include <AP_Progmem.h>
#include "AP_InertialSensor.h"

// enable debug to see a register dump on startup
#define INV_MPU_IIO_DEBUG 0

// Define the hack 
#define APM_ACCEL_X_Y_SWAP


class AP_InertialSensor_INV_MPU_IIO : public AP_InertialSensor
{
public:

    AP_InertialSensor_INV_MPU_IIO();

    /* Concrete implementation of AP_InertialSensor functions: */
    bool                update();
    float               get_gyro_drift_rate();

    // wait for a sample to be available, with timeout in milliseconds
    bool                wait_for_sample(uint16_t timeout_ms);

    // get_delta_time returns the time period in seconds overwhich the sensor data was collected
    float            	get_delta_time();

    uint16_t error_count(void) const { return _error_count; }
    bool healthy(void) const { return _error_count <= 4; }

protected:
    uint16_t                    _init_sensor( Sample_rate sample_rate );

private:
    void                 _read_data_transaction();
    bool                 _data_ready();
    bool                 _hardware_init(Sample_rate sample_rate);

    iio::inv_mpu *_inv_mpu_iio;

    uint16_t					_num_samples;
    static const float          _gyro_scale;

    uint32_t _last_sample_time_micros;

    // ensure we can't initialise twice
    bool                        _initialised;

    // how many hardware samples before we report a sample to the caller
    uint8_t _sample_shift;

    // support for updating filter at runtime
    uint8_t _last_filter_hz;

    void _set_filter_register(uint8_t filter_hz, uint8_t default_filter);

    uint16_t _error_count;

    // accumulation in timer - must be read with timer disabled
    // the sum of the values since last read
    Vector3l _accel_sum;
    Vector3l _gyro_sum;
    volatile int16_t _sum_count;

public:

#if INV_MPU_IIO_DEBUG
    void						_dump_registers(void);
#endif
};

#endif // __AP_INERTIAL_SENSOR_INV_MPU_IIO_H__
