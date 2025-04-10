/**
 * This file takes care of converting user input to correct values, saving them to the correct objects, and setting up the board peripherals for operation
 */


#pragma once
#include <configs/SYSTEM_config.h>
#include <bricks/DAC_handler.h>
#include <bricks/Derivative_handler.h>
#include <bricks/ESC_handler.h>
#include <bricks/LED_handler.h>
#include <bricks/Smoothing_handler.h>
#include <tasks/PESC_tasks.h>
#include <tasks/LED_tasks.h>


namespace boardTasks
{

    Preferences permanentMemory; // this is used when we want to write to permanent memory. 
    // OBSOLETE thanks to wifi/broadcast feature

    typedef struct boardParameters{
        bool dataReceived;
        bool redLEDOn;
        bool blueLEDOn;
        bool powerOn;
        bool frequencyOn;
        bool manualMode;
    } boardParameters; 

    typedef struct wifiBoardParams {
        bool redLEDOn;
        bool blueLEDOn;
        bool manualMode;
        bool powerOn;
        bool frequencyOn;
        float redLEDFrequency;
        float redLEDPower;
        float blueLEDFrequency;
        float blueLEDPower;
    } wifiBoardParams;

    wifiBoardParams wifiData;


    boardParameters thisBoard = { 
            false, //dataReceived
            false, //redLEDOn
            false, //blueLEDOn
            false, //powerOn
            false, //frequencyOn
            true   //manualMode
        };

    void setupBoardForParameterOptimization();
    void setLEDFrequency(strobeLED::LED* ledObject, float ledFrequency);
    void setLEDPower(strobeLED::LED* ledObject, float ledCurrent);
    void setupBoard();
    int getUserInput(int numOptions, const char* programQuestion);
    float getUserFloatInput(const char* programQuestion, float lowLimit, float upLimit );
    void calibrateLED(strobeLED::LED* localLED, char* mode);
    bool determineUserOrEEPROM();
    void saveToPermanentMemoryBool(Preferences* memory, char* keyName, bool keyVal);
    bool getFromPermanentMemoryBool(Preferences* memory, char* keyName);
    void saveToPermanentMemoryFloat(Preferences* memory, char* keyName, float keyVal);
    float getFromPermanentMemoryFloat(Preferences* memory, char* keyName);
    void saveBoardConfigs();
    void loadBoardConfigs();
    void setupLEDs();
    void setupBoardWifi(const uint8_t * mac, const uint8_t *inData, int len);
    bool stopBoard();
    void createTasks();


    char redLEDOnString[] = "redLEDOn";
    char blueLEDOnString[] = "blueLEDOn";
    char powerOnString[] = "powerOn";
    char frequencyOnString[] = "frequencyOn";
    char manualModeString[] = "manualMode";
    char optModeString[] = "optMode";
    char redLEDPowerString[] = "redLEDPower";
    char redLEDPeriodString[] = "redLEDPeriod";
    char blueLEDPowerString[] = "blueLEDPower";
    char blueLEDPeriodString[] = "blueLEDPeriod";
    char powerString[] = "power";
    char frequencyString[] = "frequency";
    char redLEDFlashingEnabledString[] = "rLEDfEnabled";
    char blueLEDFlashingEnabledString[] = "bLEDfEnabled";

    

    /**
     * @brief sets up LEDs and DACs on the board. Should be done last because this 
     * starts the LED hardware interrupts.
     */
    void setupPeripherals()
    {
        Serial.printf("Setting up LEDs\n");
        setupLEDs();
        Serial.printf("Setting up DACs\n");
        registerTalk::setupDAC(&strobeLED::redLEDBus);
        registerTalk::setupDAC(&strobeLED::blueLEDBus);
        Serial.printf("finished setting up DACs\n");
    }

    /**
     * @brief enables smoothing, filtering, and the associated ESCs
     */
    void setupBoardForParameterOptimization()
    {
        Serial.printf("Setting up board for parameter optimization\n");

        if(thisBoard.redLEDOn)
        {
            if(thisBoard.powerOn)
            {
                ESCTasks::escInit(&ESCTasks::redPower);
            }
            if(thisBoard.frequencyOn)
            {
                ESCTasks::escInit(&ESCTasks::redFrequency);
            }
        }
        
        if(thisBoard.blueLEDOn)
        {
            if(thisBoard.powerOn)
            {
                ESCTasks::escInit(&ESCTasks::bluePower);
            }

            if(thisBoard.frequencyOn)
            {
                ESCTasks::escInit(&ESCTasks::blueFrequency);
            }
        }

        setupPeripherals();
    }

