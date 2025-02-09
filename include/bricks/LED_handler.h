
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>
#include <bricks/DAC_handler.h>

/**
 * The functions here handle turning the LEDs on/off at the specific frequency with the specific power
*/

TwoWire redLEDBus = TwoWire(0);
TwoWire blueLEDBus = TwoWire(1);

SemaphoreHandle_t redLEDSemaphore;
SemaphoreHandle_t blueLEDSemaphore;

int redLEDFlyCount;
int blueLEDFlyCount;



namespace strobeLED
{

    void IRAM_ATTR redLEDOnISR();
    void IRAM_ATTR blueLEDOnISR();

    class LED
    {
        private:

            int timerNum;
            int gpio;
            void (*interruptFunction)(void); // not entirely sure that this will work but let's give it a try
            void attachTimer();
        
        public:
            hw_timer_t* timerHandle;
            TwoWire* ledptr;
            volatile unsigned period_us; // stores flashing frequency
            volatile unsigned power;    // stores power 
            volatile bool state;
            volatile unsigned ledFlyCount;
            SemaphoreHandle_t ledSemaphore = NULL; // hopefully it's overwritten later
            
            LED(TwoWire* ledptr, unsigned period_us, unsigned power, bool state);

            void trigger();
            void deTrigger();
            void cancel();

    };

    /**
     * @brief constructor for new LED object
     * @param ledptr pointer to DAC bus that we want to control
     * @param period_us chosen flashing period in microseconds (us)
     * @param power chosen power through LED
     * @param state led on or off
    */
    LED::LED(TwoWire* ledptr, unsigned period_us, unsigned power, bool state)
    {
        this->ledptr = ledptr;
        if(ledptr == &redLEDBus)
        { 
            redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
            this->interruptFunction = &redLEDOnISR;
            this->timerNum = 0;
        }
        else if(ledptr == &blueLEDBus)
        {
            blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
            this->interruptFunction = &blueLEDOnISR;
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
        registerTalk::ledOff(this->ledptr);
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
        timerAlarmWrite(this->timerHandle, this->period_us, true); // have to write a new alarm each tiem because the period_us will change
        timerAlarmEnable(this->timerHandle);
        // SEMAPHORE TAKE? i dont think so
        this->ledFlyCount++; // increment total number of interactions
        xSemaphoreGiveFromISR(this->ledSemaphore, NULL); //unblock task with this function
    }

    void LED::deTrigger()
    {
        timerAlarmDisable(this->timerHandle);
        registerTalk::ledOff(this->ledptr);
    }


    LED redLED(&redLEDBus, 1, 1, true);
    LED blueLED(&blueLEDBus, 1, 1, true);

    /**
     * @brief ISR routine for turning red LED on/off with timer
    */
    void IRAM_ATTR redLEDOnISR()
    {
        redLED.state ? registerTalk::ledControlOn(redLED.ledptr, redLED.power) : registerTalk::ledOff(redLED.ledptr);
        redLED.state = !redLED.state;
    }

    /**
     * @brief ISR routine for turning blue LED on/off with timer
    */
    void IRAM_ATTR blueLEDOnISR()
    {
        blueLED.state ? registerTalk::ledControlOn(blueLED.ledptr, blueLED.power) : registerTalk::ledOff(blueLED.ledptr);
        blueLED.state = !blueLED.state;
    }


    // one hardware interrupt will call trigger() to start flashing the LED
    // the other hardware interrupt will call end () to turn the LED off

    /**
     * @brief identifies if the change in RED interrupt pin was:
     * HIGH, which means fly landed and we should flash led or LOW
     * LOW, which means fly left and we should turn LED off
    */
     void changeRedLED()
    {
        digitalRead(RED_INT) ?  redLED.trigger() : redLED.deTrigger();
    }

    /**
     * @brief identifies if the change in BLUE interrupt pin was:
     * HIGH, which means fly landed and we should flash led or LOW
     * LOW, which means fly left and we should turn LED off
    */
     void changeBlueLED()
    {
        digitalRead(BLUE_INT) ?  blueLED.trigger() : blueLED.deTrigger();
    }

    /**
     * @brief sets interrupt pins to correct mode and attaches interrupts
    */
    void setupBoard()
    {
        pinMode(RED_INT, INPUT);
        attachInterrupt(RED_INT, &changeRedLED, CHANGE);
        // so we cannot attach more than one interrupt to the same pin
        pinMode(BLUE_INT, INPUT);
        attachInterrupt(BLUE_INT, &changeBlueLED, CHANGE);
        
    }

    /**
     * @brief This task is semaphore-dependent. It will constantly check whether or not flyCount has been updated, and will be blocked if not
     * @param ledBus the LED object we're interested in (red or blue)
    */
    



}

