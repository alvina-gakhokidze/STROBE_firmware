// #include <Arduino.h>
// #include <Wire.h>
// #include <configs/DAC_config.h>
// #include <bricks/DAC_handler.h>
// #include <bricks/LED_handler.h>

// void searchBUS(TwoWire* LEDBus){
//     byte error, address;
//     int nDevices;
//     Serial.println("Scanning...");
//     nDevices = 0;
//     for(address = 1; address < 127; address++ ) {
//     LEDBus->beginTransmission(address);
//     error = LEDBus->endTransmission();
//     if (error == 0) {
//         Serial.print("I2C device found at address 0x");
//         if (address<16) {
//         Serial.print("0");
//         }
//         Serial.println(address,HEX);
//         nDevices++;
//     }
//     else if (error==4) {
//         Serial.print("Unknow error at address 0x");
//         if (address<16) {
//         Serial.print("0");
//         }
//         Serial.println(address,HEX);
//     }    
//     }
//     if (nDevices == 0) {
//     Serial.println("No I2C devices found\n");
//     }
//     else {
//     Serial.println("done\n");
//     }
//     delay(5000);     
// }

// int power = 0;
// int increment = 1;

// void setup(){
//     Serial.begin(115200);
//     strobeLED::blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
//     strobeLED::redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);

//     unsigned long timestamp = millis();
//     while(millis()-timestamp < 3000);
//     //writeToDAC(&blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_MSB, GEN_CONFIG_LSB );
   
   
//     //writeToDAC(&blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER, RELOAD_NVM_MSB, RELOAD_NVM_LSB );
//     // writeToDAC(&blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );
//     // //writeToDAC(&blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER, SAVE_TO_NVM_MSB, SAVE_TO_NVM_LSB );
//     // timestamp = millis();
//     // while(millis()-timestamp < 1);
//     // writeToDAC(&blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_OFF, DAC_DATA_OFF);
//     // timestamp = millis();
//     // while(millis()-timestamp < 1);
//     // writeToDAC(&blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_HALF_POWER_MSB, DAC_DATA_HALF_POWER_LSB);
//     // timestamp = millis();
//     // while(millis()-timestamp < 1);
//     // writeToDAC(&blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_ON_MSB, DAC_DATA_ON_LSB);
//     // timestamp = millis();
//     // while(millis()-timestamp < 1);
//     // writeToDAC(&blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_HALF_POWER_MSB, DAC_DATA_HALF_POWER_LSB);
//     //writeToDAC(&blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER, SAVE_TO_NVM_MSB, SAVE_TO_NVM_LSB );
//     // writeToDAC(&blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER, RELOAD_NVM_MSB, RELOAD_NVM_LSB );
//     // //writeToDAC(&blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_MSB, GEN_CONFIG_LSB );
// //     timestamp = millis();
// //     while(millis()-timestamp < 3000);
// //     writeToDAC(&blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );
// //    //writeToDAC(&blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER, SAVE_TO_NVM_MSB, SAVE_TO_NVM_LSB );
    
//     registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );

//     unsigned long timestamp2 = millis();
//     Serial.println("Wait before we flash LED");
//     while(millis()-timestamp2 < 3000);


//     Serial.println("testing blue led");
//     Serial.println("Reading config register");
//     registerTalk::readRegisters(&strobeLED::blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER);
//     Serial.println("reading dac_data register");
//     registerTalk::readRegisters(&strobeLED::blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER);
//     Serial.println("Reading trigger register");
//     registerTalk::readRegisters(&strobeLED::blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER);
//     delay(10);
//     registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_OFF_MSB, GEN_CONFIG_OFF_LSB );
//     delay(500);
//     registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );
//     registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_HALF_POWER_MSB, DAC_DATA_HALF_POWER_LSB );
//     delay(500);
//     registerTalk::writeToDAC(&strobeLED::redLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_OFF_MSB, GEN_CONFIG_OFF_LSB );
//     delay(500);
//     registerTalk::writeToDAC(&strobeLED::redLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_OFF_MSB, GEN_CONFIG_OFF_LSB );
//     delay(500);
//     registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );
//     //registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_HALF_POWER_MSB, DAC_DATA_HALF_POWER_LSB );
//     delay(500);
//     //registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, DAC_DATA_OFF, DAC_DATA_OFF);
// }

// void loop() {

//     if(power == 1023){
//         increment = -1;
//     }
//     else if(power == 0){
//         increment = 1;
//     }

//     power += increment;

//     byte lower = power & 0xff;
//     byte upper = power >> 8;
//     Serial.print(int(upper << 8 | lower));
//     Serial.print(" ");
//     Serial.println(power);

//     // Serial.println("testing red led");
//     // searchBUS(&redLEDBus);
//     // Serial.println("testing blue led");
//     // searchBUS(&blueLEDBus);


//     // Serial.println("testing red led");
//     // Serial.println("Reading config register");
//     // readRegisters(&redLEDBus, GEN_CONFIG_REGISTER);
//     // Serial.println("reading dac_data register");
//     // readRegisters(&redLEDBus, DAC_DATA_REGISTER);
//     // Serial.println("Reading trigger register");
//     // readRegisters(&redLEDBus, TRIGGER_REGISTER);
//     // Serial.println("testing blue led");
//     // Serial.println("Reading config register");
//     // readRegisters(&blueLEDBus, GEN_CONFIG_REGISTER);
//     // Serial.println("reading dac_data register");
//     // readRegisters(&blueLEDBus, DAC_DATA_REGISTER);
//     // Serial.println("Reading trigger register");
//     // readRegisters(&blueLEDBus, TRIGGER_REGISTER);
//     unsigned long timestamp1 = millis();
//     unsigned long timestamp;
    
//     //writeToDAC(&blueLEDBus, DAC_ADDRESS, TRIGGER_REGISTER, SAVE_TO_NVM_MSB, SAVE_TO_NVM_LSB );
//     timestamp = millis();
//     //while(millis()-timestamp < 5);
//     registerTalk::writeToDAC(&strobeLED::blueLEDBus, DAC_ADDRESS, DAC_DATA_REGISTER, upper, lower);

//    float frequency = ((millis()-timestamp1));
//    Serial.println(frequency);

// }