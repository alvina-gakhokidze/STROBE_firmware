#pragma once
#include <bricks/Smoothing_handler.h>
#include <bricks/Derivative_handler.h>

namespace filterTasks
{

    /**
     * @brief this task is to fill the initial queue up, and then put it into state
     * where one sample is being taken out and new sample being put in and 
     * moving average being calculated constantly with O(1) complexity
    */
    void initFilter(void* filter)
    {
        // the filters are already initiated in the smoothing_handler file

        dataSmoother::weightedSumFilter* localFilter = (dataSmoother::weightedSumFilter*) filter;

        if(localFilter->setupTimer(localFilter->callbackFilterFilling) == false)
        {
            Serial.println("SOMETHING WENT WRONG INITIATING FILTER FILLING CALLBACK");
            while(1);
        }
        // the queue will start being incremented

        

        while(localFilter->q_size != WEIGHTED_SUM_FILTER_DEPTH); // we will just wait here until the queue fills up
        // NEED TO CALCULATE SUM OF THESE 5000 VARIABLES


        localFilter->endTimer();

        // we might lose one or two samples in between here, depending how fast it takes to end the timer and setup a new one

        if(localFilter->setupTimer(localFilter->callbackFilterMovingAverage) == false)
        {
            Serial.println("SOMETHING WENT WRONG INITIATING MOVING AVERAGE CALLBACK");
            while(1);
        }
        // now we set up another timer so that the moving average is calculated internally

    }

    /**
     * @brief performing integral/derivative/etc calculations for 
    */
    void finiteDifferenceDerivative(void* filter)
    {
        // maybe we have a semaphore here
        // and when the queue gives it, that means we have most recent average

        dataSmoother::weightedSumFilter* localFilter = (dataSmoother::weightedSumFilter*) filter;

        if( xSemaphoreTake( localFilter->q_semaphore, (TickType_t) 10 ) == pdTRUE )
        {
            // calculate derivative, put it into other queue
            
            float newDerivative = Filter::calcFDD(localFilter->newMovingBiteAverage, localFilter->oldMovingBiteAverage, (float) (WSF_PERIOD_MS/1000.0), 0); // don't need real time 2 and time1, just need time difference which is always 10ms
            
        }
    }

    void powerCalculation(void* filter)
    {
        
    }

    void frequencyCalculation(void* filter)
    {

    }
}



// perhaps we should attach dequeing to a timer, 

// or perhaps we can have on timer fill up the queue completely. Then once that's done. 

// we end that timer and we start a new one that enqueus and dequeues in ONE timer
// and updates the moving average itself.