#pragma once
#include <bricks/LED_handler.h>
#include <configs/FILTER_config.h>




/**
 * In this file, we want to develop all the functions that interpret data from the interupts, and create queues
*/

/**
 * We probably also want to have each queue in a class.
 * Also, I said that we need four quues but I lied. We actually only need one queue per LED (aka two)
*/


/**
 * i want to limit how many things happen in the interrupt that turns the leds on/off
 * i could have a different timer here that triggers and interrupt to check if the ledFlyCount has changd
 * but then that means that i'm using too much processing power constantly checking it... 
 * which im sure would interrupt the interrupts from happening
 * so it's probably better to actually write to the queue inside the interrupt itself.............
 * actually if we're manipulating a queue, it's bad practice to alter it inside of an interrupt because we have something ELSE clearing it....alignas
 * 
 * so maybe instead the task will have to constantly run and check when flyCount has changed... 
 * 
 * the flies actually only touch the esp32 every 0-3 seconds. 
 * 
 * so we could have the task running as usual, 
 * 
*/

/**
 * Include all P/I/D calculations
*/



namespace dataSmoother
{
    void redQueueAdd(TimerHandle_t xhandle);
    void blueQueueAdd(TimerHandle_t xhandle);
    void redQueueFiltering(TimerHandle_t xhandle);
    void blueQueueFiltering(TimerHandle_t xhandle);


    class weightedSumFilter
    {
        private:
            const char* name; 
            typedef int queue_item; // defining what type we will store in the queue
            TimerHandle_t timer_handle; 
            

        public:
            int q_size = 0;
            unsigned long oldMovingBiteAverage = 0;
            unsigned long newMovingBiteAverage = 0;
            TimerCallbackFunction_t callbackFilterFilling;
            TimerCallbackFunction_t callbackFilterMovingAverage; 
            QueueHandle_t q_handle;
            SemaphoreHandle_t q_semaphore; 
            weightedSumFilter(const char* filterName); 
            int decrementQueue();
            void deleteFilter();
            void resetFilter();
            bool setupTimer(TimerCallbackFunction_t callbackFunction);
            bool endTimer();

    };

    

    void weightedSumFilter::deleteFilter()
    {
        vQueueDelete(this->q_handle);
    }

    void weightedSumFilter::resetFilter()
    {
        xQueueReset(this->q_handle);
    }

    weightedSumFilter::weightedSumFilter(const char* filterName)
    {
        name = filterName;
        
        if(filterName == "redFilter")
        {
            callbackFilterFilling = redQueueAdd;
            callbackFilterMovingAverage = redQueueFiltering;
        }
        else if (filterName == "blueFilter")
        {
            callbackFilterFilling = blueQueueAdd;
            callbackFilterMovingAverage = blueQueueFiltering;
        }
        else
        {
            Serial.println("SUCH A FILTER DOESN'T EXIST");
            while(1);
        }


        q_handle = xQueueCreate(WEIGHTED_SUM_FILTER_DEPTH, sizeof(queue_item));
        q_semaphore = xSemaphoreCreateBinary();
    }

    /**
     * @brief initiates timer so that Queues are updated at rate of 100Hz
    */
    bool weightedSumFilter::setupTimer(TimerCallbackFunction_t callbackFunction)
    {
        this->timer_handle = xTimerCreate(this->name, pdMS_TO_TICKS(WSF_PERIOD_MS), pdTRUE, nullptr, callbackFunction);
        if(timer_handle == NULL) return false;
        if(xTimerStart(this->timer_handle, (TickType_t) 10) == pdFAIL) return false;
        return true;
    }

    bool weightedSumFilter::endTimer()
    {
        xTimerStop(this->timer_handle, (TickType_t) 10);
        xTimerDelete(this->timer_handle, (TickType_t) 10);
    }

    int weightedSumFilter::decrementQueue()
    {
        
        int* countVal;
        if(xQueuePeek(this->q_handle, countVal, (TickType_t) 10 ) == pdTRUE){
            // means the Queue is not empty
            xQueueReceive(this->q_handle, countVal, (TickType_t) 10); // pop an item from the queue
            this->q_size--; // decrement size  
            return *countVal;
        }

        return -1; // we never expect to have a negative number, so this indicates something is wrong
    }

    weightedSumFilter redLEDFilter("redFilter");
    weightedSumFilter blueLEDFilter("blueFilter");

   
    /**
     * @brief timer callback function to add flyCount value to redQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
    */
    int redQueueAdd()
    {
        if(xQueueSendToBack(redLEDFilter.q_handle, (void*) &redLEDFlyCount, 0) == pdTRUE){
            redLEDFilter.q_size++;
        }
    }

    /**
     * @brief timer callback function to add flyCount value to blueQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
    */
    int blueQueueAdd()
    {
        if(xQueueSendToBack(blueLEDFilter.q_handle, (void*) &blueLEDFlyCount, 0) == pdTRUE)
        {
            blueLEDFilter.q_size++;
        }
    }

    void redQueueFiltering()
    {
        int oldNum = redLEDFilter.decrementQueue(); // make space first
        redQueueAdd(); // add new value
        int newNum = redLEDFlyCount; // might be a race condition ... 
        redLEDFilter.oldMovingBiteAverage = redLEDFilter.newMovingBiteAverage; // store old value (for FDD purposes)
        redLEDFilter.newMovingBiteAverage = ( redLEDFilter.newMovingBiteAverage * WEIGHTED_SUM_FILTER_DEPTH - oldNum + newNum ) / WEIGHTED_SUM_FILTER_DEPTH ;
        // this is called the recrusive implementation of the moving average filter!
        xSemaphoreGive(redLEDFilter.q_semaphore);
    }
    

    void blueQueueFiltering()
    {
        int oldNum = blueLEDFilter.decrementQueue(); // make space in queue first
        blueQueueAdd(); // add new value
        int newNum = blueLEDFlyCount; // might be a race condition
        blueLEDFilter.oldMovingBiteAverage = blueLEDFilter.newMovingBiteAverage; // store old value (for FDD purposes)
        blueLEDFilter.newMovingBiteAverage = ( blueLEDFilter.newMovingBiteAverage * WEIGHTED_SUM_FILTER_DEPTH - oldNum + newNum ) / WEIGHTED_SUM_FILTER_DEPTH ;
        xSemaphoreGive(blueLEDFilter.q_semaphore);
    }




    // so we have software timers for updating the sampling queue
    // we also need to take the derivatives of these new values
    // and then we need to put the derivatives through a high pass filter




    

}