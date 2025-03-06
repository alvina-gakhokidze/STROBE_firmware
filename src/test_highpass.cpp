// #include <configs/SYSTEM_config.h>
// #include <bricks/Derivative_handler.h>
// #include <tasks/filter_tasks.h>

// int oldCounterVal = 1;
// int counterVal = 0;

// void fDD(void* filterStruct)
// {
//     // maybe we have a semaphore here
//     // and when the queue gives it, that means we have most recent average

//     filterTasks::filterParams* localFilters = (filterTasks::filterParams*) filterStruct;
//     dataSmoother::movingAverageFilter* maFilter = localFilters->movingAverage;
//     Filter::HighPass* hpFilter = localFilters->highPassFilter;


//     float newDerivative = Filter::calcFDD((float)oldCounterVal, (float)counterVal, 0, (float) (MAF_PERIOD_MS)); // don't need real time 2 and time1, just need time difference which is always 10ms
    
//     float oldDev = hpFilter->decrementQueue(); // queue is full from beginning, full of 0s
//     bool added = hpFilter->incrementQueue(newDerivative); // since we always remove 1 element first, there will always be one empty when we increment
//     //Serial.printf("%d. old: %lf new: %lf new fdd: %lf old fdd: %lf, added succesfully: %d\n", fCounter, maFilter->oldMovingBiteAverage, maFilter->newMovingBiteAverage, newDerivative, oldDev, added);
// }

// void hP(void* filter)
// {
//     Filter::HighPass* hpFilter = (Filter::HighPass*) filter;
//     Serial.printf("entering high pass task\n");

//     //Serial.printf("High Pass Semaphore given\n");
//     int numFilterCoeffs = hpFilter->filterSize;
//     double * coeffArray = hpFilter->filterCoeffs;

//     int localBack = hpFilter->q_in; //we're going to be going in reverse order for the filter (because latest sample is first in high pass filter)
//     int localFront = hpFilter->q_out;
//     int localSize = numFilterCoeffs;
//     //Serial.printf("back: %d front: %d\n", localBack, localFront);

//     double filtValue = 0; // we just sum.

//     unsigned long time1 = millis();

//     // apply high pass filter weights here
//     for(int i = 0; i < numFilterCoeffs; i++)
//     {
//         // we're reading / "removing" from the back (even though typically you INSERT at the back because that's where newest data goes)
//         localBack = ( localFront + localSize - 1 ) % numFilterCoeffs; 
//         float dVal = *( hpFilter->q_handle + localBack );
//         filtValue += coeffArray[i] * (double) dVal ;
//         localSize--;
//         Serial.printf("dval: %lf\n", dVal);
//     }
//    Serial.printf("\n");

//     hpFilter->filteredValue = filtValue; 

//     Serial.printf("filtered value: %lf\n", filtValue);
//     Serial.printf("\n");
    

//     unsigned long totalTime = millis() - time1; // tells us how long this loop took
//     Serial.printf("time: %lu\n", totalTime);
// }


// void setup()
// {
//     Serial.begin(115200);
// }

// void loop()
// {
//     int temp = counterVal;
//     counterVal += oldCounterVal;
//     oldCounterVal = temp;

//     fDD(&filterTasks::redLEDFilter);

//     for(int i = 0; i<POWER_FILTER_TAP_NUM; i++)
//     {
//         Serial.printf("%lf\n", *(filterTasks::redLEDFilter.highPassFilter->q_handle+i));
//     }
//     // fdd is overwriting correctly and beautifully! yay!! much love!!
//     Serial.printf("\n");

//     hP(&Filter::redLEDPowerFilter);

//     delay(500);

// }
