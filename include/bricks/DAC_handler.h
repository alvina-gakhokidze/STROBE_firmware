
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>


namespace registerTalk
{
  

  /**
     * @brief Write to a specific register on a specific IC
  */
  void writeToDAC(TwoWire* DACBus, uint8_t chip_address, uint8_t reg_address, uint8_t MSB, uint8_t LSB)
  {
    DACBus->beginTransmission(chip_address); 

      DACBus->write(reg_address);
      DACBus->write(MSB);
      DACBus->write(LSB);

    int result = DACBus->endTransmission();
  }

  /**
     * @brief Read from a specific register on a specific IC
  */
  void readRegisters(TwoWire* DACBus, uint8_t chip_address, uint8_t data_register)
  {
      DACBus->beginTransmission(chip_address);
      DACBus->write(data_register);
      DACBus->endTransmission();
      DACBus->requestFrom(chip_address, 2); // We want one byte
      uint8_t val2 = DACBus->read();
      uint8_t val1 = DACBus->read();
      uint16_t val = 0;
      val = (val2 << 8) | val1;
      Serial.printf("DATA REGISTER STATE: %x\n", val);
  }

  /**
     * @brief Sets default state of LED to off. Only ever needs to be called ONCE
  */
  void setupDAC(TwoWire* DACBus) 
  {
    writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER,   GEN_CONFIG_OFF_MSB,      GEN_CONFIG_OFF_LSB);
  }

  void ledOff(TwoWire* DACBus)
  {
    writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER,   GEN_CONFIG_OFF_MSB,      GEN_CONFIG_OFF_LSB);
  }
  
  /**
   * @brief turns LED to specific power
   * @param power must be between 0 and 1023 (2^10-1, since DAC has 10-bit resolution)
  */
  void ledControlOn(TwoWire* DACBus, float power)
  {
    int powerOfTwo = (int) ( (float) DAC_5V/(5.0/power) );
    // convert number into two bytes (to store a padded 10-bit number) to send to register

    uint16_t ten_bit = (powerOfTwo & 0x3FF);  // Mask with 0x3FF to ensure it's 10 bits
    
    uint16_t result = (0xF << 12) | (ten_bit << 2) | 0x3;  // Set 1111 on the left and 11 on the right
    
    byte MSB = (result >> 8) & 0xff;
    byte LSB = result & 0xff;

    writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_OFF_MSB, GEN_CONFIG_OFF_LSB);
    writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER,   GEN_CONFIG_ON_MSB,      GEN_CONFIG_ON_LSB);
    writeToDAC(DACBus, DAC_ADDRESS, DAC_DATA_REGISTER, MSB, LSB);
  }
}
