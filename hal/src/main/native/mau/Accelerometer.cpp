/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <AHRS.h>
#include "HALInitializer.h"
#include "HAL/Accelerometer.h"
#include "VMXHandler.h"

namespace hal {
    namespace init {
        void InitializeAccelerometer() {}
    }
}

extern "C" {
    void HAL_SetAccelerometerActive(HAL_Bool active) {
        // Always active: no-op
    }

    void HAL_SetAccelerometerRange(HAL_AccelerometerRange range) {
        // VMX-pi's Accelerometer range is fixed: no-op
    }

    double HAL_GetAccelerometerX(void) {
        return (double) vmxIMU->GetRawAccelX();
    }

    double HAL_GetAccelerometerY(void) {
        return (double) vmxIMU->GetRawAccelY();
    }

    double HAL_GetAccelerometerZ(void) {
        return (double) vmxIMU->GetRawAccelZ();
    }
}