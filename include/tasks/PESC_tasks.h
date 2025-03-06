// here we will have the tasks related to controlling the power through the LED
#pragma once
#include <bricks/ESC_handler.h>


namespace ESCTasks
{

    struct escStruct
    {
        Controllers::ESC* ledESC;
        volatile int* perturbationSignalSign;
    };

    escStruct redPower = 
    {
        &Controllers::redPESC, 
        &Controllers::powerPerturbationSignalSign
    };

    escStruct bluePower = 
    {
        &Controllers::bluePESC,
        &Controllers::powerPerturbationSignalSign
    };

    escStruct redFrequency = 
    {
        &Controllers::redFESC,
        &Controllers::frequencyPerturbationSignalSign
    };

    escStruct blueFrequency = 
    {
        &Controllers::blueFESC,
        &Controllers::frequencyPerturbationSignalSign
    };


    /**
     * @brief triggers timer to begin perturbation signal sign changes
     */
    void escInit(void* escStruct)
    {
        Serial.printf("initiating ESC\n");
        ESCTasks::escStruct* localStruct = (ESCTasks::escStruct*) escStruct;
        Controllers::ESC* localESC = (Controllers::ESC*) localStruct->ledESC;

        localESC->setupTimer(); // begin timer signals
    }

    /**
     * @brief Task where feedback loop of the power controller will occur, perfoming demodulation, integration, and then modulation
     */
    void ESCCalculations(void* escStruct)
    {
        // i think we want new power calculations to be done whenever we have new data from the high pass filter
        ESCTasks::escStruct* localStruct = (ESCTasks::escStruct*) escStruct;

        Controllers::ESC* localESC = (Controllers::ESC*) localStruct->ledESC;
        Serial.printf("Entering ESC Task\n");

        for(;;)
        { // are we having this endlessly execute ? not a good idea
            int perturbationSignalSign = *(localStruct->perturbationSignalSign);

            //Serial.printf("pert sig: %d\n", perturbationSignalSign);

            // get value from high pass filter
            float filteredValue = localESC->filterObject->filteredValue;
            //Serial.printf("filt val: %lf\n", filteredValue);

            //demodulate value
            float demodOutput = filteredValue * (float) perturbationSignalSign * localESC->modAmplitude;

            //Serial.printf("demodOutput: %lf\n", demodOutput);

            //store old value

            localESC->pastModVal = localESC->curModVal;

            localESC->curModVal = demodOutput;

            // integrate modulated values

            float intOutput = Filter::calcIntegral(localESC->pastModVal, localESC->curModVal, 0.0, (MAF_PERIOD_MS/1000.0));


            //Serial.printf("past: %lf, cur: %lf, int output: %lf\n", localESC->pastModVal, localESC->curModVal, intOutput);
            

            // multiply by loop gain
            intOutput *= localESC->loopGain;

            //Serial.printf("gain output: %lf\n", localESC->pastModVal, localESC->curModVal, intOutput);


            //apply saturation limits

            if(intOutput > SAT_UPP_LIMIT)
            {
                intOutput = SAT_UPP_LIMIT;
            }
            else if (intOutput < SAT_LOW_LIMIT)
            {
                intOutput = SAT_LOW_LIMIT;
            }

            // modulate that value

            float controllerOutput = intOutput + ( (float) perturbationSignalSign * localESC->modAmplitude );
            
            // pass value to LED struct so that timers can accurately turn LED on/off with desired output power/frequency
            
            bool powerESC = localESC->powerESC;

            if(powerESC)
            {
                localESC->LEDObject->power = controllerOutput;
                //Serial.printf("power output: %lf\n ", controllerOutput);
            }
            else
            {
                localESC->LEDObject->period_us = controllerOutput;
            }

            vTaskDelay((TickType_t) pdMS_TO_TICKS(5)); // we want to perform this approx every 100Hz (10ms). So block for half that time (5ms)
        }
    }

}