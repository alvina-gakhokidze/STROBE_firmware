#pragma once
#include <bricks/LED_handler.h>

namespace ledTasks
{

    /**
     * @brief creates secondary storage variable we can use to story ledFlyCount... not entirely sure this task is necessary tbh
     */
    void updateFlyCountTask(void* ledBus)
    {
        strobeLED::LED* localLED = (strobeLED::LED*) ledBus;

        for(;;)
        {
            if(localLED->ledSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->ledSemaphore, (TickType_t) 5 ) == pdTRUE)
                {
                    if(localLED == &strobeLED::redLED)
                    {
                        strobeLED::redLEDFlyCount = localLED->ledFlyCount;
                    }
                    else if(localLED == &strobeLED::blueLED)
                    {
                        strobeLED::blueLEDFlyCount = localLED->ledFlyCount;
                    }
                } 
            } 

        }

    }
}