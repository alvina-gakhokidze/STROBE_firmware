
// #include <tasks/LED_tasks.h>
// #include <tasks/filter_tasks.h>
// #include <tasks/PESC_tasks.h>
// #include <tasks/board_tasks.h>
// #include <configs/Task_config.h>

// void setup()
// {
//     // need to initialize pins 

//     Serial.begin(115200);
//     if( boardTasks::determineUserOrEEPROM() )
//     {
//         boardTasks::setupBoard(); // get user to choose settings
//     }
//     else
//     {
//         boardTasks::loadBoardConfigs(); // load previous experiment values from EEPROM
//     }
    

    
    
//     xTaskCreatePinnedToCore(
//         ledTasks::updateFlyCountTask,      // Function that should be called
//         "Fly Count for Red LED",             // Name of the task (for debugging)
//         10000,                            // Stack size (bytes)
//         &strobeLED::redLED,                            // Parameter to pass
//         2,                               // Task priority
//         &TaskHandlers::getRedLEDFlyCount, // Task handle
//         0                                // Pin to core 0
//     );

//     xTaskCreatePinnedToCore(
//         ledTasks::updateFlyCountTask,      // Function that should be called
//         "Fly Count for Blue LED",             // Name of the task (for debugging)
//         10000,                            // Stack size (bytes)
//         &strobeLED::blueLED,                            // Parameter to pass
//         2,                               // Task priority
//         &TaskHandlers::getBlueLEDFlyCount, // Task handle
//         0                                // Pin to core 0
//     );


//     if(boardTasks::thisBoard.manualMode)
//     {
//         //only tasks that are needed for manual mode go here

//         // nothing more needed actually


//     }
//     else if(boardTasks::thisBoard.optimizationMode)
//     {
//         // only tasks that are needed for optimization mode go here
//         if(boardTasks::thisBoard.redLEDOn)
//         {
//             xTaskCreatePinnedToCore(
//                 filterTasks::finiteDifferenceDerivative,           // Function that should be called
//                 "FDD for redLED",               // Name of the task (for debugging)
//                 10000,                                  // Stack size (bytes)
//                 &filterTasks::redLEDFilter,                    // Parameter to pass
//                 2,                                      // Task priority
//                 &TaskHandlers::redFDDFilter,      // Task handle
//                 0                                       // Pin to core 0
//             );

//             if(boardTasks::thisBoard.powerOn)
//             {
//                 xTaskCreatePinnedToCore(
//                     filterTasks::highPassFilterTask,           // Function that should be called
//                     "HighPass Power Filter for redLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &Filter::redLEDPowerFilter,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::redLEDPowerHighPass,      // Task handle
//                     0                                       // Pin to core 0
//                 );

//                 xTaskCreatePinnedToCore(
//                     ESCTasks::ESCCalculations,           // Function that should be called
//                     "Power ESC for redLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &ESCTasks::redPower,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::redLEDPowerESC,      // Task handle
//                     0                                       // Pin to core 0
//                 );

//             }

//             if(boardTasks::thisBoard.frequencyOn)
//             {
//                 xTaskCreatePinnedToCore(
//                     filterTasks::highPassFilterTask,           // Function that should be called
//                     "HighPass Frequency Filter for redLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &Filter::redLEDFrequencyFilter,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::redLEDFrequencyHighPass,      // Task handle
//                     0                                       // Pin to core 0
//                 );

//                 xTaskCreatePinnedToCore(
//                     ESCTasks::ESCCalculations,           // Function that should be called
//                     "Frequency ESC for redLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &ESCTasks::redFrequency,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::redLEDFrequencyESC,      // Task handle
//                     0                                       // Pin to core 0
//                 );
//             }
            
//         }
        
//         if(boardTasks::thisBoard.blueLEDOn)
//         {
//             xTaskCreatePinnedToCore(
//                 filterTasks::finiteDifferenceDerivative,           // Function that should be called
//                 "FDD for blueLED",               // Name of the task (for debugging)
//                 10000,                                  // Stack size (bytes)
//                 &filterTasks::blueLEDFilter,                    // Parameter to pass
//                 2,                                      // Task priority
//                 &TaskHandlers::blueFDDFilter,      // Task handle
//                 0                                       // Pin to core 0
//             );

//             if(boardTasks::thisBoard.powerOn)
//             {
//                 xTaskCreatePinnedToCore(
//                     filterTasks::highPassFilterTask,           // Function that should be called
//                     "HighPass Power Filter for blueLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &Filter::blueLEDPowerFilter,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::blueLEDPowerHighPass,      // Task handle
//                     0                                       // Pin to core 0
//                 );

//                 xTaskCreatePinnedToCore(
//                     ESCTasks::ESCCalculations,           // Function that should be called
//                     "Power ESC for blueLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &ESCTasks::bluePower,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::blueLEDPowerESC,      // Task handle
//                     0                                       // Pin to core 0
//                 );

//             }

//             if(boardTasks::thisBoard.frequencyOn)
//             {
//                 xTaskCreatePinnedToCore(
//                     filterTasks::highPassFilterTask,           // Function that should be called
//                     "HighPass Frequency Filter for blueLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &Filter::blueLEDFrequencyFilter,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::blueLEDFrequencyHighPass,      // Task handle
//                     0                                       // Pin to core 0
//                 );

//                 xTaskCreatePinnedToCore(
//                     ESCTasks::ESCCalculations,           // Function that should be called
//                     "Frequency ESC for blueLED",               // Name of the task (for debugging)
//                     10000,                                  // Stack size (bytes)
//                     &ESCTasks::blueFrequency,                    // Parameter to pass
//                     2,                                      // Task priority
//                     &TaskHandlers::blueLEDFrequencyESC,      // Task handle
//                     0                                       // Pin to core 0
//                 );
//             }
            
//         }
//     }
// }

// void loop()
// {
//     // "Espressif places most support tasks in CPU 0, leaving CPU 1 for appli- cation use ."
//     // need to determine stack size for each task
//     // vTaskDelay() 
//     vTaskDelete(NULL);
// }


