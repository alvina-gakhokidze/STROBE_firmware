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

    void toggleLED(void* ledBus)
    {
        strobeLED::LED* localLED = (strobeLED::LED*) ledBus;

        for(;;)
        {
            // first we TAKE semaphore
            
            if(localLED->onSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->onSemaphore, (TickType_t) 0 ) == pdTRUE)
                {
                    digitalWrite(DEBUG_LED, HIGH);
                    
                    registerTalk::ledControlOn(localLED->busptr, localLED->power);
                    localLED->state = false;
                    
                }
            }
            if(localLED->offSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->offSemaphore, (TickType_t) 1 ) == pdTRUE ) // this has to be set to 1 for delay otherwise LED won't turn on
                {
                    digitalWrite(DEBUG_LED, LOW);
                    
                    registerTalk::ledOff(localLED->busptr);
                    localLED->state = true;
                    
                }
            }
                 
        }
    }

    void flashLED(void* ledBus)
    {
        strobeLED::LED* localLED = (strobeLED::LED*) ledBus;

        for(;;)
        {
            // first we TAKE semaphore
            if(localLED->onFlashSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->onFlashSemaphore, (TickType_t) 0 ) == pdTRUE)
                {
                    
                    //localLED->state = true;
                    localLED->trigger();
                    
                    //digitalWrite(DEBUG_LED, HIGH);
                    
                }
            }
            if(localLED->offFlashSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->offFlashSemaphore, (TickType_t) 0 ) == pdTRUE )
                {
                    
                    localLED->deTrigger();
                    //digitalWrite(DEBUG_LED, LOW);
                    //registerTalk::ledOff(localLED->busptr);

                }
            }
        
        }
    }

    
    // the problem is that we can't trigger a timer inside of an ISR with the esp32 s3 for some reason



}