
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>
#include <bricks/DAC_handler.h>

/**
 * The functions here handle turning the LEDs on/off at the specific frequency with the specific power
*/




namespace strobeLED
{
    TwoWire redLEDBus = TwoWire(0);
    TwoWire blueLEDBus = TwoWire(1);

    SemaphoreHandle_t redLEDSemaphore;
    SemaphoreHandle_t blueLEDSemaphore;

    int redLEDFlyCount;
    int blueLEDFlyCount;

    void redLEDOnCallback(TimerHandle_t xhandle);
    void blueLEDOnCallback(TimerHandle_t xhandle);


    class LED
    {
        private:

            int timerNum;
            int gpio;
            TimerCallbackFunction_t interruptFunction;
            void attachTimer();
        
        public:
            TimerHandle_t timerHandle; 
            TwoWire* busptr;
            volatile float period_us; // stores flashing frequency
            volatile float power;    // stores power 
            volatile bool state;
            bool ledOn;
            bool flashingEnabled = true; // default set to true
            volatile unsigned ledFlyCount;
            volatile SemaphoreHandle_t ledSemaphore = NULL; 
            volatile SemaphoreHandle_t onSemaphore = NULL; 
            volatile SemaphoreHandle_t offSemaphore = NULL; 
            volatile SemaphoreHandle_t onFlashSemaphore = NULL; 
            volatile SemaphoreHandle_t offFlashSemaphore = NULL; 
            
            LED(TwoWire* ledptr, float period_us, float power, bool state);

            void trigger();
            void deTrigger();
            void cancel();

    };

    /**
     * @brief constructor for new LED object
     * @param busptr pointer to DAC bus that we want to control
     * @param period_us chosen flashing period in microseconds (us)
     * @param power chosen power through LED
     * @param state led on or off
    */
    LED::LED(TwoWire* ledptr, float period_us, float power, bool state)
    {
        this->busptr = ledptr;
        this->ledOn = false;
        if(ledptr == &redLEDBus)
        { 
            this->interruptFunction = &redLEDOnCallback;
            this->timerNum = 0;
        }
        else if(ledptr == &blueLEDBus)
        {
            this->interruptFunction = &blueLEDOnCallback;
            this->timerNum = 2;
        }
        else{
            Serial.print("LED BUS DOESNT EXIST!");
            while(1); // something went wrong...
        }

        this->period_us = period_us;
        this->power = power;
        this->state = state;
        this->timerHandle = NULL;
        this->ledSemaphore = xSemaphoreCreateBinary();
        this->onSemaphore = xSemaphoreCreateBinary();
        this->offSemaphore = xSemaphoreCreateBinary();
        this->onFlashSemaphore = xSemaphoreCreateBinary();
        this->offFlashSemaphore = xSemaphoreCreateBinary();
        attachTimer();
    }

    /**
     * @brief Ends timer and turns LED off
    */
    void LED::cancel()
    {
        xTimerStop(this->timerHandle, (TickType_t) 10);
        xTimerDelete(this->timerHandle, (TickType_t) 10);
    }
    
    /**
     * @brief attaches interrrupt to timer
    */
    void LED::attachTimer()
    {
        this->timerHandle = xTimerCreate("redLED", pdMS_TO_TICKS(this->period_us/1000.0), pdTRUE, nullptr, this->interruptFunction);
    }
    
   /**
     * @brief starts timer for corresponding ledBus
    */
    void LED::trigger()
    {

        float period_ms = this->period_us / 1000.0;
        xTimerChangePeriod(this->timerHandle, pdMS_TO_TICKS(period_ms), (TickType_t) 3);
        xTimerStart(this->timerHandle, (TickType_t) 3);
        xSemaphoreGive(this->onSemaphore); 
    }

    void LED::deTrigger()
    {
        //this->state = false;

        xTimerStop(this->timerHandle, (TickType_t) 3);
        xSemaphoreGive(this->ledSemaphore); //unblock task with this function
        xSemaphoreGive(this->offSemaphore);
        xSemaphoreTake(this->onSemaphore, (TickType_t) 1); // taking remaining semaphore
    }


    LED redLED(&redLEDBus, DEFAULT_LOW_PERIOD_US, 1, true); 
    LED blueLED(&blueLEDBus, DEFAULT_LOW_PERIOD_US, 1, true); 

    /**
     * @brief callback function for turning red LED on/off with timer
    */
    void redLEDOnCallback(TimerHandle_t xhandle)
    {
        redLED.state ? xSemaphoreGive(redLED.onSemaphore) : xSemaphoreGive(redLED.offSemaphore);
        redLED.state = !redLED.state;
    }

    /**
     * @brief callback function for turning blue LED on/off with timer
    */
    void blueLEDOnCallback(TimerHandle_t xhandle)
    {
        blueLED.state ? xSemaphoreGive(blueLED.onSemaphore) : xSemaphoreGive(blueLED.offSemaphore);
        blueLED.state = !blueLED.state;
    }

    /**
     * @brief this is the red led HARDWARE INTERRUPT function.  
     * It will be used to trigger the TIMER function
     * it identifies if the change in RED interrupt pin was:
     * HIGH, which means fly landed and we should flash led or LOW
     * LOW, which means fly left and we should turn LED off
    */
    void IRAM_ATTR changeRedLEDFlash()
    {
        digitalRead(RED_INT) ?  xSemaphoreGiveFromISR(redLED.onFlashSemaphore, NULL) : xSemaphoreGiveFromISR(redLED.offFlashSemaphore,NULL);
    }

    /**
     * @brief this is the blue led HARDWARE INTERRUPT function.
     * it will be used to trigger the TIMER function
     * it identifies if the change in BLUE interrupt pin was:
     * HIGH, which means fly landed and we should flash led or LOW
     * LOW, which means fly left and we should turn LED off
    */
    void IRAM_ATTR changeBlueLEDFlash()
    {
        digitalRead(BLUE_INT) ?  xSemaphoreGiveFromISR(blueLED.onFlashSemaphore, NULL) : xSemaphoreGiveFromISR(blueLED.offFlashSemaphore,NULL);
    }

    void IRAM_ATTR changeRedLED()
    {

        if(digitalRead(RED_INT))
        {
            xSemaphoreGiveFromISR(redLED.onSemaphore, NULL); //unblock task with this function
            xSemaphoreGiveFromISR(redLED.ledSemaphore,NULL); //unblock task with this function
            redLED.ledFlyCount++;
        }
        else
        {
            xSemaphoreGiveFromISR(redLED.offSemaphore, NULL); //unblock task with this function
        }
    }

    void IRAM_ATTR changeBlueLED()
    {

        if(digitalRead(BLUE_INT))
        {
            xSemaphoreGiveFromISR(blueLED.onSemaphore, NULL); //unblock task with this function
            xSemaphoreGiveFromISR(blueLED.ledSemaphore, NULL); //unblock task with this function
            blueLED.ledFlyCount++;
        }
        else
        {
            xSemaphoreGiveFromISR(blueLED.offSemaphore, NULL); //unblock task with this function
        }
    }

}