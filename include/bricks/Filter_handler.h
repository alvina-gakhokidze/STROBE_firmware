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



namespace controller
{
    void redQueueAdd(TimerHandle_t xhandle);
    void blueQueueAdd(TimerHandle_t xhandle);


    class weightedSumFilter
    {
        private:
            const char* name; 
            typedef int queue_item; // defining what type we will store in the queue
            TimerHandle_t timer_handle; 
            

        public:
            unsigned long q_size = 0;
            TimerCallbackFunction_t callbackFunction; 
            QueueHandle_t q_handle;
            weightedSumFilter(const char* filterName); 
            long decrementQueue();
            void deleteFilter();
            bool resetFilter();
            bool setupTimer();

    };

    void weightedSumFilter::deleteFilter()
    {
        vQueueDelete(this->q_handle);
    }

    bool weightedSumFilter::resetFilter()
    {
        xQueueReset(this->q_handle);
    }

    weightedSumFilter::weightedSumFilter(const char* filterName)
    {
        this->name = filterName;
        
        if(name == "redFilter")
        {
            this->callbackFunction = redQueueAdd;
        }
        else if (name == "blueFilter")
        {
            this->callbackFunction = blueQueueAdd;
        }
        else
        {
            Serial.println("WRONG NAME");
            while(1);
        }

        this->q_handle = xQueueCreate(WEIGHTED_SUM_FILTER_DEPTH, sizeof(queue_item));
    }

     bool weightedSumFilter::setupTimer()
    {
        this->timer_handle = xTimerCreate(this->name, pdMS_TO_TICKS(WSF_PERIOD_MS), pdTRUE, nullptr, this->callbackFunction);
        if(timer_handle == NULL) return false;
        return true;
    }

    long weightedSumFilter::decrementQueue()
    {
        
        unsigned long* countVal;
        if(xQueuePeek(this->q_handle, countVal, (TickType_t) 10 ) == pdTRUE){
            // means the Queue is not empty
            xQueueReceive(this->q_handle, countVal, (TickType_t) 10); // pop an item from the queue
            this->q_size--; // decrement size  
            return *countVal;
        }

        return -1; // we never expect to have a negative number, so this indicates something is wrong
    }


    float calcFDD(float val1, float val2, float time1, float time2)
    {
        float derivative = (float) ( val2 - val1 ) / ( time2 - time1 );
        return derivative;
    }

    float calcIntegral(float val1, float val2, float time1, float time2)
    {
        float integral = (float) ( time2 - time1 ) * ( ( val1 + val2 ) / 2.0 );
    }



    weightedSumFilter redLEDFilter("redFilter");
    weightedSumFilter blueLEDFilter("blueFilter");

   
    /**
     * @brief timer callback function to add flyCount value to redQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
    */
    void redQueueAdd()
    {
        if(xQueueSendToBack(redLEDFilter.q_handle, (void*) &redLEDFlyCount, (TickType_t) 10) == pdTRUE){
            redLEDFilter.q_size++;
        }
    }

    /**
     * @brief timer callback function to add flyCount value to blueQueue at rate of 100Hz (which is how quickly the CAPDAC updates)
    */
    void blueQueueAdd()
    {
        if(xQueueSendToBack(blueLEDFilter.q_handle, (void*) &blueLEDFlyCount, (TickType_t) 10) == pdTRUE)
        {
            blueLEDFilter.q_size++;
        }
    }




    // so i have to update the queue every 0.01 seconds
    // if i do a timer 



}