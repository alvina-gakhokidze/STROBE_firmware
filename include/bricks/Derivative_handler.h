
#pragma once
#include <configs/FILTER_config.h>


namespace Filter
{

    class FDD
    {

    };

    class HighPass
    {
        private:
            typedef int queue_item; // defining what type we will store in the queue
            
        public:
            int q_front = 0;
            int q_back = 0;
            int q_size = 0;
            unsigned long oldFilteredValue = 0;
            unsigned long newFilteredValue = 0;
            queue_item* q_handle;
            HighPass(); 
            int incrementQueue(double val);
            int decrementQueue();
            void deleteFilter();
            void resetFilter();
    };

    HighPass::HighPass()
    {
        q_handle = (queue_item *) malloc(FILTER_TAP_NUM * sizeof(queue_item));
    }

    int HighPass::decrementQueue()
    {
        int oldVal = -1;

        if(this->q_size == 0)
        {
            // queue is empty, nothing to decrement
            return -1; 
        }
        
        oldVal = *(this->q_handle + this->q_front); // ALVINA ARRAY DEREFERENCING
        *(this->q_handle + this->q_front) = 0; 
        

        if(this->q_front == FILTER_TAP_NUM - 1)
        {
            // front was at end, so now it circles around
            this->q_front = 0;
        }
        else
        {
            this->q_front++; 
        }
        
        this->q_size--;

        return oldVal;
    }

    int HighPass::incrementQueue(double val)
    {

        if(this->q_size == FILTER_TAP_NUM)
        {
            // queue is full, can't add more elements
            return -1;
        }

        *(this->q_handle + this->q_back) = val;

        if(this->q_back == 0)
        {

        }

        // ACTUALLY WE HAVE TO CHECK IF IT'S FULL!!!

        if(xQueueSendToBack(this->q_handle, (void*) &val, (TickType_t) 10) == pdTRUE){
            this->q_size++;
            return true;
        }

        return false;
    }

    void HighPass::deleteFilter()
    {
        vQueueDelete(this->q_handle);
    }

    void HighPass::resetFilter()
    {
        xQueueReset(this->q_handle);
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

}