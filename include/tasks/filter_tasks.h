#pragma once
#include <bricks/Smoothing_handler.h>
#include <bricks/Derivative_handler.h>


namespace filterTasks
{

    int fCounter = 0;

    typedef struct filterParams
    {
        dataSmoother::movingAverageFilter* movingAverage;
        Filter::HighPass* highPassFilter;
    } filterParams;

    filterParams redLEDFilter = 
    {
        &dataSmoother::redLEDData,
        &Filter::redLEDPowerFilter
    };

    filterParams blueLEDFilter = 
    {
        &dataSmoother::blueLEDData,
        &Filter::blueLEDPowerFilter
    };

    /**
     * @brief this task is to fill the initial queue up, and then put it into state
     * where one sample is being taken out and new sample being put in and 
     * moving average being calculated constantly with O(1) complexity
     * @param filter xLEDData object from movingAverageFilter in dataSmoother
    */
    void initFilter(void* filter)
    {
        // the filters are already initiated in the smoothing_handler file

        Serial.printf("starting filter\n");

        dataSmoother::movingAverageFilter* localFilter = (dataSmoother::movingAverageFilter*) filter;

        if(localFilter->setupTimer(localFilter->callbackFilterFilling) == false)
        {
            Serial.println("SOMETHING WENT WRONG INITIATING FILTER FILLING CALLBACK");
            while(1);
        }
        // the queue will start being incremented 
        
        
        
        //xTaskDelayUntil();
        for(;;)
        {
            vTaskDelay((TickType_t) 200);
            //int placemat;
            //xQueuePeek(localFilter->q_handle, &placemat, (TickType_t) 5);
            Serial.printf("%d\n", localFilter->q_size);
            //Serial.printf("moving bite average: %lf\n", localFilter->newMovingBiteAverage);
            
            if(localFilter->q_size >= MOVING_AVERAGE_FILTER_DEPTH)
            {
                localFilter->endTimer();

                Serial.printf("ending timer\n");
        
                // we might lose one or two samples in between here, depending how fast it takes to end the timer and setup a new one
        
                if(localFilter->setupTimer(localFilter->callbackFilterMovingAverage) == false)
                {
                    Serial.println("SOMETHING WENT WRONG INITIATING MOVING AVERAGE CALLBACK");
                    while(1);
                }
                // now we set up another timer so that the moving average is calculated internally and throughout the program
                Serial.printf("Task Deleted\n");
                vTaskDelete(localFilter->taskHandle);
            }   
            
        }

        // while(localFilter->q_size != MOVING_AVERAGE_FILTER_DEPTH)
        // {
        //     //Serial.printf("Queue Size: %d\n", localFilter->q_size);
        //     int placemat;
        //     xQueuePeek(localFilter->q_handle, &placemat, (TickType_t) 5);
        //     Serial.printf("%d\n", placemat);
        // } // we will just wait here until the queue fills up
        // // NEED TO CALCULATE SUM OF THESE 5000 VARIABLES


        // localFilter->endTimer();

        // Serial.printf("ending timer\n");

        // // we might lose one or two samples in between here, depending how fast it takes to end the timer and setup a new one

        // if(localFilter->setupTimer(localFilter->callbackFilterMovingAverage) == false)
        // {
        //     Serial.println("SOMETHING WENT WRONG INITIATING MOVING AVERAGE CALLBACK");
        //     while(1);
        // }
        // // now we set up another timer so that the moving average is calculated internally and throughout the program
        // vTaskDelete(localFilter->taskHandle);
    }

    /**
     * @brief calculate derivative
     * @param passing filterParams struct which stores xLEDData in movingAverageFilter and xLEDFilter in highPassFilter
    */
    void finiteDifferenceDerivative(void* filterStruct)
    {
        // maybe we have a semaphore here
        // and when the queue gives it, that means we have most recent average

        filterParams* localFilters = (filterParams*) filterStruct;
        dataSmoother::movingAverageFilter* maFilter = localFilters->movingAverage;
        Filter::HighPass* hpFilter = localFilters->highPassFilter;

        Serial.printf("entering derivative task\n");

        for(;;)
        { 
            if( localFilters->movingAverage->q_semaphore != NULL)
            {
                //semaphore should be triggered at rate of 100Hz, everytime new value is put into queue
                if( xSemaphoreTake( localFilters->movingAverage->q_semaphore , (TickType_t) 5 ) == pdTRUE )
                {
                    if(fCounter == 75)
                    {
                        fCounter = 0;
                    }
                    else{
                        fCounter++;
                    }
                    float newDerivative = Filter::calcFDD(maFilter->oldMovingBiteAverage, maFilter->newMovingBiteAverage, 0, (float) (MAF_PERIOD_MS/1000.0)); // don't need real time 2 and time1, just need time difference which is always 10ms
                    
                    float oldDev = hpFilter->decrementQueue(); // queue is full from beginning, full of 0s
                    bool added = hpFilter->incrementQueue(newDerivative); // since we always remove 1 element first, there will always be one empty when we increment
                    Serial.printf("%d. old: %lf new: %lf new fdd: %lf old fdd: %lf, added succesfully: %d\n", fCounter, maFilter->oldMovingBiteAverage, maFilter->newMovingBiteAverage, newDerivative, oldDev, added);
                    xSemaphoreGive(hpFilter->highpassSemaphore);
                }
            }
            //printf("stack size of fdd: %d\n", uxTaskGetStackHighWaterMark(NULL));
        }

        
        //vTaskDelay((TickType_t) pdMS_TO_TICKS(0.1) );
    }

