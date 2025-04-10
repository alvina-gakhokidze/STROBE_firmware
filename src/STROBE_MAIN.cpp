
#include <tasks/LED_tasks.h>
#include <tasks/filter_tasks.h>
#include <tasks/PESC_tasks.h>
#include <tasks/board_tasks.h>
#include <configs/Task_config.h>

void setup()
{
    Serial.begin(115200);

    pinMode(DEBUG_LED_BLUE, OUTPUT);
    digitalWrite(DEBUG_LED_BLUE, LOW);

    pinMode(DEBUG_LED_RED, OUTPUT);
    digitalWrite(DEBUG_LED_RED, LOW);

    Serial.printf("Beggining i2c busses\n");
    strobeLED::redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
    strobeLED::blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
    //gpio_install_isr_service(ESP_INTR_FLAG_LEVEL6); // higher flag number, higher priority
    // this may or may not need to be uncommented once on a new computer or STROBE ^^ if detachInterrupt() function not working

    registerTalk::ledOff(&strobeLED::redLEDBus);
    registerTalk::ledOff(&strobeLED::blueLEDBus);

    while(!boardTasks::initBroadcast());

    Serial.printf("Waiting for data from user\n");

    while(!boardTasks::thisBoard.dataReceived){
       Serial.println(boardTasks::thisBoard.dataReceived);
    }
}

void loop()
{
    vTaskDelete(NULL);
}


