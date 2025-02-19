
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
            int q_out = 0;
            int q_in = 0;
            int q_size = 0;
            unsigned long oldFilteredValue = 0;
            unsigned long newFilteredValue = 0;
            queue_item* q_handle;
            HighPass(); 
            bool incrementQueue(double val);
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

        if(this->q_size == 0) // queue is empty, nothing to decrement
        {
            return -1; 
        }
        
        oldVal = *(this->q_handle + this->q_out); // ALVINA ARRAY DEREFERENCING
        *(this->q_handle + this->q_out) = 0; 

        this->q_out = (this->q_out + 1) % FILTER_TAP_NUM;
        
        this->q_size--;

        return oldVal;
    }

    bool HighPass::incrementQueue(double val)
    {

        if(this->q_size == FILTER_TAP_NUM)
        {
            // queue is full, can't add more elements
            return false;
        }

        this->q_in = ( this->q_out + this->q_size ) % FILTER_TAP_NUM;

        *(this->q_handle + this->q_in) = val;

        this->q_size++;

        return true;
    }

    void HighPass::deleteFilter()
    {
        free(this->q_handle);
    }

    void HighPass::resetFilter()
    {
        this->q_out = 0;
        this->q_in = 0;
        this->q_size = 0;
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