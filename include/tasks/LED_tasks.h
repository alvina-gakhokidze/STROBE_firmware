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

    /**
     * @brief responsible for turning the LEDs on and off
     */
    void toggleLED(void* ledBus)
    {
        strobeLED::LED* localLED = (strobeLED::LED*) ledBus;

        for(;;)
        {

            if(localLED->onSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->onSemaphore, (TickType_t) 0 ) == pdTRUE)
                {

                    digitalWrite(DEBUG_LED_BLUE, HIGH);
                    
                    registerTalk::ledControlOn(localLED->busptr, localLED->power);
                    localLED->state = false;
                    
                }
            }
            if(localLED->offSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->offSemaphore, (TickType_t) 1 ) == pdTRUE ) // this has to be set to 1 for delay otherwise LED won't turn on
                {
                    
                    digitalWrite(DEBUG_LED_BLUE, LOW);
                    
                    registerTalk::ledOff(localLED->busptr);
                    localLED->state = true;
                    
                }
            }
                 
        }
    }

    /**
     * @brief responsible for triggering timers that will send semaphores to toggleLED to make LEDs flash at a specific frequency (as opposed to just on and off)
     */
    void flashLED(void* ledBus)
    {
        strobeLED::LED* localLED = (strobeLED::LED*) ledBus;

        for(;;)
        {
            if(localLED->onFlashSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->onFlashSemaphore, (TickType_t) 0 ) == pdTRUE)
                {
        
                    localLED->trigger();
                    
                }
            }
            if(localLED->offFlashSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->offFlashSemaphore, (TickType_t) 0 ) == pdTRUE )
                {
                    
                    localLED->deTrigger();

                }
            }
        
        }
    }

}