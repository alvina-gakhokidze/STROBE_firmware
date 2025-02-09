#pragma once
#include <bricks/LED_handler.h>

namespace ledTasks{

    void updateFlyCountTask(void* ledBus)
    {
        strobeLED::LED* localLED = (strobeLED::LED*) ledBus;
        if(localLED->ledSemaphore != NULL)
        {
            if( xSemaphoreTake( localLED->ledSemaphore, (TickType_t) 10 ) == pdTRUE)
            {
                if(localLED == &strobeLED::redLED)
                {
                    redLEDFlyCount = localLED->ledFlyCount;
                }
                else if(localLED == &strobeLED::blueLED)
                {
                    blueLEDFlyCount = localLED->ledFlyCount;
                }
            }
            else{
                // do nothing, waiting for it to unblock
            }   
        } 

    }
}