    /**
     * @brief convers frequency to period for timer
     */
    void setLEDFrequency(strobeLED::LED* ledObject, float ledFrequency)
    {
        ledObject->period_us = ( 1.0 / (ledFrequency)  ) * 0.5 * 1000000.0;
    }

    /**
     * @brief converts current to voltage that needs to be applied across 5 ohm power resistor
     */
    void setLEDPower(strobeLED::LED* ledObject, float ledCurrent)
    {
        ledObject->power = ledCurrent / 1000.0 * POWER_RESISTOR_VALUE;
    }


    /**
     * @brief determines which LEDs user wants to use, and what mode (manual/optimization)
     * OBSOLETE thanks to wifi/broadcast feature
     */
    void setupBoard()
    {

        Serial.printf("Welcome to the new STROBE!\n");

        const char* ledChoice = "Would you like to use:\n 1. Red LED \n 2. Blue LED \n 3. Both LEDs \n 4. Neither LED \n";

        int ledMode = getUserInput(4, ledChoice);

        switch(ledMode)
        {
            case 1:
                Serial.printf("Red LED Chosen \n");
                thisBoard.redLEDOn = true;
                thisBoard.blueLEDOn = false;
                break;
            case 2:
                Serial.printf("Blue LED Chosen \n");
                thisBoard.blueLEDOn = true;
                thisBoard.redLEDOn = false;
                break;
            case 3:
                Serial.printf("Both LEDs Chosen \n");
                thisBoard.redLEDOn = true;
                thisBoard.blueLEDOn = true;
                break;
            case 4:
                Serial.printf("Neither LED Chosen \n");
                thisBoard.redLEDOn = false;
                thisBoard.blueLEDOn = false;
                break;
            default:
                Serial.printf("Something's wrong with ledChoice!\n");
                while(1);
        }
        
       
        const char* modeChoice = "What mode would you like to use:\n 1. Manual Mode \n 2. Parameter Optimization Mode\n";

        int operationMode = getUserInput(2, modeChoice);

        switch(operationMode)
        {
            case 1:
            {
                Serial.printf("Manual Mode Chosen\n");

                thisBoard.manualMode = true;

                if(thisBoard.redLEDOn)
                {
                    Serial.printf("Time to choose the parameters for the red LED! \n");
                    calibrateLED(&strobeLED::redLED, powerString);
                    calibrateLED(&strobeLED::redLED, frequencyString);
                }

                if(thisBoard.blueLEDOn)
                {
                    Serial.printf("Time to choose the parameters for the blue LED! \n");
                    calibrateLED(&strobeLED::blueLED, powerString);
                    calibrateLED(&strobeLED::blueLED, frequencyString);
                }
                Serial.printf("Now setting up peripherals\n");
                setupPeripherals();
                break;
            }
            case 2:
            {
                Serial.printf("Parameter Optimization Mode Chosen\n");
                thisBoard.manualMode = false;

                const char * optimizationFeatures = "What would you like to optimize:\n 1. Power \n 2. Frequency \n 3. Both \n";
                int optimizationMode = getUserInput(3, optimizationFeatures);

                switch(optimizationMode)
                {
                    case 1:
                        Serial.printf("Only Power being optimized.\n");
                        thisBoard.powerOn = true;
                        thisBoard.frequencyOn = false;
                        if(thisBoard.redLEDOn)
                        {
                            Serial.printf("Calibrating Red LED. \n");
                            calibrateLED(&strobeLED::redLED, frequencyString);
                            
                        }
                        if(thisBoard.blueLEDOn)
                        {
                            Serial.printf("Calibrating Blue LED. \n");
                            calibrateLED(&strobeLED::blueLED, frequencyString);
                            
                        }
                        break;
                    case 2:
                        Serial.printf("Only Frequency being optimized.\n");
                        thisBoard.frequencyOn = true;
                        thisBoard.powerOn = false;
                        if(thisBoard.redLEDOn)
                        {
                            Serial.printf("Calibrating Red LED. \n");
                            calibrateLED(&strobeLED::redLED, powerString);
                        }
                        if(thisBoard.blueLEDOn)
                        {
                            Serial.printf("Calibrating Blue LED. \n");
                            calibrateLED(&strobeLED::blueLED, powerString);
                        }
                        break;
                    case 3:
                        Serial.printf("Both Power and Frequency being optimized.\n");
                        thisBoard.powerOn = true;
                        thisBoard.frequencyOn = true;
                        //dont need to do anything else here
                        break;
                }
                setupBoardForParameterOptimization();
                break;
            }
            default:  
            { 
                Serial.printf("Invalid Choice. Something wrong with choosing mode! \n");
                while(1);
            }
        }
    }

