#include <AP_HAL.h>
#include <unistd.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_V2R

#include "Scheduler.h"
#include "Storage.h"
#include "UARTDriver.h"
#include "RCInput.h"
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/prctl.h>

using namespace V2R;

extern const AP_HAL::HAL& hal;
extern V2RUARTDriver uartConsole;
extern V2RRCInput rcinDriver;


#define APM_V2R_TIMER_PRIORITY    13
#define APM_V2R_UART_PRIORITY     12
#define APM_V2R_MAIN_PRIORITY     11
#define APM_V2R_IO_PRIORITY       10
#define APM_V2R_RC_PRIORITY       13

V2RScheduler::V2RScheduler()
{}

typedef void *(*pthread_startroutine_t)(void *);

/*
  setup for realtime. Lock all of memory in the thread and pre-fault
  the given stack size, so stack faults don't cause timing jitter
 */
void V2RScheduler::_setup_realtime(uint32_t size) 
{
        uint8_t dummy[size];
        mlockall(MCL_CURRENT|MCL_FUTURE);
        memset(dummy, 0, sizeof(dummy));
}

void V2RScheduler::init(void* machtnichts)
{
    clock_gettime(CLOCK_MONOTONIC, &_sketch_start_time);

    _setup_realtime(32768);

    pthread_attr_t thread_attr;
    struct sched_param param;

    memset(&param, 0, sizeof(param));

    param.sched_priority = APM_V2R_MAIN_PRIORITY;
    sched_setscheduler(0, SCHED_FIFO, &param);

    param.sched_priority = APM_V2R_TIMER_PRIORITY;
    pthread_attr_init(&thread_attr);
    (void)pthread_attr_setschedparam(&thread_attr, &param);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);

    pthread_create(&_timer_thread_ctx, &thread_attr, (pthread_startroutine_t)&V2R::V2RScheduler::_timer_thread, this);

    // the UART thread runs at a medium priority
    pthread_attr_init(&thread_attr);
    param.sched_priority = APM_V2R_UART_PRIORITY;
    (void)pthread_attr_setschedparam(&thread_attr, &param);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);

    pthread_create(&_uart_thread_ctx, &thread_attr, (pthread_startroutine_t)&V2R::V2RScheduler::_uart_thread, this);
    
    // the IO thread runs at lower priority
    pthread_attr_init(&thread_attr);
    param.sched_priority = APM_V2R_IO_PRIORITY;
    (void)pthread_attr_setschedparam(&thread_attr, &param);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    
    pthread_create(&_io_thread_ctx, &thread_attr, (pthread_startroutine_t)&V2R::V2RScheduler::_io_thread, this);
    
    // RC Input thread
    pthread_attr_init(&thread_attr);
    param.sched_priority = APM_V2R_RC_PRIORITY;
    (void)pthread_attr_setschedparam(&thread_attr, &param);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    
    pthread_create(&_rc_thread_ctx, &thread_attr, (pthread_startroutine_t)&V2R::V2RScheduler::_rc_thread, this);
}

void V2RScheduler::_microsleep(uint32_t usec)
{
    usleep(usec);
}

void V2RScheduler::delay(uint16_t ms)
{
    if (_delay_cb) {
        uint32_t start = millis();

        while ((millis() - start) < ms) {
            // this yields the CPU to other apps
            _microsleep(1000);
            if (_min_delay_cb_ms <= ms) {
                _delay_cb();
            }
        }
    } else {
        _microsleep(ms * 1000);
    }
}

uint32_t V2RScheduler::millis() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return 1.0e3*((ts.tv_sec + (ts.tv_nsec*1.0e-9)) - 
                  (_sketch_start_time.tv_sec +
                   (_sketch_start_time.tv_nsec*1.0e-9)));
}

uint32_t V2RScheduler::micros() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return 1.0e6*((ts.tv_sec + (ts.tv_nsec*1.0e-9)) - 
                  (_sketch_start_time.tv_sec +
                   (_sketch_start_time.tv_nsec*1.0e-9)));
}

void V2RScheduler::delay_microseconds(uint16_t us)
{
    _microsleep(us);
}

void V2RScheduler::register_delay_callback(AP_HAL::Proc proc,
                                             uint16_t min_time_ms)
{
    _delay_cb = proc;
    _min_delay_cb_ms = min_time_ms;
}

void V2RScheduler::register_timer_process(AP_HAL::MemberProc proc) 
{
    for (uint8_t i = 0; i < _num_timer_procs; i++) {
        if (_timer_proc[i] == proc) {
            return;
        }
    }

    if (_num_timer_procs < V2R_SCHEDULER_MAX_TIMER_PROCS) {
        _timer_proc[_num_timer_procs] = proc;
        _num_timer_procs++;
    } else {
        hal.console->printf("Out of timer processes\n");
    }
}

