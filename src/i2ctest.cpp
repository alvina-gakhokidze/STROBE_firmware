// #include <Arduino.h>
// #include <Wire.h>
// #include <configs/DAC_config.h>

// TwoWire redLEDBus = TwoWire(0);
// TwoWire blueLEDBus = TwoWire(1);

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

// void setup(){
//     Serial.begin(115200);
//     // Wire.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
//     redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
//     blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
// }

// void loop() {

//     Serial.println("testing red led");
//     searchBUS(&redLEDBus);
//     Serial.println("testing blue led");
//     searchBUS(&blueLEDBus);
// }