    /**
     * @brief read Serial input from user, keep asking until valid option chosen
     * OBSOLETE thanks to wifi/broadcast feature
     */
    int getUserInput(int numOptions, const char* programQuestion)
    {
        // KEY ASSUMPTION: that we always start option numbering at 1

        int userInput = 0;
        
        while(userInput < 1 || userInput > numOptions)
        {
            Serial.printf("%s\n", programQuestion);

            while (Serial.available() == 0);

            userInput = Serial.parseInt();
            
            if(userInput < 1 || userInput > numOptions)
            {
                Serial.printf("Invalid Option! Try Again\n");
                
            }
        }

        //exit once valid option has been chosen

        return userInput;
        
    }

    /** 
     * @brief function that gets float input, and ensures it's within limits, otherwise prompts user to try again
     * OBSOLETE thanks to wifi/broadcast feature
     */
    float getUserFloatInput(const char* programQuestion, float lowLimit, float upLimit )
    {
        float userInput = lowLimit - 1.0; // ensuring we're lower than lowest limit
        
        while(userInput < lowLimit || userInput > upLimit )
        {
            Serial.printf("%s\n", programQuestion);

            while( Serial.available() == 0 );

            userInput = Serial.parseFloat();
            
            if(userInput < lowLimit)
            {
                Serial.printf("Value too Low! Cannot use value lower than %.0f. Try Again\n", lowLimit);
            }
            else if(userInput > upLimit)
            {
                Serial.printf("Value too high! Cannot use value higher than: %.0f. Try again \n", upLimit);
            }
        }

        return userInput;
    }

    /**
     * @brief function that prompts user to provide LED current and LED frequency for manual operation
     * OBSOLETE thanks to wifi/broadcast feature
     */
    void calibrateLED(strobeLED::LED* localLED, char* mode)
    {
        Serial.printf("Calibrating power of LED\n");
        const char* currentChoice = "Provide the LED current in mA: \n";
        const char* frequencyChoice = "Provide the LED frequency in Hz: \n";

        if(mode == powerString){

            float ledCurrent = getUserFloatInput(currentChoice, CURR_LOW_LIMIT, CURR_UPP_LIMIT);

            Serial.printf("Value chosen by user: %.0lf mA\n", ledCurrent);

            // converting it to voltage that will be applied across the resistor to create desired current
            localLED->power = (float) ledCurrent / 1000.0 * POWER_RESISTOR_VALUE; // V = I * R formula
            Serial.printf("power is %lf\n", localLED->power);
        }

        if(mode == frequencyString)
        {
            float ledFrequency = getUserFloatInput(frequencyChoice, FREQ_LOW_LIMIT, FREQ_UPP_LIMIT);

            Serial.printf("Value chosen by user: %.0lf Hz\n", ledFrequency);
            if(ledFrequency == 0.0)
            {
                Serial.printf("Frequency set to 0 so flashing disabled\n");
                if(localLED == &strobeLED::redLED)
                {
                    strobeLED::redLED.flashingEnabled = false;
                }
                else
                {
                    strobeLED::blueLED.flashingEnabled = false;
                }
            }
            else
            {
                localLED->period_us =  ( 1.0 / ledFrequency ) * 1000000.0 ;
                Serial.printf("Period is: %.0lf\n", localLED->period_us);
            }
        }
    }

    /**
     * @brief if no input from user, we will load 
     * @return true if specifiying new parameters, false if we want to load old ones
     * tip: do not press enter after providing input!
     * OBSOLETE thanks to wifi/broadcast feature
     */
    bool determineUserOrEEPROM()
    {
        Serial.printf("Would you like to specify new experiment parameters? Y/N \n");
        unsigned long waitingTime = millis();
        unsigned long maxWaitTime = 10000; //10 second wait time for input
        
        while( Serial.available() == 0 && millis()-waitingTime < maxWaitTime );
        
        if( millis() - waitingTime > maxWaitTime )
        {
            // means no input given in the past 10 seconds, so reload default values
            Serial.printf("Using previous experiment configuration\n");
            return false;
        }
        else
        {
            String userChoice = Serial.readString();
            if(userChoice == "Y" || userChoice == "y" || userChoice == "YES" || userChoice == "yes" || userChoice == "YE" || userChoice == "ye" )
            {
                Serial.printf("User indicated new experiment settings to be used\n");
                return true;
            }
            
            Serial.printf("Using previous experiment configuration\n");
            return false;
        }
    }

    /**
     * @brief save the values to permanent memory so that upon restart we don't need user input again
     * OBSOLETE thanks to wifi/broadcast feature
     */
    void saveToPermanentMemoryBool(Preferences* memory, char* keyName, bool keyVal)
    {
        memory->begin(keyName, false);
        memory->putBool(keyName, keyVal);
        memory->end();
    }

