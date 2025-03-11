
#pragma once
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>


namespace registerTalk
{
  

  /**
     * @brief Write to a specific register on a specific IC
     *
     * @returns true if successful, false if not (i.e. NVM busy)
  */
  bool writeToDAC(TwoWire* DACBus, uint8_t chip_address, uint8_t reg_address, uint8_t MSB, uint8_t LSB)
  {
    DACBus->beginTransmission(chip_address); 

      // if(reg_address == DAC_DATA_REGISTER)
      // {
      //     DACBus->write(reg_address);
      //     DACBus->write(DAC_DATA_ON >> 2);
      //     DACBus->write((DAC_DATA_ON & 0x03) << 6);
      // }
      // else
      // {
      //     DACBus->write(reg_address);
      //     DACBus->write(MSB);
      //     DACBus->write(LSB);
      // }

      DACBus->write(reg_address);
      DACBus->write(MSB);
      DACBus->write(LSB);

    int result = DACBus->endTransmission();

    // if(result == 0){
    //   Serial.println("successful transmission!");
    //   return true;
    // }
    // else{
    //   Serial.printf("failed transmission %d error \n", result);
    //   return false;
    // }
    return true;
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
      // if(DATA_REGISTER == DAC_DATA_REGISTER){
      //     val = (val2 << 2) | (val1 >> 6);
      // }
      // else{
      //     val = (val2 << 8) | val1;
      // }
      val = (val2 << 8) | val1;
      Serial.printf("DATA REGISTER STATE: %x\n", val);
  }

  /**
     * @brief Sets default state of LED to off. Only ever needs to be called ONCE
  */
  void setupDAC(TwoWire* DACBus) 
  {
    // writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER, GEN_CONFIG_ON_MSB, GEN_CONFIG_ON_LSB );
    // // turn DAC on

    // Serial.printf("wait\n");

    // writeToDAC(DACBus, DAC_ADDRESS, DAC_DATA_REGISTER,   DAC_DATA_OFF,      DAC_DATA_OFF);
    // Serial.printf("turning LED off\n");
    // set DAC output to OFF (0b00000000)
    // writeToDAC(DACBus, DAC_ADDRESS, TRIGGER_REGISTER,    SAVE_TO_NVM_MSB,   SAVE_TO_NVM_LSB );
    // writeToDAC(DACBus, DAC_ADDRESS, TRIGGER_REGISTER,    RELOAD_NVM_MSB,    RELOAD_NVM_LSB );
    writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER,   GEN_CONFIG_OFF_MSB,      GEN_CONFIG_OFF_LSB);
  }

  bool ledOff(TwoWire* DACBus)
  {
    //return writeToDAC(DACBus, DAC_ADDRESS, DAC_DATA_REGISTER,   DAC_DATA_OFF,      DAC_DATA_OFF);
    return writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER,   GEN_CONFIG_OFF_MSB,      GEN_CONFIG_OFF_LSB);
  }
  
  /**
   * @brief turns LED to specific power
   * @param power must be between 0 and 1023 (2^10-1, since DAC has 10-bit resolution)
  */
  bool ledControlOn(TwoWire* DACBus, float power)
  {
    int powerOfTwo = DAC_5V/(5.0/power);
    // convert number into two bytes (to store a padded 10-bit number) to send to register
    uint8_t MSB = powerOfTwo >> 6;
    uint8_t LSB = ( powerOfTwo & 0x3F) << 2;
    
    writeToDAC(DACBus, DAC_ADDRESS, GEN_CONFIG_REGISTER,   GEN_CONFIG_ON_MSB,      GEN_CONFIG_ON_LSB);
    return writeToDAC(DACBus, DAC_ADDRESS, DAC_DATA_REGISTER, MSB, LSB);
  }
}