    // this is actually a high pass filter that is dependent on the loop... so to generalize this task, 
    // we should actually not hard code the hard pass filter

    void highPassFilterTask(void* filter)
    {
        Filter::HighPass* hpFilter = (Filter::HighPass*) filter;
        Serial.printf("entering high pass task\n");

        for(;;) // infinite looping task
        {
            if(hpFilter->highpassSemaphore != NULL)
            {
                if(xSemaphoreTake(hpFilter->highpassSemaphore, (TickType_t) 5) == pdTRUE)
                {
                    //Serial.printf("High Pass Semaphore given\n");
                    int numFilterCoeffs = hpFilter->filterSize;
                    double * coeffArray = hpFilter->filterCoeffs;

                    int localBack = hpFilter->q_in; //we're going to be going in reverse order for the filter (because latest sample is first in high pass filter)
                    int localFront = hpFilter->q_out;
                    int localSize = numFilterCoeffs;
                    //Serial.printf("back: %d front: %d\n", localBack, localFront);

                    double filtValue = 0; // we just sum.

                    unsigned long time1 = millis();

                    // apply high pass filter weights here
                    for(int i = 0; i < numFilterCoeffs; i++)
                    {
                        // we're reading / "removing" from the back (even though typically you INSERT at the back because that's where newest data goes)
                        localBack = ( localFront + localSize - 1 ) % numFilterCoeffs; 
                        float dVal = *( hpFilter->q_handle + localBack );
                        filtValue += coeffArray[i] * (double) dVal ;
                        localSize--;
                    }

                    hpFilter->filteredValue = filtValue; 
                    //Serial.printf("filtered value: %lf\n", filtValue);

                    unsigned long totalTime = millis() - time1; // tells us how long this loop took

                    //Serial.printf("total time taken: %lu\n", totalTime);

                    // the thing is if we give a semaphore here,
                    // we have 4 ESC tasks, but only 2 filters. so we would need a counting semaphore...
                    // and in that case what if one ESC somehow completed faster htan the other? how would we limit it's semaphore usage.

                    // so do we only 
                }
            }
            
            //printf("stack size of high pass: %d\n", uxTaskGetStackHighWaterMark(NULL));
            vTaskDelay((TickType_t) 5); // would adding task delay fix the guru error?
        }

    }

    void printingTask(void * filter)
    {
        dataSmoother::movingAverageFilter* localFilter = (dataSmoother::movingAverageFilter*) filter;
        float new_moving_average = 0;

        for(;;)
        {
            if(localFilter->newMovingBiteAverage != new_moving_average)
            {
                Serial.printf("new moving average: %lf\n", localFilter->newMovingBiteAverage);
                new_moving_average = localFilter->newMovingBiteAverage;
            }
            
        }
        vTaskDelay((TickType_t) 10);
    }

    void flashingTask(void* nothing)
    {
        for(;;)
        {
            
            //Serial.print("Entering flashing task\n");
            //digitalWrite(DEBUG_LED2, HIGH);
            vTaskDelay(pdMS_TO_TICKS(10));
            digitalWrite(DEBUG_TRIGGER, HIGH);
            vTaskDelay(pdMS_TO_TICKS(5));
            digitalWrite(DEBUG_TRIGGER, LOW);
            //digitalWrite(DEBUG_LED2, LOW);
            vTaskDelay(pdMS_TO_TICKS(20));
            // Serial.printf("Red Count raw: %d", strobeLED::redLED.ledFlyCount);
            // Serial.printf(" Blue Count raw: %d\n", strobeLED::blueLED.ledFlyCount);
            //Serial.printf("Red Count: %d\n", strobeLED::redLEDFlyCount);
            
            //Serial.printf("queue size: %d\n", dataSmoother::redLEDData.q_size);
            // Serial.printf(" Blue Count: %d\n", strobeLED::blueLEDFlyCount);
            
            //Serial.printf("red: %d , %lf\n", strobeLED::redLEDFlyCount, dataSmoother::redLEDData.newMovingBiteAverage);
            //Serial.printf("blue: %d, %lf\n", strobeLED::blueLEDFlyCount, dataSmoother::blueLEDData.newMovingBiteAverage);
            
        }
    }

}
