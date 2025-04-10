#pragma once
#include <bricks/LED_handler.h>
#include <configs/FILTER_config.h>
#include <configs/Task_config.h>




/**
 * This file is for the parameter optimization mode. Here is where we develop the moving average filter to smooth the data before it's processed further and fed to the controllers
 * The values are stored in a queue, and the moving average filter is applied with O(1) complexity once the queue is full. 
*/


namespace dataSmoother
{
    void redQueueFill(TimerHandle_t xhandle);
    void blueQueueFill(TimerHandle_t xhandle);
    void redQueueFiltering(TimerHandle_t xhandle);
    void blueQueueFiltering(TimerHandle_t xhandle);

    portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

    class movingAverageFilter
    {
        private:
            const char* name; 
            typedef int queue_item; // defining what type we will store in the queue
            TimerHandle_t timer_handle; 
            

        public:
            int q_size = 0;
            volatile float oldMovingBiteAverage = 0;
            volatile float newMovingBiteAverage = 0;
            TaskHandle_t taskHandle;
            TimerCallbackFunction_t callbackFilterFilling;
            TimerCallbackFunction_t callbackFilterMovingAverage; 
            QueueHandle_t q_handle;
            SemaphoreHandle_t q_semaphore; 
            movingAverageFilter(const char* filterName); 
            void deleteFilter();
            void resetFilter();
            bool setupTimer(TimerCallbackFunction_t callbackFunction);
            void endTimer();

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
            callbackFilterFilling = redQueueFill;
            callbackFilterMovingAverage = redQueueFiltering;
            taskHandle = TaskHandlers::redInitFilter;
        }
        else if (filterName == "blueFilter")
        {
            callbackFilterFilling = blueQueueFill;
            callbackFilterMovingAverage = blueQueueFiltering;
            taskHandle = TaskHandlers::blueInitFilter;
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
        if( xTimerStart(this->timer_handle, (TickType_t) 10) == pdFAIL ) return false;
        return true;
    }

    void movingAverageFilter::endTimer()
    {
        xTimerStop(this->timer_handle, (TickType_t) 10);
        xTimerDelete(this->timer_handle, (TickType_t) 10);
    }

    movingAverageFilter redLEDData("redFilter");
    movingAverageFilter blueLEDData("blueFilter");

   
    /**
     * @brief timer callback function to add flyCount value to redQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
     * This is meant for filling up the queue initially
    */
    void redQueueFill(TimerHandle_t xhandle) //Potential problem: since we're then using this as a regular function there may be problems
    {
        if(xQueueSendToBack(redLEDData.q_handle, (void*) &strobeLED::redLEDFlyCount, 0) == pdTRUE){
            
            redLEDData.oldMovingBiteAverage = redLEDData.newMovingBiteAverage;
            redLEDData.newMovingBiteAverage = ( (float) redLEDData.oldMovingBiteAverage * redLEDData.q_size + (float) strobeLED::redLEDFlyCount) / ( (float) redLEDData.q_size + 1.0 );
            // don't need to subtract anything because nums being kicked out are assumed to be 0s (empty queue)
            redLEDData.q_size++;
            xSemaphoreGive(redLEDData.q_semaphore);
        }
    }

    /**
     * @brief timer callback function to add flyCount value to blueQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
     * This is meant for filling up the queue initially. 
    */
    void blueQueueFill(TimerHandle_t xhandle) // potential problem: since we're then using this as a regular function there may be problems
    {
        if(xQueueSendToBack(blueLEDData.q_handle, (void*) &strobeLED::blueLEDFlyCount, 0) == pdTRUE)
        {
            blueLEDData.oldMovingBiteAverage = blueLEDData.newMovingBiteAverage;
            blueLEDData.newMovingBiteAverage = ( (float) blueLEDData.oldMovingBiteAverage * blueLEDData.q_size + (float) strobeLED::blueLEDFlyCount) / ( (float) blueLEDData.q_size + 1.0);
            blueLEDData.q_size++;
            xSemaphoreGive(blueLEDData.q_semaphore);
        }
    }

    /**
     * @brief timer callback function for redLED that will be used once the initial moving average queue has been filled up properly
     */
    void redQueueFiltering(TimerHandle_t xhandle)
    {
        int oldNum;
        xQueueReceive(redLEDData.q_handle, &oldNum, (TickType_t) 0);
        int newNum = strobeLED::redLEDFlyCount; // might be a race condition ... 
        xQueueSendToBack(redLEDData.q_handle, (void*) &newNum, (TickType_t) 0);
        redLEDData.oldMovingBiteAverage = redLEDData.newMovingBiteAverage; // store old value (for FDD purposes)
        redLEDData.newMovingBiteAverage = ( redLEDData.oldMovingBiteAverage * (float) MOVING_AVERAGE_FILTER_DEPTH - oldNum + newNum ) / ( (float) MOVING_AVERAGE_FILTER_DEPTH ) ;
        xSemaphoreGive(redLEDData.q_semaphore);
    }
    

    /**
    * @brief timer callback function for blueLED that will be used once the initial moving average queue has been filled up properly
    */
    void blueQueueFiltering(TimerHandle_t xhandle)
    {
        int oldNum; 
        xQueueReceive(blueLEDData.q_handle, &oldNum, (TickType_t) 0);
        int newNum = strobeLED::blueLEDFlyCount; // might be a race condition
        xQueueSendToBack(blueLEDData.q_handle, (void*) &newNum, (TickType_t) 0);
        blueLEDData.oldMovingBiteAverage = blueLEDData.newMovingBiteAverage; // store old value (for FDD purposes)
        blueLEDData.newMovingBiteAverage = ( blueLEDData.oldMovingBiteAverage * (float) MOVING_AVERAGE_FILTER_DEPTH - oldNum + newNum ) / ( (float) MOVING_AVERAGE_FILTER_DEPTH ) ;
        xSemaphoreGive(blueLEDData.q_semaphore);
    }

}