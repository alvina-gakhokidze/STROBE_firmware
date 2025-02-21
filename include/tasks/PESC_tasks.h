// here we will have the tasks related to controlling the power through the LED

#include <bricks/ESC_handler.h>


namespace PowerTasks
{

    /**
     * @brief Task where feedback loop of the power controller will occur, perfoming demodulation, integration, and then modulation
     */
    void powerCalculations(void* esc)
    {

        Controllers::ESC* localESC = (Controllers::ESC*) esc;

        // get value from high pass filter
        float filteredValue = localESC->filterObject->filteredValue;

        //demodulate value
        float demodOutput = filteredValue * Controllers::perturbationSignalSign * localESC->modAmplitude;
        
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

        float controllerOutput = intOutput + ( Controllers::perturbationSignalSign * localESC->modAmplitude );
        
        // pass value to LED struct so that timers can accurately turn LED on/off with desired output power/frequency
        
        bool powerESC = localESC->powerESC;

        if(powerESC)
        {
            localESC->LEDObject->power = controllerOutput;
        }
        else{
            localESC->LEDObject->period_us = controllerOutput;
        }

    }
}