    bool getFromPermanentMemoryBool(Preferences* memory, char* keyName)
    {
        memory->begin(keyName, false);
        bool val = memory->getBool(keyName);
        memory->end();
        
        return val;
    }

    void saveToPermanentMemoryFloat(Preferences* memory, char* keyName, float keyVal)
    {
        memory->begin(keyName, false);
        memory->putFloat(keyName, keyVal);
        memory->end();
    }   

    float getFromPermanentMemoryFloat(Preferences* memory, char* keyName)
    {
        memory->begin(keyName, false);
        float val = memory->getFloat(keyName);
        memory->end();
        
        return val;
    }

    /**
     * @brief saves board configs to permenent memory so they can be reloaded later if needed
     * OBSOLETE thanks to wifi/broadcast feature
     */
    void saveBoardConfigs()
    {
        //the keyNames are limited to 15 chars

        // experiment parameters
        saveToPermanentMemoryBool(&permanentMemory, redLEDOnString, thisBoard.redLEDOn);
        saveToPermanentMemoryBool(&permanentMemory, blueLEDOnString, thisBoard.blueLEDOn);
        saveToPermanentMemoryBool(&permanentMemory, powerOnString, thisBoard.powerOn);
        saveToPermanentMemoryBool(&permanentMemory, frequencyOnString, thisBoard.frequencyOn);
        saveToPermanentMemoryBool(&permanentMemory, manualModeString, thisBoard.manualMode);

        saveToPermanentMemoryBool(&permanentMemory, redLEDFlashingEnabledString, strobeLED::redLED.flashingEnabled);
        saveToPermanentMemoryBool(&permanentMemory, blueLEDFlashingEnabledString, strobeLED::blueLED.flashingEnabled);
        
        //LED power and frequency values
        saveToPermanentMemoryFloat(&permanentMemory, redLEDPowerString, strobeLED::redLED.power);
        saveToPermanentMemoryFloat(&permanentMemory, redLEDPeriodString, strobeLED::redLED.period_us);

        saveToPermanentMemoryFloat(&permanentMemory, blueLEDPowerString, strobeLED::blueLED.power);
        saveToPermanentMemoryFloat(&permanentMemory, blueLEDPeriodString, strobeLED::blueLED.period_us);

    }

    /**
     * @brief if no user input given for 10 seconds, assume we will just use conditions of previous experiment
     * OBSOLETE thanks to wifi/broadcast feature
     */
    void loadBoardConfigs()
    {
        thisBoard.redLEDOn = getFromPermanentMemoryBool(&permanentMemory, redLEDOnString);
        if(thisBoard.redLEDOn)
        {
            Serial.printf("Red LED in operation.\n");
        }
        else
        {
            Serial.printf("Red LED not being used.\n");
        }
        thisBoard.blueLEDOn = getFromPermanentMemoryBool(&permanentMemory, blueLEDOnString);
        if(thisBoard.blueLEDOn)
        {
            Serial.printf("Blue LED in operation.\n");
        }
        else
        {
            Serial.printf("Blue LED not being used.\n");
        }
        thisBoard.manualMode = getFromPermanentMemoryBool(&permanentMemory, manualModeString);
        if(thisBoard.manualMode)
        {
            Serial.printf("Manual mode on.\n");
            if(thisBoard.redLEDOn)
            {
                strobeLED::redLED.flashingEnabled = getFromPermanentMemoryBool(&permanentMemory, redLEDFlashingEnabledString);
                if(strobeLED::redLED.flashingEnabled)
                {
                    Serial.printf("red led flashing enabled");
                }
                else
                {
                    Serial.printf("red led flashing disabled");
                }
            }
            if(thisBoard.blueLEDOn)
            {
                strobeLED::blueLED.flashingEnabled = getFromPermanentMemoryBool(&permanentMemory, blueLEDFlashingEnabledString);
                if(strobeLED::blueLED.flashingEnabled)
                {
                    Serial.printf("blue led flashing enabled");
                }
                else
                {
                    Serial.printf("blue led flashing disabled");
                }
            }
                
        }
        else
        {
            Serial.printf("Optimization mode on\n");
            thisBoard.powerOn = getFromPermanentMemoryBool(&permanentMemory, powerOnString);
            
            if(thisBoard.powerOn)
            {
                Serial.printf("Power being optimized\n");
            }
            else
            {
                Serial.printf("Power not being optimized\n");
            }

            thisBoard.frequencyOn = getFromPermanentMemoryBool(&permanentMemory, frequencyOnString);

            if(thisBoard.frequencyOn)
            {
                Serial.printf("Frequency being optimized.\n");
            }
            else
            {
                Serial.printf("Frequency not being optimized.\n");
            }

            setupBoardForParameterOptimization();
        }
    
        strobeLED::redLED.power = getFromPermanentMemoryFloat(&permanentMemory, redLEDPowerString);
        Serial.printf("If applicable, previous redLED power: %lf in mA\n", strobeLED::redLED.power * 1000.0 / POWER_RESISTOR_VALUE);
        strobeLED::redLED.period_us = getFromPermanentMemoryFloat(&permanentMemory, redLEDPeriodString);
        Serial.printf("If applicable, previous redLED frequency: %lf in Hz\n", 1.0 / strobeLED::redLED.period_us * 1000000.0);

        strobeLED::blueLED.power = getFromPermanentMemoryFloat(&permanentMemory, blueLEDPowerString);
        Serial.printf("If applicable, previous blueLED power: %lf in mA\n", strobeLED::blueLED.power * 1000.0 / POWER_RESISTOR_VALUE);
        strobeLED::blueLED.period_us = getFromPermanentMemoryFloat(&permanentMemory, blueLEDPeriodString);
        Serial.printf("If applicable, previous blueLED frequency: %lf in Hz\n", 1.0 / strobeLED::blueLED.period_us * 1000000.0);
    }



