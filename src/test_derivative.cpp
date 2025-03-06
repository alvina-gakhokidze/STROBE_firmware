// #include <configs/SYSTEM_config.h>
// #include <bricks/Smoothing_handler.h>
// #include <bricks/Derivative_handler.h>
// #include <tasks/filter_tasks.h>


// int devVal = 0;
// int devVal2 = 10;

// int counter = 0;
// void fDD(void* filterStruct)
// {
//     // maybe we have a semaphore here
//     // and when the queue gives it, that means we have most recent average

//     filterTasks::filterParams* localFilters = (filterTasks::filterParams*) filterStruct;
//     dataSmoother::movingAverageFilter* maFilter = localFilters->movingAverage;
//     Filter::HighPass* hpFilter = localFilters->highPassFilter;

//     //semaphore should be triggered at rate of 100Hz, everytime new value is put into queue
        
//     float newDerivative = Filter::calcFDD(devVal, devVal2, 0, (float) (MAF_PERIOD_MS/1000.0)); // don't need real time 2 and time1, just need time difference which is always 10ms
    
//     float oldDev = hpFilter->decrementQueue(); // queue is full from beginning, full of 0s
//     bool added = hpFilter->incrementQueue(newDerivative); // since we always remove 1 element first, there will always be one empty when we increment
//     Serial.printf("old: %d new: %d new fdd: %lf old fdd: %lf, added succesfully: %d\n", devVal, devVal2, newDerivative, oldDev, added);
// }
        


// void setup()
// {
//     Serial.begin(115200);
// }

// void loop()
// {
//     if(counter == 75)
//     {
//         counter = 0;
//     }
//     else{
//         counter++;
//     }

//     devVal++;

//     devVal2 += devVal;
//     fDD(&filterTasks::redLEDFilter);
//     Serial.printf("%d\n", counter);
// }