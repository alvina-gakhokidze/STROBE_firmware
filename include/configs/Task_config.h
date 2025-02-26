#pragma once
#include <configs/SYSTEM_config.h>

namespace TaskHandlers
{
    TaskHandle_t
        // List of all the tasks we could be running..
        // .. (use these handles to control tasks later)
        getRedLEDFlyCount = NULL,
        getBlueLEDFlyCount = NULL,
        redFDDFilter = NULL,
        blueFDDFilter = NULL,
        redLEDPowerHighPass = NULL,
        blueLEDPowerHighPass = NULL,
        redLEDFrequencyHighPass = NULL,
        blueLEDFrequencyHighPass = NULL,
        redLEDPowerESC = NULL,
        redLEDFrequencyESC = NULL,
        blueLEDPowerESC = NULL,
        blueLEDFrequencyESC = NULL;
}