#pragma once
#include <configs/SYSTEM_config.h>

#include <bricks/DAC_handler.h>
#include <bricks/Derivative_handler.h>
#include <bricks/ESC_handler.h>
#include <bricks/LED_handler.h>
#include <bricks/Smoothing_handler.h>
#include <tasks/PESC_tasks.h>


namespace boardTasks
{

    Preferences permanentMemory; // will be used to store wifi data, and also red and blue LED configs

    typedef struct boardParameters{
        bool redLEDOn;
        bool blueLEDOn;
        bool powerOn;
        bool frequencyOn;
        bool manualMode;
        bool optimizationMode;
    }; 

    boardParameters thisBoard = {false, false, false, false, true, false};

    /**
     * @brief sets up LEDs and DACs on the board. Should be done last because this 
     * starts the LED hardware interrupts.
     */
    bool setupPeripherals()
    {
        strobeLED::setupLEDs();
        registerTalk::setupDAC(&strobeLED::redLEDBus);
        registerTalk::setupDAC(&strobeLED::blueLEDBus);
    }

    /**
     * @brief enables smoothing, filtering, and the associated ESCs
     */
    bool setupBoardForParameterOptimization()
    {
        // need a mode for enabling only certain LEDs

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
     * @brief enables board operation for manually-chosen power and frequency
     */
    void setupLEDsForManualOperation(strobeLED::LED* ledObject , unsigned long period_us, unsigned long power)
    {
        ledObject->period_us = period_us;
        ledObject->power = power;

        setupPeripherals();
    }


    /**
     * @brief determines which LEDs user wants to use, and what mode (manual/optimization)
     */
    void setupBoard()
    {

        Serial.printf("Welcome to the new STROBE!\n");

        const char* ledChoice = "Would you like to use:\n 1. Red LED \n 2. Blue LED \n 3. Both LEDs? \n 4. Neither LED \n";

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
                Serial.printf("Manual Mode Chosen\n");

                thisBoard.manualMode = true;

                if(thisBoard.redLEDOn)
                {
                    Serial.printf("Time to choose the parameters for the red LED! \n");
                    calibrateLED(&strobeLED::redLED, "power");
                    calibrateLED(&strobeLED::redLED, "frequency");
                }

                if(thisBoard.blueLEDOn)
                {
                    Serial.printf("Time to choose the parameters for the blue LED! \n");
                    calibrateLED(&strobeLED::blueLED, "power");
                    calibrateLED(&strobeLED::blueLED, "frequency");
                }
                setupPeripherals();
            case 2:
                Serial.printf("Parameter Optimization Mode Chosen");
                thisBoard.optimizationMode = true;

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
                            calibrateLED(&strobeLED::redLED, "frequency");
                        }
                        if(thisBoard.blueLEDOn)
                        {
                            Serial.printf("Calibrating Blue LED. \n");
                            calibrateLED(&strobeLED::blueLED, "frequency");
                        }
                    case 2:
                        Serial.printf("Only Frequency being optimized.\n");
                        thisBoard.frequencyOn = true;
                        thisBoard.powerOn = false;
                        if(thisBoard.redLEDOn)
                        {
                            Serial.printf("Calibrating Red LED. \n");
                            calibrateLED(&strobeLED::redLED, "power");
                        }
                        if(thisBoard.blueLEDOn)
                        {
                            Serial.printf("Calibrating Blue LED. \n");
                            calibrateLED(&strobeLED::blueLED, "power");
                        }
                    case 3:
                        Serial.printf("Both Power and Frequency being optimized.\n");
                        thisBoard.powerOn = true;
                        thisBoard.frequencyOn = true;
                        //dont need to do anything else here
                }

