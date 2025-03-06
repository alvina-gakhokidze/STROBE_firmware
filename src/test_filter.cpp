// #include <configs/SYSTEM_config.h>


// int queueItem = 0;
// float newMovingBiteAverage = 0;
// float oldMovingBiteAverage = 0;
// int q_size = 0;
// QueueHandle_t q_handle;
// int oldItem = 0;

// void queueFiltering(TimerHandle_t xhandle)
// {
//     int oldNum;
//     xQueueReceive(q_handle, &oldNum, (TickType_t) 0); // make space first
//     oldItem = oldNum;
//     int newNum = queueItem; // might be a race condition ... 
//     xQueueSendToBack(q_handle, (void*) &newNum, (TickType_t) 0);
//     oldMovingBiteAverage = newMovingBiteAverage; // store old value (for FDD purposes)
//     newMovingBiteAverage = (oldMovingBiteAverage * (float) 10 - oldNum + newNum ) / ( (float) 10 ) ;
//     // this is called the recrusive implementation of the moving average filter!
// }
// // this looks good! i think our main filters are working then
// void setup()
// {
//     Serial.begin(115200);
//     q_handle = xQueueCreate(10, sizeof(int));
// }


// void loop()
// {

//     while(q_size != 10)
//     {
//         if(xQueueSendToBack(q_handle, (void*) &queueItem, 0) == pdTRUE){
            
//             // oldMovingBiteAverage = newMovingBiteAverage;
//             // newMovingBiteAverage = ( (float) oldMovingBiteAverage * q_size + (float) queueItem) / ( (float) q_size + 1.0 );
//             // don't need to subtract anything because nums being kicked out are assumed to be 0s (empty queue)
//             q_size++;
//         }
//     }

//     queueItem++;
//     queueFiltering(q_handle);
//     Serial.printf("old item: %d", oldItem);
//     Serial.printf(" new item: %d\n", queueItem);
//     Serial.printf("old av: %lf", oldMovingBiteAverage);
//     Serial.printf(" new av: %lf\n", newMovingBiteAverage);
// }