    /**
     * @brief begins i2c on led buses and sets interrupt pins to correct mode and attaches interrupts
    */
   void setupLEDs()
   {

       if(boardTasks::thisBoard.redLEDOn)
       {
           pinMode(RED_INT, INPUT);
           
           if(strobeLED::redLED.flashingEnabled)
           {
                Serial.printf("Attaching interrupt for red LED \n");
                attachInterrupt(digitalPinToInterrupt(RED_INT), &strobeLED::changeRedLEDFlash, CHANGE);
           }
           else
           {
               Serial.printf("no flashing for red LED\n");
               attachInterrupt(digitalPinToInterrupt(RED_INT), &strobeLED::changeRedLED, CHANGE);
           }
       }

       if(boardTasks::thisBoard.blueLEDOn)
       {
           pinMode(BLUE_INT, INPUT);
           
           if(strobeLED::blueLED.flashingEnabled)
           {
                Serial.printf("Attaching interrupt for blue LED \n");
                attachInterrupt(digitalPinToInterrupt(BLUE_INT), &strobeLED::changeBlueLEDFlash, CHANGE);
           }
           else
           {
               Serial.printf("no flashing for blue LED");
               attachInterrupt(digitalPinToInterrupt(BLUE_INT), &strobeLED::changeBlueLED, CHANGE);
           }

       }
   }

   /**
     * @brief calls setup functions to reload features from previous experiment
     * OBSOLETE thanks to wifi/broadcast feature
     */
   void setupBoardFromPreviousExperiment()
   {
        boardTasks::loadBoardConfigs();
        setupPeripherals();
   }

   bool initBroadcast()
   {
        //Set device as a Wi-Fi Station
        WiFi.mode(WIFI_STA);

        //Init ESP-NOW
        if (esp_now_init() != ESP_OK) 
        {
            Serial.println("Error initializing ESP-NOW");
            return false;
        }

        esp_now_register_recv_cb(esp_now_recv_cb_t(setupBoardWifi)); // funtion to start monitoring for input

        return true;
   }

      /**
     * @brief for wifi/broadcast feature - function that is called to stop STROBE operation temporarily before new values can be loaded
     */
   bool stopBoard()
   {
        //assume manual mode for now 
        //when parameter optimization implemented, additional code needs to be added this
        // to delete ESC tasks, etc. and reset the board for new parameters

        Serial.printf("Stopping board!\n");

        if(thisBoard.manualMode)
        {

            Serial.printf("Detaching interrupts\n");
            if(thisBoard.redLEDOn)
            {
                vTaskDelete(TaskHandlers::redLEDToggle);

                if(strobeLED::redLED.flashingEnabled){
                    vTaskDelete(TaskHandlers::redLEDFlash);
                }   

                detachInterrupt(digitalPinToInterrupt(RED_INT));
                strobeLED::redLED.ledFlyCount = 0;
                strobeLED::redLEDFlyCount = 0;
                xSemaphoreTake(strobeLED::redLED.onFlashSemaphore, (TickType_t) 20);
                xSemaphoreTake(strobeLED::redLED.onSemaphore, (TickType_t) 20);
                xSemaphoreTake(strobeLED::redLED.offFlashSemaphore, (TickType_t) 20);
                xSemaphoreTake(strobeLED::redLED.offSemaphore, (TickType_t) 20);
    
            }
            if(thisBoard.blueLEDOn)
            {
                vTaskDelete(TaskHandlers::blueLEDToggle);

                if(strobeLED::blueLED.flashingEnabled){
                    vTaskDelete(TaskHandlers::blueLEDFlash);
                }   

                detachInterrupt(digitalPinToInterrupt(BLUE_INT));
                strobeLED::blueLED.ledFlyCount = 0;
                strobeLED::blueLEDFlyCount = 0;

                xSemaphoreTake(strobeLED::blueLED.onFlashSemaphore, (TickType_t) 20);
                xSemaphoreTake(strobeLED::blueLED.onSemaphore, (TickType_t) 20);
                xSemaphoreTake(strobeLED::blueLED.offFlashSemaphore, (TickType_t) 20);
                xSemaphoreTake(strobeLED::blueLED.offSemaphore, (TickType_t) 20);
            }
           
            registerTalk::ledOff(strobeLED::redLED.busptr);
            registerTalk::ledOff(strobeLED::blueLED.busptr);

            return true;
        }

        return true;
   }