                setupBoardForParameterOptimization();
            default:   
                Serial.printf("Invalid Choice. Something wrong with choosing mode! \n");
                while(1);
        }
    }

    /**
     * @brief read Serial input from user, keep asking until valid option chosen
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
                Serial.printf("Value too Low! Cannot use value higher than %.0f. Try Again\n", lowLimit);
            }
            else if(userInput > upLimit)
            {
                Serial.printf("Value too high! Cannot use value higher than: %.0f. Try again \n", upLimit);
            }
        }

        //exit once valid option has been chosen

        return userInput;
    }

    /**
     * @brief function that prompts user to provide LED current and LED frequency for manual operation
     */
    void calibrateLED(strobeLED::LED* localLED, char* mode)
    {
        const char* currentChoice = "Provide the LED current in mA: \n";
        const char* frequencyChoice = "Provide the LED frequency in Hz: \n";

        if(mode == "power"){
            float ledCurrent = getUserFloatInput(currentChoice, CURR_UPP_LIMIT, CURR_LOW_LIMIT);

            // converting it to voltage that will be applied across the resistor to create desired current
            localLED->power = ledCurrent / 1000.0 * POWER_RESISTOR_VALUE; // V = I * R formula
        }

        if(mode == "frequency")
        {
            float ledFrequency = getUserFloatInput(frequencyChoice, FREQ_LOW_LIMIT, FREQ_UPP_LIMIT);
            if(ledFrequency == 0.0)
            {
                if(localLED == &strobeLED::redLED)
                {
                    strobeLED::NO_RED_FLASHING = true;
                }
                else
                {
                    strobeLED::NO_BLUE_FLASHING = false;
                }
            }
            else
            {
                localLED->period_us =  ( 1.0 / ledFrequency ) * 1000000.0 ;
            }
        }
    }

    /**
     * @brief if no input from user, we will load 
     * @return true if specifiying new parameters, false if we want to load old ones
     */
    bool determineUserOrEEPROM()
    {
        Serial.printf("Would you like to specify new experiment parameters? Y/N\n");
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
                return true;
            }
            
            Serial.printf("Using previous experiment configuration\n");
            return false;
        }
    }

    /**
     * @brief save the values to permanent memory so that upon restart we don't need user input again
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

    void saveToPermanentMemoryULong(Preferences* memory, char* keyName, unsigned long keyVal)
    {
        memory->begin(keyName, false);
        memory->putULong(keyName, keyVal);
        memory->end();
    }   

    unsigned long getFromPermanentMemoryULong(Preferences* memory, char* keyName)
    {
        memory->begin(keyName, false);
        unsigned long val = memory->getULong(keyName);
        memory->end();
        
        return val;
    }

    void saveBoardConfigs()
    {
        //the keyNames are limited to 15 chars

        // experiment parameters
        saveToPermanentMemoryBool(&permanentMemory, "redLEDOn", thisBoard.redLEDOn);
        saveToPermanentMemoryBool(&permanentMemory, "blueLEDOn", thisBoard.blueLEDOn);
        saveToPermanentMemoryBool(&permanentMemory, "powerOn", thisBoard.powerOn);
        saveToPermanentMemoryBool(&permanentMemory, "frequencyOn", thisBoard.frequencyOn);
        saveToPermanentMemoryBool(&permanentMemory, "manualMode", thisBoard.manualMode);
        saveToPermanentMemoryBool(&permanentMemory, "optMode", thisBoard.optimizationMode);
        
        //LED power and frequency values
        saveToPermanentMemoryULong(&permanentMemory, "redLEDPower", strobeLED::redLED.power);
        saveToPermanentMemoryULong(&permanentMemory, "redLEDPeriod", strobeLED::redLED.period_us);

        saveToPermanentMemoryULong(&permanentMemory, "blueLEDPower", strobeLED::blueLED.power);
        saveToPermanentMemoryULong(&permanentMemory, "blueLEDPeriod", strobeLED::blueLED.period_us);
    }

    void loadBoardConfigs()
    {
        thisBoard.redLEDOn = getFromPermanentMemoryBool(&permanentMemory, "redLEDOn");
        thisBoard.blueLEDOn = getFromPermanentMemoryBool(&permanentMemory, "blueLEDOn");
        thisBoard.powerOn = getFromPermanentMemoryBool(&permanentMemory, "powerOn");
        thisBoard.frequencyOn = getFromPermanentMemoryBool(&permanentMemory, "frequencyOn");
        thisBoard.manualMode = getFromPermanentMemoryBool(&permanentMemory, "manualMode");
        thisBoard.optimizationMode = getFromPermanentMemoryBool(&permanentMemory, "optMode");

        strobeLED::redLED.power = getFromPermanentMemoryULong(&permanentMemory, "redLEDPower");
        strobeLED::redLED.period_us = getFromPermanentMemoryULong(&permanentMemory, "redLEDPeriod");

        strobeLED::blueLED.power = getFromPermanentMemoryULong(&permanentMemory, "blueLEDPower");
        strobeLED::blueLED.period_us = getFromPermanentMemoryULong(&permanentMemory, "blueLEDPeriod");
    }

}