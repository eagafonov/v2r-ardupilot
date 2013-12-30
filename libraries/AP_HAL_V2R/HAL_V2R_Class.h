
#ifndef __AP_HAL_V2R_CLASS_H__
#define __AP_HAL_V2R_CLASS_H__

#include <AP_HAL.h>

#include "AP_HAL_V2R_Namespace.h"

class HAL_V2R : public AP_HAL::HAL {
public:
    HAL_V2R();
    void init(int argc, char * const * argv) const;
};

extern const HAL_V2R AP_HAL_V2R;

#endif // __AP_HAL_V2R_CLASS_H__

