
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
            bool flashingEnabled = true; // default set to true
            volatile unsigned ledFlyCount;
            SemaphoreHandle_t ledSemaphore = NULL; // hopefully it's overwritten later
            
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
        if(ledptr == &redLEDBus)
        { 
            //redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
            this->interruptFunction = &redLEDOnCallback;
            this->timerNum = 0;
        }
        else if(ledptr == &blueLEDBus)
        {
            //blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
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
        attachTimer(); // can i do this? use a method inside of the class constructor
    }

    /**
     * @brief Ends timer and turns LED off
    */
    void LED::cancel()
    {
        timerEnd(this->timerHandle); // do we ever actually need to do this?
        registerTalk::ledOff(this->busptr);
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
        timerAlarmWrite(this->timerHandle, this->period_us, true); // have to write a new alarm each time because the period_us will change
        // OOLVOONOO is it correct to feed the period in microseconds? i dont think so
        timerAlarmEnable(this->timerHandle);
        // SEMAPHORE TAKE? i dont think so
        this->ledFlyCount++;
        
    }

    void LED::deTrigger()
    {
        timerAlarmDisable(this->timerHandle);
        xSemaphoreGiveFromISR(this->ledSemaphore, NULL); //unblock task with this function
        // this semaphore wouldn't give when we were doing trigger() for some reason
        //registerTalk::ledOff(this->busptr); 
        // OOLVOONOO

        digitalWrite(DEBUG_LED, LOW);
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
        //OOLVOONOO

        redLED.state ? digitalWrite(DEBUG_LED, HIGH) : digitalWrite(DEBUG_LED,LOW);
        redLED.state = !redLED.state;

    }

    /**
     * @brief callback function for turning blue LED on/off with timer
    */
    void blueLEDOnCallback()
    {
        // blueLED.state ? registerTalk::ledControlOn(blueLED.busptr, blueLED.power) : registerTalk::ledOff(blueLED.busptr);
        // blueLED.state = !blueLED.state;
        // OOLVOONOO

        blueLED.state ? digitalWrite(DEBUG_LED, HIGH) : digitalWrite(DEBUG_LED,LOW);
        blueLED.state = !blueLED.state;

    }


    // one hardware interrupt will call trigger() to start flashing the LED
    // the other hardware interrupt will call end () to turn the LED off

    /**
     * @brief this is the red led HARDWARE INTERRUPT function.  
     * It will be used to trigger the TIMER function
     * it identifies if the change in RED interrupt pin was:
     * HIGH, which means fly landed and we should flash led or LOW
     * LOW, which means fly left and we should turn LED off
    */
    void IRAM_ATTR changeRedLEDFlash()
    {
        digitalRead(RED_INT) ?  redLED.trigger() : redLED.deTrigger();
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
        digitalRead(BLUE_INT) ?  blueLED.trigger() : blueLED.deTrigger();
    }

    void changeRedLED()
    {

        // if(digitalRead(RED_INT))
        // {
        //     registerTalk::ledControlOn(redLED.busptr, redLED.power);
        //     redLED.ledFlyCount++; // increment total number of interactions
        //     xSemaphoreGiveFromISR(redLED.ledSemaphore, NULL); //unblock task with this function
        // }
        // else
        // {
        //     registerTalk::ledOff(redLED.busptr);
        // }
        //OOLVOONOO

        if(digitalRead(RED_INT))
        {
            digitalWrite(DEBUG_LED, HIGH);
            redLED.ledFlyCount++; // increment total number of interactions
            xSemaphoreGiveFromISR(redLED.ledSemaphore, NULL); //unblock task with this function
        }
        else
        {
            digitalWrite(DEBUG_LED, LOW);
        }
    }

    void changeBlueLED()
    {

        // if(digitalRead(BLUE_INT))
        // {
        //     registerTalk::ledControlOn(blueLED.busptr, blueLED.power);
        //     blueLED.ledFlyCount++;
        //     xSemaphoreGiveFromISR(blueLED.ledSemaphore, NULL); //unblock task with this function
        // }
        // else
        // {
        //     registerTalk::ledOff(blueLED.busptr);
        // }
        //OOLVOONOO

        if(digitalRead(BLUE_INT))
        {
            digitalWrite(DEBUG_LED, HIGH);
            blueLED.ledFlyCount++;
        }
        else
        {
            digitalWrite(DEBUG_LED, LOW);
            xSemaphoreGiveFromISR(blueLED.ledSemaphore, NULL); //unblock task with this function
        }
    }

}