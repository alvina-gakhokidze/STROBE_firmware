
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/ESC_config.h>
#include <bricks/Derivative_handler.h>
#include <bricks/LED_handler.h>


// here we will store the timers that handle the perturbation signal (that will alternate every 50 seconds)



namespace Controllers
{

    volatile int perturbationSignalSign; // need to write timer for this that changes the state based on the period

    class ESC
    {
        // so we need data to store past and current derivative values
        // need data to store past and current integral values

        public: 
            float loopGain;
            float maxSat = SAT_UPP_LIMIT ;
            float minSat = SAT_LOW_LIMIT;
            float modAmplitude;
            float pastModVal = 0.0; // previous modulated value for integral
            float curModVal = 0.0;  // new/current modulated value for integral
            bool powerESC; // if true, this is powerESC, if false it's frequency ESC
            Filter::HighPass* filterObject;
            strobeLED::LED* LEDObject;
            SemaphoreHandle_t escSemaphore = NULL;
            ESC(float gain, float amplitude, Filter::HighPass* LEDFilter, strobeLED::LED* LEDType, bool ESCType);
    };

    ESC::ESC(float gGain, float amplitude, Filter::HighPass* LEDFilter, strobeLED::LED* LEDType, bool ESCType)
    {
        this->loopGain = loopGain;
        this->modAmplitude = modAmplitude;
        this->escSemaphore = xSemaphoreCreateBinary();
        this->filterObject = LEDFilter;
        this->LEDObject = LEDType;
        this->powerESC = ESCType;

    }

    ESC redPESC(POWER_GAIN, POWER_PERTURBATION_AMPLITUDE, &Filter::redLEDPowerFilter, &strobeLED::redLED, true);
    ESC bluePESC(POWER_GAIN, POWER_PERTURBATION_AMPLITUDE, &Filter::blueLEDPowerFilter, &strobeLED::blueLED, true);

    ESC redFESC(FREQUENCY_GAIN, FREQUENCY_PERTURBATION_AMPLITUDE, NULL, &strobeLED::redLED, false); // change pointer after ALVINA
    ESC blueFESC(FREQUENCY_GAIN, FREQUENCY_PERTURBATION_AMPLITUDE, NULL, &strobeLED::blueLED, false); // change pointer after ALVINA





}