      /**
     * @brief sets up board based on inputs given from broadcast message
     */
    void setupBoardWifi(const uint8_t * mac, const uint8_t *inData, int len)
    {

        Serial.printf("Welcome to the new STROBE!\n");

        memcpy(&wifiData, inData, sizeof(wifiData));

        if(thisBoard.dataReceived) // checking to see if we've uploaded data before
        {
            stopBoard(); // resetting values before continuing
        }

        registerTalk::ledOff(&strobeLED::redLEDBus);
        registerTalk::ledOff(&strobeLED::blueLEDBus);

        thisBoard.dataReceived = true;


        thisBoard.manualMode = wifiData.manualMode;

        if(thisBoard.manualMode)
        {
            Serial.printf("Manual Mode Enabled\n");
            thisBoard.redLEDOn = wifiData.redLEDOn;
            thisBoard.blueLEDOn = wifiData.blueLEDOn;

            if(thisBoard.redLEDOn)
            {
                Serial.printf("Red LED Enabled\n");
                //strobeLED::redLED.power = wifiData.redLEDPower / 1000.0 * POWER_RESISTOR_VALUE;
                setLEDPower(&strobeLED::redLED, wifiData.redLEDPower);
                if(wifiData.redLEDFrequency == 0)
                {
                    strobeLED::redLED.flashingEnabled = false;
                    strobeLED::redLED.period_us = 0;
                }
                else{
                    strobeLED::redLED.flashingEnabled = true;
                    //strobeLED::redLED.period_us = ( 1.0 / (wifiData.redLEDFrequency * 2.0)  ) * 1000000.0;
                    setLEDFrequency(&strobeLED::redLED, wifiData.redLEDFrequency);
                }
                Serial.printf("Red LED Power: %lf, Red LED Period: %lf\n", strobeLED::redLED.power, strobeLED::redLED.period_us); 
            }
            if(thisBoard.blueLEDOn)
            {
                Serial.printf("Blue LED Enabled\n");
                //strobeLED::blueLED.power = wifiData.blueLEDPower  / 1000.0 * POWER_RESISTOR_VALUE;
                setLEDPower(&strobeLED::blueLED, wifiData.blueLEDPower);
                if(wifiData.blueLEDFrequency == 0)
                {
                    strobeLED::blueLED.flashingEnabled = false;
                    strobeLED::blueLED.period_us = 0;
                }
                else
                {
                    strobeLED::blueLED.flashingEnabled = true;
                    //strobeLED::blueLED.period_us = ( 1.0 / (wifiData.blueLEDFrequency * 2.0)  ) * 1000000.0; 
                    setLEDFrequency(&strobeLED::blueLED, wifiData.blueLEDFrequency);
                }
                Serial.printf("Blue LED Power: %lf, Blue LED Period: %lf\n", strobeLED::blueLED.power, strobeLED::blueLED.period_us); 
            }
            Serial.printf("Now setting up peripherals\n");
            setupPeripherals();
            createTasks();
            Serial.printf("Finished setting up board");
        }
        else
        {
            Serial.printf("Parameter Optimization Mode Enabled\n");
            thisBoard.powerOn = wifiData.powerOn;
            thisBoard.frequencyOn = wifiData.frequencyOn;

            if(thisBoard.powerOn && thisBoard.frequencyOn)
            {
                Serial.printf("Both power and frequency being optimized\n");
            }
            else if(thisBoard.powerOn)
            {
                Serial.printf("Only power being optimized\n");

                if(thisBoard.redLEDOn)
                {
                    //strobeLED::redLED.period_us = ( 1.0 / (wifiData.redLEDFrequency*2.0)  ) * 1000000.0;
                    setLEDFrequency(&strobeLED::redLED, wifiData.redLEDFrequency);
                    Serial.printf("Red LED Period: %lf\n", strobeLED::redLED.period_us); 
                }
                if(thisBoard.blueLEDOn)
                {
                    //strobeLED::blueLED.period_us = ( 1.0 / (wifiData.blueLEDFrequency*2.0)  ) * 1000000.0;
                    setLEDFrequency(&strobeLED::blueLED, wifiData.blueLEDFrequency);
                    Serial.printf("Blue LED Period: %lf\n", strobeLED::blueLED.period_us); 
                }
            }
            else if(thisBoard.frequencyOn)
            {
                Serial.printf("Only frequency being optimized\n");

                if(thisBoard.redLEDOn)
                {
                    //strobeLED::redLED.power = wifiData.redLEDPower / 1000.0 * POWER_RESISTOR_VALUE;
                    setLEDPower(&strobeLED::redLED, wifiData.redLEDPower);
                    Serial.printf("Red LED Power: %lf\n", strobeLED::redLED.power); 
                }
                if(thisBoard.blueLEDOn)
                {
                    //strobeLED::blueLED.power = wifiData.blueLEDPower / 1000.0 * POWER_RESISTOR_VALUE;
                    setLEDPower(&strobeLED::blueLED, wifiData.blueLEDPower);
                    Serial.printf("Blue LED Power: %lf\n", strobeLED::blueLED.power); 
                }
            }
            Serial.printf("Now setting up board for parameter optimization\n");
            setupBoardForParameterOptimization();
        }
    }


