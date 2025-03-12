
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



    void IRAM_ATTR redLEDOnCallback();
    void IRAM_ATTR blueLEDOnCallback();


    class LED
    {
        private:

            int timerNum;
            int gpio;
            void (*interruptFunction)(void); // not entirely sure that this will work but let's give it a try
            void attachTimer();
        
        public:
            hw_timer_t* timerHandle;
            TwoWire* busptr;
            volatile float period_us; // stores flashing frequency
            volatile float power;    // stores power 
            volatile bool state;
            bool ledOn;
            bool flashingEnabled = true; // default set to true
            volatile unsigned ledFlyCount;
            SemaphoreHandle_t ledSemaphore = NULL; // hopefully it's overwritten later
            SemaphoreHandle_t onSemaphore = NULL; // hopefully it's overwritten later
            SemaphoreHandle_t offSemaphore = NULL; // hopefully it's overwritten later
            SemaphoreHandle_t onFlashSemaphore = NULL; // hopefully it's overwritten later
            SemaphoreHandle_t offFlashSemaphore = NULL; // hopefully it's overwritten later
            
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
        this->timerHandle = timerBegin(this->timerNum, 80, true);
        this->ledSemaphore = xSemaphoreCreateBinary();
        this->onSemaphore = xSemaphoreCreateBinary();
        this->offSemaphore = xSemaphoreCreateBinary();
        this->onFlashSemaphore = xSemaphoreCreateBinary();
        this->offFlashSemaphore = xSemaphoreCreateBinary();
        attachTimer(); // can i do this? use a method inside of the class constructor
    }

    /**
     * @brief Ends timer and turns LED off
    */
    void LED::cancel()
    {
        timerEnd(this->timerHandle); // do we ever actually need to do this?
        //registerTalk::ledOff(this->busptr);
    }
    
    /**
     * @brief attaches interrrupt to timer
    */
    void LED::attachTimer()
    {
        timerAttachInterrupt(this->timerHandle, this->interruptFunction, true);
    }
    
   /**
     * @brief starts timer for corresponding ledBus
    */
    void LED::trigger()
    {

        //this->state = true; // because it was triggered
        // so that when redLED calback is called, we enter TRUE

        //digitalWrite(DEBUG_LED,HIGH);

        timerAlarmWrite(this->timerHandle, this->period_us, true); // have to write a new alarm each time because the period_us will change
        // OOLVOONOO is it correct to feed the period in microseconds? i dont think so
        timerAlarmEnable(this->timerHandle);
        
        
        // SEMAPHORE TAKE? i dont think so
        this->ledFlyCount++;
        //digitalWrite(DEBUG_LED,HIGH); 
        
        this->ledOn = true;
        
        
    }

    void LED::deTrigger()
    {
        //this->state = false;

        timerAlarmDisable(this->timerHandle);
        xSemaphoreGive(this->ledSemaphore); //unblock task with this function
        // this semaphore wouldn't give when we were doing trigger() for some reason
       
        xSemaphoreGive(this->offSemaphore);

        this->ledOn=false;
        //digitalWrite(DEBUG_LED,LOW);

       
        
        //registerTalk::ledOff(this->busptr);

        //digitalWrite(DEBUG_LED, LOW);

        // xSemaphoreTake(this->onSemaphore, (TickType_t) 1);
        // xSemaphoreTake(this->onFlashSemaphore, (TickType_t) 1);
       
        //registerTalk::ledOff(this->busptr); 
    }


    LED redLED(&redLEDBus, DAFAULT_LOW_PERIOD_US, 1, true); //OOLVOONOO set initial period to something really large
    LED blueLED(&blueLEDBus, DAFAULT_LOW_PERIOD_US, 1, true); //OOLVOONOO set initial period to something really large

    /**
     * @brief callback function for turning red LED on/off with timer
    */
    void redLEDOnCallback()
    {
        // redLED.state ? registerTalk::ledControlOn(redLED.busptr, redLED.power) : registerTalk::ledOff(redLED.busptr);
        // redLED.state = !redLED.state;
        //digitalWrite(DEBUG_LED, LOW);
        redLED.state ? xSemaphoreGive(redLED.onSemaphore) : xSemaphoreGive(redLED.offSemaphore);
        //digitalWrite(DEBUG_LED, redLED.state);
        redLED.state = !redLED.state;
        

        // redLED.state ? digitalWrite(DEBUG_LED,HIGH) : digitalWrite(DEBUG_LED,LOW);
        // redLED.state = !redLED.state;
    }

    /**
     * @brief callback function for turning blue LED on/off with timer
    */
    void blueLEDOnCallback()
    {
        // blueLED.state ? registerTalk::ledControlOn(blueLED.busptr, blueLED.power) : registerTalk::ledOff(blueLED.busptr);
        // blueLED.state = !blueLED.state;

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
        //digitalRead(RED_INT) ? digitalWrite(DEBUG_LED,HIGH) : digitalWrite(DEBUG_LED, LOW);
        //digitalRead(RED_INT) ?  redLED.trigger() : redLED.deTrigger();
        digitalRead(RED_INT) ?  xSemaphoreGiveFromISR(redLED.onFlashSemaphore, NULL) : xSemaphoreGiveFromISR(redLED.offFlashSemaphore,NULL);
        //digitalRead(RED_INT) ? digitalWrite(DEBUG_LED, HIGH) : digitalWrite(DEBUG_LED, LOW);

        // digitalRead(RED_INT) ?  digitalWrite(DEBUG_LED, HIGH) : digitalWrite(DEBUG_LED, LOW);
        
        // digitalRead(RED_INT) ? redLED.ledOn = true : redLED.ledOn = false;

        // so we know we enter this
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
        //digitalRead(BLUE_INT) ?  blueLED.trigger() : blueLED.deTrigger();
        digitalRead(BLUE_INT) ?  xSemaphoreGiveFromISR(blueLED.onFlashSemaphore, NULL) : xSemaphoreGiveFromISR(blueLED.offFlashSemaphore,NULL);
    }

    void IRAM_ATTR changeRedLED()
    {

        if(digitalRead(RED_INT))
        {
            // registerTalk::ledControlOn(redLED.busptr, redLED.power);
            // redLED.ledFlyCount++; // increment total number of interactions
            // xSemaphoreGiveFromISR(redLED.ledSemaphore, NULL); //unblock task with this function
            redLED.ledOn = true;
            xSemaphoreGiveFromISR(redLED.onSemaphore, NULL); //unblock task with this function
            xSemaphoreGiveFromISR(redLED.ledSemaphore,NULL); //unblock task with this function
            redLED.ledFlyCount++;
        }
        else
        {
            //registerTalk::ledOff(redLED.busptr);
            xSemaphoreGiveFromISR(redLED.offSemaphore, NULL); //unblock task with this function
        }
    }

    void IRAM_ATTR changeBlueLED()
    {

        if(digitalRead(BLUE_INT))
        {
            //registerTalk::ledControlOn(blueLED.busptr, blueLED.power);
            //blueLED.ledFlyCount++;
            //xSemaphoreGiveFromISR(blueLED.ledSemaphore, NULL); //unblock task with this function
            blueLED.ledOn = true;
            xSemaphoreGiveFromISR(blueLED.onSemaphore, NULL); //unblock task with this function
            xSemaphoreGiveFromISR(blueLED.ledSemaphore, NULL); //unblock task with this function
            blueLED.ledFlyCount++;
        }
        else
        {
            //registerTalk::ledOff(blueLED.busptr);
            xSemaphoreGiveFromISR(blueLED.offSemaphore, NULL); //unblock task with this function
        }
    }

}