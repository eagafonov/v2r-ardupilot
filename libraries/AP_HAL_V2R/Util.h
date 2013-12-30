
#ifndef __AP_HAL_V2R_UTIL_H__
#define __AP_HAL_V2R_UTIL_H__

#include <AP_HAL.h>
#include "AP_HAL_V2R_Namespace.h"

class V2R::V2RUtil : public AP_HAL::Util {
public:
    bool run_debug_shell(AP_HAL::BetterStream *stream) { return false; }
};

#endif // __AP_HAL_V2R_UTIL_H__
