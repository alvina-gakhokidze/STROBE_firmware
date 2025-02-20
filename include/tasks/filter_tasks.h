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

    /**
     * @brief this task is to fill the initial queue up, and then put it into state
     * where one sample is being taken out and new sample being put in and 
     * moving average being calculated constantly with O(1) complexity
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
        // the queue will start being incremented

        

        while(localFilter->q_size != MOVING_AVERAGE_FILTER_DEPTH); // we will just wait here until the queue fills up
        // NEED TO CALCULATE SUM OF THESE 5000 VARIABLES


        localFilter->endTimer();

        // we might lose one or two samples in between here, depending how fast it takes to end the timer and setup a new one

        if(localFilter->setupTimer(localFilter->callbackFilterMovingAverage) == false)
        {
            Serial.println("SOMETHING WENT WRONG INITIATING MOVING AVERAGE CALLBACK");
            while(1);
        }
        // now we set up another timer so that the moving average is calculated internally and throughout the program

    }

    void initDerivativeQueue(void* )
    {

    }

    /**
     * @brief calculate derivative and high pass filter ?
     * 
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
            if( xSemaphoreTake( localFilters->movingAverage->q_semaphore , (TickType_t) 10 ) == pdTRUE )
            {
               
                float newDerivative = Filter::calcFDD(maFilter->oldMovingBiteAverage, maFilter->newMovingBiteAverage, 0, (float) (MAF_PERIOD_MS/1000.0)); // don't need real time 2 and time1, just need time difference which is always 10ms
                hpFilter->decrementQueue(); // queue is full from beginning, full of 0s
                hpFilter->incrementQueue(newDerivative); // since we always remove 1 element first, there will always be one empty when we increment

                int localBack = hpFilter->q_in; //we're going to be going in reverse order for the filter (because latest sample is first in high pass filter)
                int localFront = hpFilter->q_out;
                int localSize = FILTER_TAP_NUM;

                double filtered_value = 0; // do we divide afterwards?? or do we just sum.

                double time1 = millis();

                // apply high pass filter weights here
                for(int i = 0; i < FILTER_TAP_NUM; i++)
                {
                    // we're reading / "removing" from the back (even though typically you INSERT at the back because that's where newest data goes)
                    localBack = ( localFront + localSize - 1 ) % FILTER_TAP_NUM; 
                    int dVal = *( hpFilter->q_handle + localBack );
                    filtered_value += filter_taps[i] * (double) dVal ;
                    localSize--;
                }

                // now here we need to pass this filtered value to the ESC tasks! we can use more semaphores
                // INSERT CODE TO PASS VALUE HERE



                double totalTime = millis() - time1; // tells us how long this loop took
            }
        }

        
        //vTaskDelay((TickType_t) pdMS_TO_TICKS(0.1) );
    }
    // i think in this 

    void powerCalculation(void* filterStruct)
    {
        filterParams* localFilters = (filterParams*) filterStruct;
        dataSmoother::movingAverageFilter* maFilter = localFilters->movingAverage;
        Filter::HighPass* hpFilter = localFilters->highPassFilter;

        // here we want to do our simulink calculations

        
        
    }

    void frequencyCalculation(void* filterStruct)
    {
        // have nothign to do here until we figure out frequency calculation
    }


}



// perhaps we should attach dequeing to a timer, 

// or perhaps we can have on timer fill up the queue completely. Then once that's done. 

// we end that timer and we start a new one that enqueus and dequeues in ONE timer
// and updates the moving average itself.



// so we need to load up the queue with derivatives, and then we need to perform 



// it's odd because we first wait for the queue to fill up entirely before we start calculating moving average...
// maybe we shouldn't and we should just allow it to happen immediately