/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/DIO.h"

#include <cmath>

#include "DigitalInternal.h"
#include "HAL/handles/HandlesInternal.h"
#include "HAL/handles/LimitedHandleResource.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "MauInternal.h"
#include <VMXIO.h>
#include <VMXResource.h>

using namespace hal;
using namespace mau;

static LimitedHandleResource<HAL_DigitalPWMHandle, uint8_t,
        kNumDigitalPWMOutputs, HAL_HandleEnum::DigitalPWM>*
        digitalPWMHandles;

namespace hal {
    namespace init {
        void InitializeDIO() {
            static LimitedHandleResource<HAL_DigitalPWMHandle, uint8_t,
                    kNumDigitalPWMOutputs, HAL_HandleEnum::DigitalPWM> dpH;
            digitalPWMHandles = &dpH;
        }
    }
}

extern "C" {
    /**
     * Create a new instance of a digital port.
     */
    HAL_DigitalHandle HAL_InitializeDIOPort(HAL_PortHandle portHandle, HAL_Bool input, int32_t* status) {
        hal::init::CheckInit();
        if (*status != 0) return HAL_kInvalidHandle;

        int16_t channel = getPortHandleChannel(portHandle);
        if (channel == InvalidHandleIndex) {
            *status = PARAMETER_OUT_OF_RANGE;
            return HAL_kInvalidHandle;
        }

        auto handle =
                digitalChannelHandles->Allocate(channel, HAL_HandleEnum::DIO, status);

        if (*status != 0)
            return HAL_kInvalidHandle;  // failed to allocate. Pass error back.

        auto port = digitalChannelHandles->Get(handle, HAL_HandleEnum::DIO);
        if (port == nullptr) {  // would only occur on thread issue.
            *status = HAL_HANDLE_ERROR;
            return HAL_kInvalidHandle;
        }

        port->channel = static_cast<uint8_t>(channel);

//        SimDIOData[channel].SetInitialized(true);
////
//        SimDIOData[channel].SetIsInput(input);

        return handle;
    }

    HAL_Bool HAL_CheckDIOChannel(int32_t channel) {
        return channel < kNumDigitalChannels && channel >= 0;
    }

    void HAL_FreeDIOPort(HAL_DigitalHandle dioPortHandle) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        // no status, so no need to check for a proper free.
        digitalChannelHandles->Free(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) return;
//        SimDIOData[port->channel].SetInitialized(true);
    }

    /**
     * Allocate a DO PWM Generator.
     * Allocate PWM generators so that they are not accidentally reused.
     *
     * @return PWM Generator handle
     */
    HAL_DigitalPWMHandle HAL_AllocateDigitalPWM(int32_t* status) {
        auto handle = digitalPWMHandles->Allocate();
        if (handle == HAL_kInvalidHandle) {
            *status = NO_AVAILABLE_RESOURCES;
            return HAL_kInvalidHandle;
        }

        auto id = digitalPWMHandles->Get(handle);
        if (id == nullptr) {  // would only occur on thread issue.
            *status = HAL_HANDLE_ERROR;
            return HAL_kInvalidHandle;
        }
        *id = static_cast<uint8_t>(getHandleIndex(handle));

//        SimDigitalPWMData[*id].SetInitialized(true);

        return handle;
    }

    /**
     * Free the resource associated with a DO PWM generator.
     *
     * @param pwmGenerator The pwmGen to free that was allocated with
     * allocateDigitalPWM()
     */
    void HAL_FreeDigitalPWM(HAL_DigitalPWMHandle pwmGenerator, int32_t* status) {
        auto port = digitalPWMHandles->Get(pwmGenerator);
        digitalPWMHandles->Free(pwmGenerator);
        if (port == nullptr) return;
        int32_t id = *port;
//        SimDigitalPWMData[id].SetInitialized(false);
    }

    /**
     * Change the frequency of the DO PWM generator.
     *
     * The valid range is from 0.6 Hz to 19 kHz.  The frequency resolution is
     * logarithmic.
     *
     * @param rate The frequency to output all digital output PWM signals.
     */
    void HAL_SetDigitalPWMRate(double rate, int32_t* status) {
        // Currently rounding in the log rate domain... heavy weight toward picking a
        // higher freq.
        // TODO: Round in the linear rate domain.
        // uint8_t pwmPeriodPower = static_cast<uint8_t>(
        //    std::log(1.0 / (kExpectedLoopTiming * 0.25E-6 * rate)) /
        //        std::log(2.0) +
        //    0.5);
        // TODO(THAD) : Add a case to set this in the simulator
        // digitalSystem->writePWMPeriodPower(pwmPeriodPower, status);
    }

    /**
     * Configure the duty-cycle of the PWM generator
     *
     * @param pwmGenerator The generator index reserved by allocateDigitalPWM()
     * @param dutyCycle The percent duty cycle to output [0..1].
     */
    void HAL_SetDigitalPWMDutyCycle(HAL_DigitalPWMHandle pwmGenerator, double dutyCycle, int32_t* status) {
        auto port = digitalPWMHandles->Get(pwmGenerator);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }
        int32_t id = *port;
        if (dutyCycle > 1.0) dutyCycle = 1.0;
        if (dutyCycle < 0.0) dutyCycle = 0.0;
//        SimDigitalPWMData[id].SetDutyCycle(dutyCycle);
    }

    /**
     * Configure which DO channel the PWM signal is output on
     *
     * @param pwmGenerator The generator index reserved by allocateDigitalPWM()
     * @param channel The Digital Output channel to output on
     */
    void HAL_SetDigitalPWMOutputChannel(HAL_DigitalPWMHandle pwmGenerator, int32_t channel, int32_t* status) {
        auto port = digitalPWMHandles->Get(pwmGenerator);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }
        int32_t id = *port;
