/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/AnalogGyro.h"

#include <chrono>
#include <thread>
#include <AHRS.h>

#include "AnalogInternal.h"
#include "HAL/AnalogAccumulator.h"
#include "HAL/AnalogInput.h"
#include "HAL/handles/IndexedHandleResource.h"
#include "HALInitializer.h"
#include "Translator/VMXHandler.h"
#include <AHRS.h>

using namespace hal;

namespace hal {
    struct AnalogGyro {
        HAL_AnalogInputHandle handle;
        uint8_t index;
    };

    static IndexedHandleResource<HAL_GyroHandle, AnalogGyro, kNumAccumulators,
            HAL_HandleEnum::AnalogGyro>* analogGyroHandles;

    namespace init {
        void InitializeAnalogGyro() {
            static IndexedHandleResource<HAL_GyroHandle, AnalogGyro, kNumAccumulators,
                    HAL_HandleEnum::AnalogGyro> agH;
            analogGyroHandles = &agH;
        }
    }
}

extern "C" {
    HAL_GyroHandle HAL_InitializeAnalogGyro(HAL_AnalogInputHandle analogHandle, int32_t* status) {
        hal::init::CheckInit();
        if (!HAL_IsAccumulatorChannel(analogHandle, status)) {
            if (*status == 0) {
                *status = HAL_INVALID_ACCUMULATOR_CHANNEL;
            }
            return HAL_kInvalidHandle;
        }

        // handle known to be correct, so no need to type check
        int16_t channel = getHandleIndex(analogHandle);

        auto handle = analogGyroHandles->Allocate(channel, status);

        if (*status != 0)
            return HAL_kInvalidHandle;  // failed to allocate. Pass error back.

        // Initialize port structure
        auto gyro = analogGyroHandles->Get(handle);
        if (gyro == nullptr) {  // would only error on thread issue
            *status = HAL_HANDLE_ERROR;
            return HAL_kInvalidHandle;
        }

        gyro->handle = analogHandle;
        gyro->index = channel;

    //    SimAnalogGyroData[channel].SetInitialized(true);

        return handle;
    }

    void HAL_SetupAnalogGyro(HAL_GyroHandle handle, int32_t* status) {
        // No op
    }

    void HAL_FreeAnalogGyro(HAL_GyroHandle handle) {
        auto gyro = analogGyroHandles->Get(handle);
        analogGyroHandles->Free(handle);
        if (gyro == nullptr) return;
    //    SimAnalogGyroData[gyro->index].SetInitialized(false);
    }

    void HAL_SetAnalogGyroParameters(HAL_GyroHandle handle, double voltsPerDegreePerSecond, double offset,
                                     int32_t center, int32_t* status) {
        // No op
    }

    void HAL_SetAnalogGyroVoltsPerDegreePerSecond(HAL_GyroHandle handle, double voltsPerDegreePerSecond,
                                                  int32_t* status) {
        // No op
    }

    void HAL_ResetAnalogGyro(HAL_GyroHandle handle, int32_t* status) {
        auto gyro = analogGyroHandles->Get(handle);
        if (gyro == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }

        vmxIMU->ZeroYaw();
    }

    void HAL_CalibrateAnalogGyro(HAL_GyroHandle handle, int32_t* status) {
        // Calibration occurs automatically: no-op
    }

    void HAL_SetAnalogGyroDeadband(HAL_GyroHandle handle, double volts, int32_t* status) {
        // no-op
    }

    double HAL_GetAnalogGyroAngle(HAL_GyroHandle handle, int32_t* status) {
        auto gyro = analogGyroHandles->Get(handle);
        if (gyro == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return 0;
        }

        return vmxIMU->GetYaw();
    }

    double HAL_GetAnalogGyroRate(HAL_GyroHandle handle, int32_t* status) {
        auto gyro = analogGyroHandles->Get(handle);
        if (gyro == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return 0;
        }

        return vmxIMU->GetRate();
    }

    double HAL_GetAnalogGyroOffset(HAL_GyroHandle handle, int32_t* status) {
        // no-op
        // TODO: Add GetAnalogGyroOffset functionality to VMX-pi HAL [Issue: #93]
        return 0.0;
    }

    int32_t HAL_GetAnalogGyroCenter(HAL_GyroHandle handle, int32_t* status) {
        // no-op
        // TODO: GetAnalogGyroCenter functionality to VMX-pi HAL [Issue: #93]
        return 0;
    }
}

