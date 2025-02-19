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


    class movingAverageFilter
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
            movingAverageFilter(const char* filterName); 
            int decrementQueue();
            void deleteFilter();
            void resetFilter();
            bool setupTimer(TimerCallbackFunction_t callbackFunction);
            bool endTimer();

    };

    

    void movingAverageFilter::deleteFilter()
    {
        vQueueDelete(this->q_handle);
    }

    void movingAverageFilter::resetFilter()
    {
        xQueueReset(this->q_handle);
    }

    movingAverageFilter::movingAverageFilter(const char* filterName)
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


        q_handle = xQueueCreate(MOVING_AVERAGE_FILTER_DEPTH, sizeof(queue_item));
        q_semaphore = xSemaphoreCreateBinary();
    }

    /**
     * @brief initiates timer so that Queues are updated at rate of 100Hz
    */
    bool movingAverageFilter::setupTimer(TimerCallbackFunction_t callbackFunction)
    {
        this->timer_handle = xTimerCreate(this->name, pdMS_TO_TICKS(MAF_PERIOD_MS), pdTRUE, nullptr, callbackFunction);
        if(timer_handle == NULL) return false;
        if(xTimerStart(this->timer_handle, (TickType_t) 10) == pdFAIL) return false;
        return true;
    }

    bool movingAverageFilter::endTimer()
    {
        xTimerStop(this->timer_handle, (TickType_t) 10);
        xTimerDelete(this->timer_handle, (TickType_t) 10);
    }

    int movingAverageFilter::decrementQueue()
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

    movingAverageFilter redLEDData("redFilter");
    movingAverageFilter blueLEDData("blueFilter");

   
    /**
     * @brief timer callback function to add flyCount value to redQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
    */
    int redQueueAdd()
    {
        if(xQueueSendToBack(redLEDData.q_handle, (void*) &redLEDFlyCount, 0) == pdTRUE){
            
            redLEDData.oldMovingBiteAverage = redLEDData.newMovingBiteAverage;
            redLEDData.newMovingBiteAverage = ( (unsigned long) redLEDData.oldMovingBiteAverage * redLEDData.q_size + (unsigned long) redLEDFlyCount) / ( (unsigned long) redLEDData.q_size + 1.0 );
            // don't need to subtract anything because nums being kicked out are assumed to be 0s (empty queue)
            redLEDData.q_size++;
            xSemaphoreGive(redLEDData.q_semaphore);
        }
    }

    /**
     * @brief timer callback function to add flyCount value to blueQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
    */
    int blueQueueAdd()
    {
        if(xQueueSendToBack(blueLEDData.q_handle, (void*) &blueLEDFlyCount, 0) == pdTRUE)
        {
            blueLEDData.oldMovingBiteAverage = blueLEDData.newMovingBiteAverage;
            blueLEDData.newMovingBiteAverage = ( (unsigned long) blueLEDData.oldMovingBiteAverage * blueLEDData.q_size + (unsigned long) blueLEDFlyCount) / ( (unsigned long) blueLEDData.q_size + 1.0);
            blueLEDData.q_size++;
            xSemaphoreGive(blueLEDData.q_semaphore);
        }
    }

    /**
     * @brief timer callback function for redLED that will be used once the initial moving average queue has been filled up properly
     */
    void redQueueFiltering()
    {
        int oldNum = redLEDData.decrementQueue(); // make space first
        redQueueAdd(); // add new value
        int newNum = redLEDFlyCount; // might be a race condition ... 
        redLEDData.oldMovingBiteAverage = redLEDData.newMovingBiteAverage; // store old value (for FDD purposes)
        redLEDData.newMovingBiteAverage = ( redLEDData.oldMovingBiteAverage * (unsigned long) MOVING_AVERAGE_FILTER_DEPTH - oldNum + newNum ) / ( (unsigned long) MOVING_AVERAGE_FILTER_DEPTH ) ;
        // this is called the recrusive implementation of the moving average filter!
        xSemaphoreGive(redLEDData.q_semaphore);
    }
    

    /**
    * @brief timer callback function for blueLED that will be used once the initial moving average queue has been filled up properly
    */
    void blueQueueFiltering()
    {
        int oldNum = blueLEDData.decrementQueue(); // make space in queue first
        blueQueueAdd(); // add new value
        int newNum = blueLEDFlyCount; // might be a race condition
        blueLEDData.oldMovingBiteAverage = blueLEDData.newMovingBiteAverage; // store old value (for FDD purposes)
        blueLEDData.newMovingBiteAverage = ( blueLEDData.oldMovingBiteAverage * (unsigned long) MOVING_AVERAGE_FILTER_DEPTH - oldNum + newNum ) / ( (unsigned long) MOVING_AVERAGE_FILTER_DEPTH ) ;
        xSemaphoreGive(blueLEDData.q_semaphore);
    }




    // so we have software timers for updating the sampling queue
    // we also need to take the derivatives of these new values
    // and then we need to put the derivatives through a high pass filter




    

}