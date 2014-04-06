/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "AP_InertialSensor_INV_MPU_IIO.h"
#include <logger.h>
#include <iomanip>
#include <inv_mpu_iio.h>
#include <AP_HAL_V2R_Namespace.h>
#include <unistd.h>

/*
 *  RM-MPU-6000A-00.pdf, page 31, section 4.23 lists LSB sensitivity of
 *  accel as 4096 LSB/mg at scale factor of +/- 8g (AFS_SEL==2)
 *
 *  See note below about accel scaling of engineering sample MPU6k
 *  variants however
 */

// INV_MPU_IIO accelerometer scaling
#define INV_MPU_IIO_ACCEL_SCALE_1G    (GRAVITY_MSS / 4096.0f)

/*
 *  RM-MPU-6000A-00.pdf, page 33, section 4.25 lists LSB sensitivity of
 *  gyro as 16.4 LSB/DPS at scale factor of +/- 2000dps (FS_SEL==3)
 */
const float AP_InertialSensor_INV_MPU_IIO::_gyro_scale = (0.0174532f / 16.4f);

AP_InertialSensor_INV_MPU_IIO::AP_InertialSensor_INV_MPU_IIO() : 
	AP_InertialSensor(),
    _initialised(false)
{
}

uint16_t AP_InertialSensor_INV_MPU_IIO::_init_sensor( Sample_rate sample_rate )
{
    log_dbg() << "INV_MPU_IIO::_init_sensor";
    if (_initialised) return 0; //_mpu6000_product_id; TODO change return type
    _initialised = true;

    _inv_mpu_iio = new iio::inv_mpu(0);

    if (_hardware_init(sample_rate)) {
        log_wrn() << "Failed to initialize IIO";

        if (!wait_for_sample(500)) {
            log_inf() << "INV_MPU_IIO startup failed: no data";
        }
    } else {
         log_err() << "Failed to initialize IIO";
    }

#if INV_MPU_IIO_DEBUG
    _dump_registers();
#endif
    log_dbg() << "INV_MPU_IIO::_init_sensor done. Chip name: " << _inv_mpu_iio->chip_name();

    return 0;
}

/*================ AP_INERTIALSENSOR PUBLIC INTERFACE ==================== */

bool AP_InertialSensor_INV_MPU_IIO::wait_for_sample(uint16_t timeout_ms)
{
    int r = _inv_mpu_iio->wait(timeout_ms);

    return r > 0;
}

bool AP_InertialSensor_INV_MPU_IIO::update( void )
{
    // wait for at least 1 sample
    if (!wait_for_sample(1000)) {
        log_dbg() << "INV_MPU_IIO update not now yet";
        return false;
    }

    _read_data_transaction();

    _previous_accel[0] = _accel[0];

    // disable timer procs for mininum time
    _gyro[0]  = Vector3f(_gyro_sum.x, _gyro_sum.y, _gyro_sum.z);
    _accel[0] = Vector3f(_accel_sum.x, _accel_sum.y, _accel_sum.z);
    _num_samples = _sum_count;
    _accel_sum.zero();
    _gyro_sum.zero();
    _sum_count = 0;

    _gyro[0].rotate(_board_orientation);
    _gyro[0] *= _gyro_scale / _num_samples;
    _gyro[0] -= _gyro_offset[0];

    _accel[0].rotate(_board_orientation);
    _accel[0] *= INV_MPU_IIO_ACCEL_SCALE_1G / _num_samples;

    Vector3f accel_scale = _accel_scale[0].get();
    _accel[0].x *= accel_scale.x;
    _accel[0].y *= accel_scale.y;
    _accel[0].z *= accel_scale.z;
    _accel[0] -= _accel_offset[0];

    if (_last_filter_hz != _mpu6000_filter) {
        _set_filter_register(_mpu6000_filter, 0);
        _error_count = 0;
    }

    return true;
}

/*================ HARDWARE FUNCTIONS ==================== */

/**
 * Return true if the INV_MPU_IIO has new data available for reading.
 *
 * We use the data ready pin if it is available.  Otherwise, read the
 * status register.
 */
bool AP_InertialSensor_INV_MPU_IIO::_data_ready()
{
   int ready_count = _inv_mpu_iio->wait(0);

   return ready_count > 0;
}

void AP_InertialSensor_INV_MPU_IIO::_read_data_transaction() {
    typedef struct {
        int16_t accl_x;
        int16_t accl_y;
        int16_t accl_z;
        int16_t gyro_x; 
        int16_t gyro_y; 
        int16_t gyro_z; 
    } gyro_accl_t;

    static gyro_accl_t buffer[30];

    int r = read(_inv_mpu_iio->fd(), buffer, sizeof(buffer));

//             cerr << '.';
//             std::cerr << count << " got " << r << endl;

    if (r % sizeof(buffer[0])) {
        log_wrn() << "Unaligned PMU data read. size:" << r;
    };

    r /= sizeof(buffer[0]);

    gyro_accl_t *p = buffer;

    for (int i = 0; i < r; i++, p++) {
#ifdef APM_ACCEL_X_Y_SWAP
        // X and Y is swapped due to hardware layout or some other reason
        // http://ardupilot.com/forum/viewtopic.php?f=69&t=5322
        _accel_sum.x += p->accl_y;
        _accel_sum.y += p->accl_x;
        _accel_sum.z -= p->accl_z;
        _gyro_sum.x  += p->gyro_y;
        _gyro_sum.y  += p->gyro_x;
        _gyro_sum.z  -= p->gyro_z;  // Z axis goes up but gravity goes down, Fix this inconvinience here
#else
        _accel_sum.x += p->accl_x;
        _accel_sum.y += p->accl_y;
        _accel_sum.z += p->accl_z;
        _gyro_sum.x  += p->gyro_x;
        _gyro_sum.y  += p->gyro_y;
        _gyro_sum.z  += p->gyro_z;
#endif

//         log_dbg()
//             << std::setw(5) << p->accl_x << " " 
//             << std::setw(5) << p->accl_y << " "
//             << std::setw(5) << p->accl_z << " "
//             << " - "
//             << std::setw(5) << p->gyro_x << " "
//             << std::setw(5) << p->gyro_y << " "
//             << std::setw(5) << p->gyro_z << " ";

        _sum_count++;
    }

    if (_sum_count == 0) {
        // rollover - v unlikely
        _accel_sum.zero();
        _gyro_sum.zero();
    }
}

