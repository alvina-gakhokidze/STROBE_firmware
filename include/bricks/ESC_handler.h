
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/ESC_config.h>
#include <bricks/Derivative_handler.h>
#include <bricks/LED_handler.h>


// here we will store the timers that handle the perturbation signal (that will alternate every 50 seconds)


namespace Controllers
{

    volatile int powerPerturbationSignalSign; // need to write timer for this that changes the state based on the period
    volatile int frequencyPerturbationSignalSign;

    void powerESCCallback(TimerHandle_t xTimer);
    void frequencyESCCallback(TimerHandle_t xTimer);

    class ESC
    {
        // so we need data to store past and current derivative values
        // need data to store past and current integral values
        private:
            const char* name;

        public: 
         
            float loopGain;
            float maxSat = SAT_UPP_LIMIT ;
            float minSat = SAT_LOW_LIMIT;
            float modAmplitude;
            float modPeriod;
            float pastModVal = 0.0; // previous modulated value for integral
            float curModVal = 0.0;  // new/current modulated value for integral
            bool powerESC; // if true, this is powerESC, if false it's frequency ESC


            Filter::HighPass* filterObject;
            strobeLED::LED* LEDObject;
            SemaphoreHandle_t escSemaphore = NULL;
            TimerHandle_t timerHandle;
            TimerCallbackFunction_t perturbationCallback;

            ESC
            ( 
                const char* escName, 
                float gain, float amplitude, 
                float period, 
                Filter::HighPass* LEDFilter, 
                strobeLED::LED* LEDType, 
                bool ESCType
            );

            bool setupTimer();
            bool endTimer();
    };

    ESC::ESC(const char* escName, float loopGain, float amplitude, float period, Filter::HighPass* LEDFilter, strobeLED::LED* LEDType, bool ESCType)
    {
        this->name = escName;
        this->loopGain = loopGain;
        this->modAmplitude = modAmplitude;
        this->modPeriod = period;
        this->escSemaphore = xSemaphoreCreateBinary();
        this->filterObject = LEDFilter;
        this->LEDObject = LEDType;
        this->powerESC = ESCType;

        if(this-> name == "redPower" || this->name == "bluePower")
        {
            this->perturbationCallback = powerESCCallback;
        }
        else if (this->name == "redFrequency" || this->name == "blueFrequency")
        {
            this->perturbationCallback = frequencyESCCallback;
        }
        else
        {
            Serial.println("WRONG ESC NAME GIVEN TO TIMER!");
            while(1);
        }

        this->timerHandle = xTimerCreate(this->name, pdMS_TO_TICKS(period * 1000.0), pdTRUE, nullptr, this->perturbationCallback);    
        if(timerHandle == NULL)
        {
            Serial.printf("ESC TIMER FOR %s NOT CREATED!", name);
        }
    }

    /**
     * @brief start timer that changes sign of perturbation signal
     */
    bool ESC::setupTimer()
    {
        if(xTimerStart( this->timerHandle , (TickType_t) 10 ) == pdFAIL ) return false;
        return true;
    }

    /**
     * @brief end timer that changes sign of perturbation signal
     */
    bool ESC::endTimer()
    {
        xTimerStop( this->timerHandle, (TickType_t) 10 );
        xTimerDelete( this->timerHandle, (TickType_t) 10 );
    }

    ESC redPESC
    (
        "redPower",
        POWER_GAIN, 
        POWER_PERTURBATION_AMPLITUDE, 
        POWER_PERTURBATION_PERIOD, 
        &Filter::redLEDPowerFilter,
        &strobeLED::redLED, true
    );

    ESC bluePESC
    (
        "bluePower",
        POWER_GAIN, 
        POWER_PERTURBATION_AMPLITUDE, 
        POWER_PERTURBATION_PERIOD, 
        &Filter::blueLEDPowerFilter, 
        &strobeLED::blueLED, true
    );

    ESC redFESC
    (
        "redFrequency",
        FREQUENCY_GAIN, 
        FREQUENCY_PERTURBATION_AMPLITUDE, 
        FREQUENCY_PERTURBATION_PERIOD, 
        NULL, 
        &strobeLED::redLED, 
        false
    ); // change pointer after ALVINA

    ESC blueFESC
    (
        "blueFrequency",
        FREQUENCY_GAIN, 
        FREQUENCY_PERTURBATION_AMPLITUDE, 
        FREQUENCY_PERTURBATION_PERIOD, 
        NULL, 
        &strobeLED::blueLED, 
        false
    ); // change pointer after ALVINA


    /**
     * @brief chnages sign of power perturbation signal
     */
    void powerESCCallback(TimerHandle_t xTimer)
    {
        Controllers::powerPerturbationSignalSign *= (-1); // sign change
    }

    /**
     * @brief changes sign of frequency perturbation signal
     */
    void frequencyESCCallback(TimerHandle_t xTimer)
    {
        Controllers::frequencyPerturbationSignalSign *= (-1); // sign change
    }

}

// so for timers we need callback functions

// and these callback functions cannot have inputs

// but we need the function to change the sign of a variable that determines the state of the perturbation signal.

// so i ownder if we need to write four callback functions...

// or should we only write two callbacks, one for power one for frequency. Then the state of each LED will change simultanously, 
// but i thinkk that hsould be ok becuase they'll both be working at the same time anyway (red and blue) if at all



