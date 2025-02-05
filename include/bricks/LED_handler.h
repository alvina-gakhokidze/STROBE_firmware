
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>
#include <bricks/DAC_handler.h>

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
            
        
        public:
            hw_timer_t* timerHandle;
            TwoWire* ledptr;
            volatile unsigned frequency; // stores flashing frequency
            volatile unsigned power;    // stores power 
            volatile bool state;
            void IRAM_ATTR ledISR();
            
            LED(TwoWire* ledptr, unsigned frequency, unsigned power, bool state);

            void trigger();
            void cancel();

    };

    /**
     * @brief constructor for new LED object
     * @param ledptr pointer to DAC bus that we want to control
     * @param period_ms for chosen frequency through LED
     * @param power chosen power through LED
     * @param state led on or off
    */
    LED::LED(TwoWire* ledptr, unsigned frequency, unsigned power, bool state)
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

        this->frequency = frequency;
        this->power = power;
        this->state = state;
        this->timerHandle = timerBegin(this->timerNum, 80, true);

    }
    
    /**
     * @brief Ends timer and turns LED off
    */
    void LED::cancel()
    {
        timerEnd(this->timerHandle);
        registerTalk::ledOff(this->ledptr);
    }
    
   /**
     * @brief creates timer for corresponding ledBus
    */
    void LED::trigger()
    {
        timerAttachInterrupt(this->timerHandle, this->interruptFunction, true);
        timerStart(this->timerHandle);
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

     void stopRedLED()
    {
        timerEnd(redLED.timerHandle);
        registerTalk::ledOff(redLED.ledptr);
    }

     void stopBlueLED()
    {
        timerEnd(blueLED.timerHandle);
        registerTalk::ledOff(blueLED.ledptr);
    }

    void setupBoard()
    {
        pinMode(RED_INT, INPUT);
        attachInterrupt(RED_INT, &stopRedLED, CHANGE);
        pinMode(BLUE_INT, INPUT);
    }

    //if it triggers with change we need a way for it to identify if the change was rising or falling, and what set of steps to execute based on that

}