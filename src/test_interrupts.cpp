// #include <configs/SYSTEM_config.h>
// #include <tasks/board_tasks.h>

// void ARDUINO_ISR_ATTR changeRedLED()
// {

    
//         registerTalk::ledControlOn(strobeLED::redLED.busptr, 0.5);
    
// }


// void setup()
// {

//     Serial.begin(115200);
//     delay(2000);
//     pinMode(DEBUG_LED, OUTPUT);
//     digitalWrite(DEBUG_LED,LOW);
//     pinMode(RED_INT, INPUT);
//     Serial.printf("Going to setup board\n");
//     Serial.printf("no flashing for red LED\n");
    

//     Serial.printf("Beggining i2c busses\n");
//     strobeLED::redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
//     //strobeLED::blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);

//     registerTalk::writeToDAC(&strobeLED::redLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );
//     Serial.printf("trying to flash\n");
    
//     float power = 2;

//     int powerOfTwo = DAC_5V/(5.0/power);
//     // convert number into two bytes (to store a padded 10-bit number) to send to register
//     uint8_t MSB = powerOfTwo >> 6;
//     uint8_t LSB = ( powerOfTwo & 0x3F) << 2;

//     printf("msb: %d\n", MSB);
//     printf("lsb: %d\n", LSB);
//     unsigned long time1 = millis();
//     registerTalk::ledControlOn(strobeLED::redLED.busptr, 1);
//     while(millis()-time1<2000);
//     time1 = millis();
//     registerTalk::ledControlOn(strobeLED::redLED.busptr, 5);
//     while(millis()-time1<1000);
//     Serial.printf("part2\n");
//     time1 = millis();
    
//     while(millis()-time1 < 500);
//     Serial.printf("part3\n");
//    registerTalk::ledOff(&strobeLED::redLEDBus);
//     Serial.printf("flashed!\n");

//     attachInterrupt(RED_INT, &changeRedLED, CHANGE);
// }

// void loop()
// {

// }