
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>

TwoWire redLEDBus = TwoWire(0);
TwoWire blueLEDBus = TwoWire(1);


/*
  Function to write to specific registers
  Starting/ending transmission has to be done externally
*/
bool writeToDAC(TwoWire* DACBus, uint8_t chip_address, uint8_t reg_address, uint8_t MSB, uint8_t LSB)
{
  //beginTransmission done externally
  DACBus->beginTransmission(chip_address); 

    if(reg_address == DAC_DATA_REGISTER)
    {
        DACBus->write(reg_address);
        DACBus->write(DAC_DATA_ON >> 2);
        DACBus->write((DAC_DATA_ON & 0x03) << 6);
    }
    else
    {
        DACBus->write(reg_address);
        DACBus->write(MSB);
        DACBus->write(LSB);
    }

    // DACBus->write(reg_address);
    //     DACBus->write(MSB);
    //     DACBus->write(LSB);

  int result = DACBus->endTransmission();

  if(result == 0){
    Serial.println("successful transmission!");
  }
  else{
    Serial.printf("failed transmission %d error \n", result);
  }

  return true;

  //endTransmission done externally

}

void readRegisters(TwoWire* LEDBus, uint8_t DATA_REGISTER){
    LEDBus->beginTransmission(DAC_ADDRESS);
    LEDBus->write(DATA_REGISTER);
    LEDBus->endTransmission();
    LEDBus->requestFrom(DAC_ADDRESS, 2); // We want one byte
    uint8_t val2 = LEDBus->read();
    uint8_t val1 = LEDBus->read();
    uint16_t val = 0;
    // if(DATA_REGISTER == DAC_DATA_REGISTER){
    //     val = (val2 << 2) | (val1 >> 6);
    // }
    // else{
    //     val = (val2 << 8) | val1;
    // }
    val = (val2 << 8) | val1;
    Serial.printf("DATA REGISTER STATE: %x\n", val);

}