void V2RScheduler::register_io_process(AP_HAL::MemberProc proc) 
{
    for (uint8_t i = 0; i < _num_io_procs; i++) {
        if (_io_proc[i] == proc) {
            return;
        }
    }

    if (_num_io_procs < V2R_SCHEDULER_MAX_TIMER_PROCS) {
        _io_proc[_num_io_procs] = proc;
        _num_io_procs++;
    } else {
        hal.console->printf("Out of IO processes\n");
    }
}

void V2RScheduler::register_timer_failsafe(AP_HAL::Proc failsafe, uint32_t period_us)
{
    _failsafe = failsafe;
}

void V2RScheduler::suspend_timer_procs()
{
    _timer_suspended = true;
    while (_in_timer_proc) {
        usleep(1);
    }
}

void V2RScheduler::resume_timer_procs()
{
    _timer_suspended = false;
    if (_timer_event_missed == true) {
        _run_timers(false);
        _timer_event_missed = false;
    }
}

void V2RScheduler::_run_timers(bool called_from_timer_thread)
{
    if (_in_timer_proc) {
        return;
    }
    _in_timer_proc = true;

    if (!_timer_suspended) {
        // now call the timer based drivers
        for (int i = 0; i < _num_timer_procs; i++) {
            if (_timer_proc[i] != NULL) {
                _timer_proc[i]();
            }
        }
    } else if (called_from_timer_thread) {
        _timer_event_missed = true;
    }

    // and the failsafe, if one is setup
    if (_failsafe != NULL) {
        _failsafe();
    }

    _in_timer_proc = false;
}

void *V2RScheduler::_timer_thread(void)
{
    prctl(PR_SET_NAME,"ArduPilot timer",0,0,0);

    _setup_realtime(32768);
    while (system_initializing()) {
        poll(NULL, 0, 1001);
    }
    while (true) {
        _microsleep(5000);

        // run registered timers
        _run_timers(true);

    }
    return NULL;
}

void V2RScheduler::_run_io(void)
{
    if (_in_io_proc) {
        return;
    }
    _in_io_proc = true;

    if (!_timer_suspended) {
        // now call the IO based drivers
        for (int i = 0; i < _num_io_procs; i++) {
            if (_io_proc[i] != NULL) {
                _io_proc[i]();
            }
        }
    }

    _in_io_proc = false;
}

void *V2RScheduler::_uart_thread(void)
{
    prctl(PR_SET_NAME,"ArduPilot uart",0,0,0);
    
    _setup_realtime(32768);
    while (system_initializing()) {
        poll(NULL, 0, 1002);
        // we still need to pump console messages
        uartConsole._timer_tick();
    }
    while (true) {
        _microsleep(10000);

        // process any pending serial bytes
        ((V2RUARTDriver *)hal.uartA)->_timer_tick();
        ((V2RUARTDriver *)hal.uartB)->_timer_tick();
        ((V2RUARTDriver *)hal.uartC)->_timer_tick();

        uartConsole._timer_tick();
    }
    return NULL;
}

void *V2RScheduler::_io_thread(void)
{
    prctl(PR_SET_NAME,"ArduPilot IO",0,0,0);

    _setup_realtime(32768);
    while (system_initializing()) {
        poll(NULL, 0, 1003);        
    }
    while (true) {
        _microsleep(20000);

        // process any pending storage writes
        ((V2RStorage *)hal.storage)->_timer_tick();

        // run registered IO processes
        _run_io();
    }
    return NULL;
}

void *V2RScheduler::_rc_thread(void)
{
    prctl(PR_SET_NAME,"ArduPilot RC",0,0,0);

    _setup_realtime(32768);
    while (system_initializing()) {
        poll(NULL, 0, 1004);
    }
    while (true) {
        rcinDriver.process_input();
    }
    return NULL;
}

void V2RScheduler::panic(const prog_char_t *errormsg) 
{
    write(1, errormsg, strlen(errormsg));
    write(1, "\n", 1);
    hal.scheduler->delay_microseconds(10000);
    exit(1);
}

bool V2RScheduler::in_timerprocess() 
{
    return _in_timer_proc;
}

void V2RScheduler::begin_atomic()
{}

void V2RScheduler::end_atomic()
{}

bool V2RScheduler::system_initializing() {
    return !_initialized;
}

void V2RScheduler::system_initialized()
{
    if (_initialized) {
        panic("PANIC: scheduler::system_initialized called more than once");
    }
    _initialized = true;
}

void V2RScheduler::reboot(bool hold_in_bootloader) 
{
    for(;;);
}

#endif // CONFIG_HAL_BOARD
