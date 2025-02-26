
#pragma once
#include <configs/FILTER_config.h>


namespace Filter
{

    class HighPass
    {
        private:
            typedef int queue_item; // defining what type we will store in the queue
            
        public:
            int q_out = 0;
            int q_in = 0;
            int q_size = 0;
            unsigned long filteredValue = 0;
            SemaphoreHandle_t highpassSemaphore = NULL;
            queue_item* q_handle;
            double * filterCoeffs;
            int filterSize;
            HighPass(double* filter_coeffs, int size); 
            bool incrementQueue(float val);
            int decrementQueue();
            void deleteFilter();
            void resetFilter();
    };

    HighPass::HighPass(double* filter_coeffs, int size)
    {
        this->filterSize = size;
        this->filterCoeffs = filter_coeffs;
        q_handle = (queue_item *) calloc(this->filterSize, sizeof(queue_item)); //initializes to 0
        this->highpassSemaphore = xSemaphoreCreateBinary();
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

        this->q_out = (this->q_out + 1) % this->filterSize;
        
        //this->q_size--;

        return oldVal;
    }

    bool HighPass::incrementQueue(float val)
    {
        int numFilterCoeffs = this->filterSize;

        if(this->q_size == numFilterCoeffs)
        {
            // queue is full, can't add more elements
            return false;
        }

        this->q_in = ( this->q_out + numFilterCoeffs - 1 ) % numFilterCoeffs;
        // we assume this queue is full except for 1 element (that's because they are intialized to 0)

        *(this->q_handle + this->q_in) = val;

        //this->q_size++;

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

    HighPass redLEDPowerFilter = Filter::HighPass(power_filter_taps, POWER_FILTER_TAP_NUM);
    HighPass blueLEDPowerFilter = Filter::HighPass(power_filter_taps, POWER_FILTER_TAP_NUM);
    HighPass redLEDFrequencyFilter = Filter::HighPass(power_filter_taps, POWER_FILTER_TAP_NUM); // change ALVINA
    HighPass blueLEDFrequencyFilter = Filter::HighPass(power_filter_taps, POWER_FILTER_TAP_NUM); // change ALVINA

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