#include <Arduino.h>


#define BLUE_INT 1
#define RED_INT 2

#define BLUE_SDA 9
#define BLUE_SCL 10

#define RED_SDA 47
#define RED_SCL 48

#define DEBUG_RED 39
#define DEBUG_BLUE 41

#define I2C_FREQUENCY 100000

#define DAC_ADDRESS 0b1001000 // address 000 and read/write bit set to write to child

#define GEN_CONFIG_REGISTER 0b11010001 //D1h
#define GEN_CONFIG_MSB 0b00000001 //user-defined, reference datasheet
#define GEN_CONFIG_LSB 0b11100000 //user-defined, reference datasheet

#define DAC_DATA_REGISTER 0b00010000 //10h
#define DAC_DATA_ON_MSB 0b00001111 //temp, for testing
#define DAC_DATA_ON_LSB 0b11111100 //temp, for testing
#define DAC_DATA_OFF 0b00000000

#define TRIGGER_REGISTER 0b11010011 //D3h
#define TRIGGER_MSB 0b00000000
#define TRIGGER_LSB 0b00011000

#define DAC_RESOLUTION 10
#define DAC_5V 1023

#include <Wire.h>


TwoWire redLEDBus = TwoWire(0);
TwoWire blueLEDBus = TwoWire(1);

bool redLEDOn = false;
bool blueLEDOn = false;



//DAC SYNTAX: WRITE <REGISTER NAME (Hex code)>, <MSB DATA>, <LSB DATA>
//Power-up the first DAC, enable VDD reference
//SLEW_RATE: 1.6384 ms (Square wave frequency: 610 Hz)

/*
  Function to write to specific registers
  Starting/ending transmission has to be done externally
*/
bool writeToDAC(TwoWire DACBus, uint8_t reg_address, uint8_t MSB, uint8_t LSB)
{
  //beginTransmission done externally

  DACBus.write(reg_address);
  DACBus.write(MSB);
  DACBus.write(LSB);

  return true;

  //endTransmission done externally

}

// put function declarations here:
void testRedInterrupt(){
  digitalWrite(DEBUG_RED, HIGH);
  redLEDOn = true;
}

void testBlueInterrupt(){
  digitalWrite(DEBUG_BLUE, HIGH);
  blueLEDOn = true;
}

bool testDAC(TwoWire LEDBus){
  // let's do a simple test where we make it turn 5V on and off at 100Hz
  LEDBus.beginTransmission(DAC_ADDRESS); 

  writeToDAC(LEDBus, GEN_CONFIG_REGISTER, GEN_CONFIG_MSB, GEN_CONFIG_LSB);

  writeToDAC(LEDBus, DAC_DATA_REGISTER, DAC_DATA_ON_MSB, DAC_DATA_ON_LSB);

  writeToDAC(LEDBus, TRIGGER_REGISTER, TRIGGER_MSB, TRIGGER_LSB);

  LEDBus.endTransmission();

  unsigned long timestamp = micros();
  while(micros()-timestamp < 33000); // this should give us an APPROX square wave at 30Hz

  LEDBus.beginTransmission(DAC_ADDRESS); 
  
  writeToDAC(LEDBus, GEN_CONFIG_REGISTER, GEN_CONFIG_MSB, GEN_CONFIG_LSB);

  writeToDAC(LEDBus, DAC_DATA_REGISTER, DAC_DATA_OFF, DAC_DATA_OFF);

  writeToDAC(LEDBus, TRIGGER_REGISTER, TRIGGER_MSB, TRIGGER_LSB);

  LEDBus.endTransmission();
  return true;
}

bool testBothDAC(TwoWire Bus1, TwoWire Bus2){
  
  testDAC(Bus1);
  testDAC(Bus2);
  return true;
}

int askUser(){
  while(Serial.available() == 0){}

  int userChoice = Serial.parseInt();

  return userChoice;

}


int menuChoice = 0;

void setup() {

  pinMode(BLUE_INT, INPUT_PULLDOWN);
  pinMode(RED_INT, INPUT_PULLDOWN);

  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Choose your test: ");
  Serial.println("1.Test Red Interrupt");
  Serial.println("2.Test Blue Interrupt");
  Serial.println("3.Test Red LED");
  Serial.println("4.Test Blue LED");
  Serial.println("5.Test Red AND Blue LED Together");

  while(menuChoice != 1 && menuChoice != 2 && menuChoice != 3 && menuChoice != 4 && menuChoice != 5){
    menuChoice = askUser();

    switch(menuChoice)
    {
      case 1:
        attachInterrupt(digitalPinToInterrupt(RED_INT), testRedInterrupt, HIGH);
        break;
      case 2:
        attachInterrupt(digitalPinToInterrupt(BLUE_INT), testBlueInterrupt, HIGH);
        break;
      case 3:
        redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
        break;
      case 4:
        blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
        break;
      case 5:
        redLEDBus.begin(RED_SDA, RED_SCL, I2C_FREQUENCY);
        blueLEDBus.begin(BLUE_SDA, BLUE_SCL, I2C_FREQUENCY);
        break;
      default:
        Serial.println("Try Again");
        break;
    }
  }
    

}



void loop() {
  // put your main code here, to run repeatedly:

  unsigned long timestamp;

  switch(menuChoice)
  {
    case 1:
      Serial.println("Testing Red Interrupt");
      timestamp = micros();
        while(micros() - timestamp < 5000);
        digitalWrite(DEBUG_RED, LOW);
        timestamp = micros();
        while(micros()-timestamp < 5000);
      break;
    case 2:
      Serial.println("Testing Blue Interrupt");
      timestamp = micros();
      if( blueLEDOn == true)
      { 
        while(micros() - timestamp < 5000);
        digitalWrite(DEBUG_BLUE, LOW);
        timestamp = micros();
        while(micros()- timestamp < 5000);
      }
      break;
    case 3:
      testDAC(redLEDBus);
      break;
    case 4:
      testDAC(blueLEDBus);
      break;
    case 5:
      testBothDAC(redLEDBus, blueLEDBus);
      break;
    default:
      Serial.println("Doing Nothing");
      break;
  }


  
}