/*
  set the DLPF filter frequency. Assumes caller has taken semaphore
 */
void AP_InertialSensor_INV_MPU_IIO::_set_filter_register(uint8_t filter_hz, uint8_t default_filter)
{
    return; // TODO Chek if implemented in IIO driver

#if 0
    uint8_t filter = default_filter;
    // choose filtering frequency
    switch (filter_hz) {
    case 5:
        filter = BITS_DLPF_CFG_5HZ;
        break;
    case 10:
        filter = BITS_DLPF_CFG_10HZ;
        break;
    case 20:
        filter = BITS_DLPF_CFG_20HZ;
        break;
    case 42:
        filter = BITS_DLPF_CFG_42HZ;
        break;
    case 98:
        filter = BITS_DLPF_CFG_98HZ;
        break;
    }

    if (filter != 0) {
        _last_filter_hz = filter_hz;

        _register_write(MPUREG_CONFIG, filter);
    }
#endif
}

bool AP_InertialSensor_INV_MPU_IIO::_hardware_init(Sample_rate sample_rate)
{
    log_dbg() << "INV_MPU_IIO::_hardware_init " << sample_rate;

    _inv_mpu_iio->init();

    _inv_mpu_iio->enable_buffer(false);

    _inv_mpu_iio->disable_all();

    _inv_mpu_iio->gyro_enable(true);
    _inv_mpu_iio->accl_enable(true);
    _inv_mpu_iio->accl_scale(iio::inv_mpu::ACCEL_SCALE_8g);
    _inv_mpu_iio->gyro_scale(iio::inv_mpu::GYRO_SCALE_2000dps);
//     mpu.timestamp_enable(true); // Update _read_transaction if enabled

    switch (sample_rate) {
        case RATE_50HZ:
            _inv_mpu_iio->sampling_freq(iio::inv_mpu::SAMPLING_50HZ);
            break;
        case RATE_100HZ:
            _inv_mpu_iio->sampling_freq(iio::inv_mpu::SAMPLING_100HZ);
            break;
        case RATE_200HZ:
            _inv_mpu_iio->sampling_freq(iio::inv_mpu::SAMPLING_200HZ);
            break;
    }

#if 0
// TODO Implement filter settings
    uint8_t default_filter;

    // sample rate and filtering
    // to minimise the effects of aliasing we choose a filter
    // that is less than half of the sample rate
    switch (sample_rate) {
    case RATE_50HZ:
        // this is used for plane and rover, where noise resistance is
        // more important than update rate. Tests on an aerobatic plane
        // show that 10Hz is fine, and makes it very noise resistant
        default_filter = BITS_DLPF_CFG_10HZ;
        _sample_shift = 2;
        break;
    case RATE_100HZ:
        default_filter = BITS_DLPF_CFG_20HZ;
        _sample_shift = 1;
        break;
    case RATE_200HZ:
    default:
        default_filter = BITS_DLPF_CFG_20HZ;
        _sample_shift = 0;
        break;
    }

    _set_filter_register(_mpu6000_filter, default_filter);
#endif

    _inv_mpu_iio->enable_buffer(true);

    log_dbg() << "INV_MPU_IIO::_hardware_init complete";
    return true;
}

// return the MPU6k gyro drift rate in radian/s/s
// note that this is much better than the oilpan gyros
float AP_InertialSensor_INV_MPU_IIO::get_gyro_drift_rate(void)
{
    // 0.5 degrees/second/minute
    return ToRad(0.5/60);
}

#if INV_MPU_IIO_DEBUG
// dump all config registers - used for debug
void AP_InertialSensor_INV_MPU_IIO::_dump_registers(void)
{
//     hal.console->println_P(PSTR("INV_MPU_IIO registers"));
//     for (uint8_t reg=MPUREG_PRODUCT_ID; reg<=108; reg++) {
//         uint8_t v = _register_read(reg);
//         hal.console->printf_P(PSTR("%02x:%02x "), (unsigned)reg, (unsigned)v);
//         if ((reg - (MPUREG_PRODUCT_ID-1)) % 16 == 0) {
//             hal.console->println();
//         }
//     }
//     hal.console->println();
}
#endif


// get_delta_time returns the time period in seconds overwhich the sensor data was collected
float AP_InertialSensor_INV_MPU_IIO::get_delta_time() 
{
    // the sensor runs at 200Hz
    return 0.005 * _num_samples;
}