    void createTasks()
    {
        if(boardTasks::thisBoard.manualMode)
        {
            //only tasks that are needed for manual mode go here
    
            // nothing more needed actually
            Serial.printf("Attaching Tasks for Manual Mode\n"); 
    
            if(boardTasks::thisBoard.redLEDOn)
            {
                xTaskCreatePinnedToCore(
                    ledTasks::toggleLED,      // Function that should be called
                    "Toggling red LED",             // Name of the task (for debugging)
                    5000,                            // Stack size (bytes)
                    &strobeLED::redLED,                           // Parameter to pass
                    7,                               // Task priority
                    &TaskHandlers::redLEDToggle, // Task handle
                    1                                // Pin to core 1
                );
    
                if(strobeLED::redLED.flashingEnabled)
                {
                    xTaskCreatePinnedToCore(
                        ledTasks::flashLED,      // Function that should be called
                        "Flashing red LED",             // Name of the task (for debugging)
                        5000,                            // Stack size (bytes)
                        &strobeLED::redLED,                           // Parameter to pass
                        7,                               // Task priority
                        &TaskHandlers::redLEDFlash, // Task handle
                        1                                // Pin to core 1
                    );
                }
                
            }
            
            if(boardTasks::thisBoard.blueLEDOn)
            {
                xTaskCreatePinnedToCore(
                    ledTasks::toggleLED,      // Function that should be called
                    "Toggling blue LED",             // Name of the task (for debugging)
                    5000,                            // Stack size (bytes)
                    &strobeLED::blueLED,                           // Parameter to pass
                    7,                               // Task priority
                    &TaskHandlers::blueLEDToggle, // Task handle
                    1                                // Pin to core 1
                );
    
                if(strobeLED::blueLED.flashingEnabled)
                {
                    xTaskCreatePinnedToCore(
                        ledTasks::flashLED,      // Function that should be called
                        "Flashing blue LED",             // Name of the task (for debugging)
                        5000,                            // Stack size (bytes)
                        &strobeLED::blueLED,                           // Parameter to pass
                        7,                               // Task priority
                        &TaskHandlers::blueLEDFlash, // Task handle
                        1                                // Pin to core 1
                    );
                }
                
            }
        }
        if(!boardTasks::thisBoard.manualMode)
        {
            Serial.printf("Attaching Tasks for Parameter Optimization Mode");
            if(boardTasks::thisBoard.redLEDOn)
            {
                xTaskCreatePinnedToCore(
                    ledTasks::updateFlyCountTask,      // Function that should be called
                    "Bite Count red LED",             // Name of the task (for debugging)
                    10000,                            // Stack size (bytes)
                    &strobeLED::redLED,                           // Parameter to pass
                    4,                               // Task priority
                    &TaskHandlers::getRedLEDFlyCount, // Task handle
                    1                                // Pin to core 0
                );

                xTaskCreatePinnedToCore(
                    filterTasks::initFilter,      // Function that should be called
                    "Init Filter for red LED",             // Name of the task (for debugging)
                    10000,                            // Stack size (bytes)
                    &dataSmoother::redLEDData,                           // Parameter to pass
                    4,                               // Task priority
                    &TaskHandlers::redMovingAverage, // Task handle
                    1                                // Pin to core 0
                );
        

                xTaskCreatePinnedToCore(
                    filterTasks::finiteDifferenceDerivative,           // Function that should be called
                    "FDD for redLED",               // Name of the task (for debugging)
                    6000,                                  // Stack size (bytes)
                    &filterTasks::redLEDFilter,                    // Parameter to pass
                    2,                                      // Task priority
                    &TaskHandlers::redFDDFilter,      // Task handle
                    0                                       // Pin to core 0
                );
                
                if(boardTasks::thisBoard.powerOn)
                {
                    xTaskCreatePinnedToCore(
                        filterTasks::highPassFilterTask,           // Function that should be called
                        "HighPass Power Filter for redLED",               // Name of the task (for debugging)
                        6000,                                  // Stack size (bytes)
                        &Filter::redLEDPowerFilter,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::redLEDPowerHighPass,      // Task handle
                        0                                       // Pin to core 0
                    );

                    xTaskCreatePinnedToCore(
                        ESCTasks::ESCCalculations,           // Function that should be called
                        "Power ESC for redLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &ESCTasks::redPower,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::redLEDPowerESC,      // Task handle
                        0                                       // Pin to core 0
                    );

                }

                if(boardTasks::thisBoard.frequencyOn)
                {
                    xTaskCreatePinnedToCore(
                        filterTasks::highPassFilterTask,           // Function that should be called
                        "HighPass Frequency Filter for redLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &Filter::redLEDFrequencyFilter,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::redLEDFrequencyHighPass,      // Task handle
                        0                                       // Pin to core 0
                    );

                    xTaskCreatePinnedToCore(
                        ESCTasks::ESCCalculations,           // Function that should be called
                        "Frequency ESC for redLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &ESCTasks::redFrequency,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::redLEDFrequencyESC,      // Task handle
                        0                                       // Pin to core 0
                    );
                }
        }
            
            if(boardTasks::thisBoard.blueLEDOn)
            {
                xTaskCreatePinnedToCore(
                    ledTasks::updateFlyCountTask,      // Function that should be called
                    "Bite Count Blue LED",             // Name of the task (for debugging)
                    10000,                            // Stack size (bytes)
                    &strobeLED::blueLED,                           // Parameter to pass
                    4,                               // Task priority
                    &TaskHandlers::getBlueLEDFlyCount, // Task handle
                    1                                // Pin to core 0
                );

                xTaskCreatePinnedToCore(
                    filterTasks::initFilter,      // Function that should be called
                    "Init Filter for blue LED",             // Name of the task (for debugging)
                    10000,                            // Stack size (bytes)
                    &dataSmoother::blueLEDData,                           // Parameter to pass
                    4,                               // Task priority
                    &TaskHandlers::blueMovingAverage, // Task handle
                    1                                // Pin to core 0
                );
        
                xTaskCreatePinnedToCore
                (
                    filterTasks::finiteDifferenceDerivative,           // Function that should be called
                    "FDD for blueLED",               // Name of the task (for debugging)
                    10000,                                  // Stack size (bytes)
                    &filterTasks::blueLEDFilter,                    // Parameter to pass
                    2,                                      // Task priority
                    &TaskHandlers::blueFDDFilter,      // Task handle
                    0                                       // Pin to core 0
                );

                

                if(boardTasks::thisBoard.powerOn)
                {
                    xTaskCreatePinnedToCore(
                        filterTasks::highPassFilterTask,           // Function that should be called
                        "HighPass Power Filter for blueLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &Filter::blueLEDPowerFilter,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::blueLEDPowerHighPass,      // Task handle
                        0                                       // Pin to core 0
                    );

                    xTaskCreatePinnedToCore(
                        ESCTasks::ESCCalculations,           // Function that should be called
                        "Power ESC for blueLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &ESCTasks::bluePower,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::blueLEDPowerESC,      // Task handle
                        0                                       // Pin to core 0
                    );

                }

                if(boardTasks::thisBoard.frequencyOn)
                {
                    xTaskCreatePinnedToCore(
                        filterTasks::highPassFilterTask,           // Function that should be called
                        "HighPass Frequency Filter for blueLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &Filter::blueLEDFrequencyFilter,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::blueLEDFrequencyHighPass,      // Task handle
                        0                                       // Pin to core 0
                    );

                    xTaskCreatePinnedToCore(
                        ESCTasks::ESCCalculations,           // Function that should be called
                        "Frequency ESC for blueLED",               // Name of the task (for debugging)
                        10000,                                  // Stack size (bytes)
                        &ESCTasks::blueFrequency,                    // Parameter to pass
                        2,                                      // Task priority
                        &TaskHandlers::blueLEDFrequencyESC,      // Task handle
                        0                                       // Pin to core 0
                    );
                }
                
            }
        }
    }

}