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
            if(localLED->ledOn)
            {
                if(localLED->onSemaphore != NULL)
                {
                    if( xSemaphoreTake( localLED->onSemaphore, (TickType_t) 1 ) == pdTRUE)
                    {
                        registerTalk::ledControlOn(localLED->busptr, localLED->power);
                        digitalWrite(DEBUG_LED, HIGH);
                    }
                }
            }
            if(localLED->offSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->offSemaphore, (TickType_t) 1 ) == pdTRUE )
                {
                    registerTalk::ledOff(localLED->busptr);
                    digitalWrite(DEBUG_LED, LOW);
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
                if( xSemaphoreTake( localLED->onFlashSemaphore, (TickType_t) 5 ) == pdTRUE)
                {
                    // registerTalk::ledControlOn(localLED->busptr, localLED->power);
                    // localLED->ledFlyCount++; // increment total number of interactions
                    // xSemaphoreGive(localLED->ledSemaphore); //unblock task with this function
                    localLED->trigger();
                }
            }
            if(localLED->offFlashSemaphore != NULL)
            {
                if( xSemaphoreTake( localLED->offFlashSemaphore, (TickType_t) 10 ) == pdTRUE )
                {
                    //registerTalk::ledOff(localLED->busptr);
                    localLED->deTrigger();
                    //registerTalk::ledOff(localLED->busptr);
                    //xSemaphoreGive(localLED->offSemaphore);
                    
                // digitalWrite(DEBUG_LED, LOW);

                }
            }

           
            // if( localLED->ledOn)
            // {
            //     // registerTalk::ledControlOn(localLED->busptr, localLED->power);
            //     // localLED->ledFlyCount++; // increment total number of interactions
            //     // xSemaphoreGive(localLED->ledSemaphore); //unblock task with this function
            //     localLED->trigger();
            // }
            // else
            // {
            //     //registerTalk::ledOff(localLED->busptr);
            //     localLED->deTrigger();
            //     //registerTalk::ledOff(localLED->busptr);
            //     //xSemaphoreGive(localLED->offSemaphore);
                
            // // digitalWrite(DEBUG_LED, LOW);

            // }
        
            
        }
    }

    
    // the problem is that we can't trigger a timer inside of an ISR with the esp32 s3 for some reason



}