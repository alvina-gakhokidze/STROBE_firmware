// here we will have the tasks related to controlling the power through the LED

#include <bricks/ESC_handler.h>


namespace ESCTasks
{

    typedef struct escStruct
    {
        Controllers::ESC* ledESC;
        volatile int* perturbationSignalSign;

    } escStruct;

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


        for(;;)
        { // are we having this endlessly execute ? not a good idea
            int perturbationSignalSign = *(localStruct->perturbationSignalSign);

            // get value from high pass filter
            float filteredValue = localESC->filterObject->filteredValue;

            //demodulate value
            float demodOutput = filteredValue * perturbationSignalSign * localESC->modAmplitude;
            
            //store old value

            localESC->pastModVal = localESC->curModVal;

            localESC->curModVal = demodOutput;

            // integrate modulated values

            float intOutput = Filter::calcIntegral(localESC->pastModVal, localESC->curModVal, 0.0, (MAF_PERIOD_MS/1000.0));

            // multiply by loop gain
            intOutput *= localESC->loopGain;

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

            unsigned long controllerOutput = intOutput + ( perturbationSignalSign * localESC->modAmplitude );
            
            // pass value to LED struct so that timers can accurately turn LED on/off with desired output power/frequency
            
            bool powerESC = localESC->powerESC;

            if(powerESC)
            {
                localESC->LEDObject->power = controllerOutput;
            }
            else
            {
                localESC->LEDObject->period_us = controllerOutput;
            }

            vTaskDelay((TickType_t) pdMS_TO_TICKS(5)); // we want to perform this approx every 100Hz (10ms). So block for half that time (5ms)
        }
    }

}