//        SimDigitalPWMData[id].SetPin(channel);
    }

    /**
     * Write a digital I/O bit to the FPGA.
     * Set a single value on a digital I/O channel.
     *
     * @param channel The Digital I/O channel
     * @param value The state to set the digital channel (if it is configured as an
     * output)
     */
    void HAL_SetDIO(HAL_DigitalHandle dioPortHandle, HAL_Bool value, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }
        if (value != 0 && value != 1) {
            if (value != 0) value = 1;
        }

//        VMXResourceHandle handle = HAL_DIOToVMXHandle(dioPortHandle);
//        vmxIO->DIO_Set(handle, (bool)value, vmxError);
    }

    /**
     * Set direction of a DIO channel.
     *
     * @param channel The Digital I/O channel
     * @param input true to set input, false for output
     */
    void HAL_SetDIODirection(HAL_DigitalHandle dioPortHandle, HAL_Bool input, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }

//        SimDIOData[port->channel].SetIsInput(input);
    }

    /**
     * Read a digital I/O bit from the FPGA.
     * Get a single value from a digital I/O channel.
     *
     * @param channel The digital I/O channel
     * @return The state of the specified channel
     */
    HAL_Bool HAL_GetDIO(HAL_DigitalHandle dioPortHandle, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return false;
        }

//        bool value;
//        VMXResourceHandle handle = HAL_DIOToVMXHandle(dioPortHandle);
//        return mau::vmxIO->DIO_Get(handle, value, vmxError);
        return true;
    }

    /**
     * Read the direction of a the Digital I/O lines
     * A 1 bit means output and a 0 bit means input.
     *
     * @param channel The digital I/O channel
     * @return The direction of the specified channel
     */
    HAL_Bool HAL_GetDIODirection(HAL_DigitalHandle dioPortHandle, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return false;
        }

//        vmxIO
//
//        HAL_Bool value = SimDIOData[port->channel].GetIsInput();
//        if (value > 1) value = 1;
//        if (value < 0) value = 0;
//        return value;
        // TODO: ALL DYLAN! ALL!!!!
        return 0;
    }

    /**
     * Generate a single pulse.
     * Write a pulse to the specified digital output channel. There can only be a
     * single pulse going at any time.
     *
     * @param channel The Digital Output channel that the pulse should be output on
     * @param pulseLength The active length of the pulse (in seconds)
     */
    void HAL_Pulse(HAL_DigitalHandle dioPortHandle, double pulseLength,
                   int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }
        // TODO (Thad) Add this
    }

    /**
     * Check a DIO line to see if it is currently generating a pulse.
     *
     * @return A pulse is in progress
     */
    HAL_Bool HAL_IsPulsing(HAL_DigitalHandle dioPortHandle, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return false;
        }
        return false;
        // TODO (Thad) Add this
    }

    /**
     * Check if any DIO line is currently generating a pulse.
     *
     * @return A pulse on some line is in progress
     */
    HAL_Bool HAL_IsAnyPulsing(int32_t* status) {
        return false;  // TODO(Thad) Figure this out
    }

    /**
     * Write the filter index from the FPGA.
     * Set the filter index used to filter out short pulses.
     *
     * @param dioPortHandle Handle to the digital I/O channel
     * @param filterIndex The filter index.  Must be in the range 0 - 3, where 0
     *                    means "none" and 1 - 3 means filter # filterIndex - 1.
     */
    void HAL_SetFilterSelect(HAL_DigitalHandle dioPortHandle, int32_t filterIndex, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return;
        }

        // TODO(Thad) Figure this out
    }

    /**
     * Read the filter index from the FPGA.
     * Get the filter index used to filter out short pulses.
     *
     * @param dioPortHandle Handle to the digital I/O channel
     * @return filterIndex The filter index.  Must be in the range 0 - 3,
     * where 0 means "none" and 1 - 3 means filter # filterIndex - 1.
     */
    int32_t HAL_GetFilterSelect(HAL_DigitalHandle dioPortHandle, int32_t* status) {
        auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
        if (port == nullptr) {
            *status = HAL_HANDLE_ERROR;
            return 0;
        }
        return 0;
        // TODO(Thad) Figure this out
    }

    /**
     * Set the filter period for the specified filter index.
     *
     * Set the filter period in FPGA cycles.  Even though there are 2 different
     * filter index domains (MXP vs HDR), ignore that distinction for now since it
     * compilicates the interface.  That can be changed later.
     *
     * @param filterIndex The filter index, 0 - 2.
     * @param value The number of cycles that the signal must not transition to be
     * counted as a transition.
     */
    void HAL_SetFilterPeriod(int32_t filterIndex, int64_t value, int32_t* status) {
        // TODO(Thad) figure this out
    }

    /**
     * Get the filter period for the specified filter index.
     *
     * Get the filter period in FPGA cycles.  Even though there are 2 different
     * filter index domains (MXP vs HDR), ignore that distinction for now since it
     * compilicates the interface.  Set status to NiFpga_Status_SoftwareFault if the
     * filter values miss-match.
     *
     * @param filterIndex The filter index, 0 - 2.
     * @param value The number of cycles that the signal must not transition to be
     * counted as a transition.
     */
    int64_t HAL_GetFilterPeriod(int32_t filterIndex, int32_t* status) {
        return 0;  // TODO(Thad) figure this out
    }
}
