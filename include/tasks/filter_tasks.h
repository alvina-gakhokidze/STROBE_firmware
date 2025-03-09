#pragma once
#include <bricks/Smoothing_handler.h>
#include <bricks/Derivative_handler.h>


namespace filterTasks
{

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

        dataSmoother::movingAverageFilter* localFilter = (dataSmoother::movingAverageFilter*) filter;

        if(localFilter->setupTimer(localFilter->callbackFilterFilling) == false)
        {
            Serial.println("SOMETHING WENT WRONG INITIATING FILTER FILLING CALLBACK");
            while(1);
        }
       
        for(;;)
        {
            //vTaskDelay((TickType_t) 200); OOLVOONOO DO I NEED THIS??
            
            if(localFilter->q_size >= MOVING_AVERAGE_FILTER_DEPTH)
            {
                localFilter->endTimer();

                // we might lose one or two samples in between here, depending how fast it takes to end the timer and setup a new one
        
                if(localFilter->setupTimer(localFilter->callbackFilterMovingAverage) == false)
                {
                    Serial.println("SOMETHING WENT WRONG INITIATING MOVING AVERAGE CALLBACK");
                    while(1);
                }
                // now we set up another timer so that the moving average is calculated internally and throughout the program
                vTaskDelete(localFilter->taskHandle);
            }   
            
        }

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


        for(;;)
        { 
            if( localFilters->movingAverage->q_semaphore != NULL)
            {
                //semaphore should be triggered at rate of 100Hz, everytime new value is put into queue
                if( xSemaphoreTake( localFilters->movingAverage->q_semaphore , (TickType_t) 5 ) == pdTRUE )
                {
                    float newDerivative = Filter::calcFDD(maFilter->oldMovingBiteAverage, maFilter->newMovingBiteAverage, 0, (float) (MAF_PERIOD_MS/1000.0)); // don't need real time 2 and time1, just need time difference which is always 10ms
                    
                    hpFilter->decrementQueue(); // queue is full from beginning, full of 0s
                    hpFilter->incrementQueue(newDerivative); // since we always remove 1 element first, there will always be one empty when we increment
                    //Serial.printf("%d. old: %lf new: %lf new fdd: %lf old fdd: %lf, added succesfully: %d\n", fCounter, maFilter->oldMovingBiteAverage, maFilter->newMovingBiteAverage, newDerivative, oldDev, added);
                    xSemaphoreGive(hpFilter->highpassSemaphore);
                }
            }
            //printf("stack size of fdd: %d\n", uxTaskGetStackHighWaterMark(NULL));
        }
    }

    // this is actually a high pass filter that is dependent on the loop... so to generalize this task, 
    // we should actually not hard code the hard pass filter

    void highPassFilterTask(void* filter)
    {
        Filter::HighPass* hpFilter = (Filter::HighPass*) filter;
    
        for(;;) // infinite looping task
        {
            if(hpFilter->highpassSemaphore != NULL)
            {
                if(xSemaphoreTake(hpFilter->highpassSemaphore, (TickType_t) 5) == pdTRUE)
                {
                    int numFilterCoeffs = hpFilter->filterSize;
                    double * coeffArray = hpFilter->filterCoeffs;

                    int localBack = hpFilter->q_in; //we're going to be going in reverse order for the filter (because latest sample is first in high pass filter)
                    int localFront = hpFilter->q_out;
                    int localSize = numFilterCoeffs;

                    double filtValue = 0; // we just sum.

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

}
