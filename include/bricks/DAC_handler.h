
#include <configs/SYSTEM_config.h>
#include <configs/DAC_config.h>

TwoWire redLEDBus = TwoWire(0);
TwoWire blueLEDBus = TwoWire(1);


/*
  Function to write to specific registers
  Starting/ending transmission has to be done externally
*/
bool writeToDAC(TwoWire* DACBus, uint8_t reg_address, uint8_t MSB, uint8_t LSB)
{
  //beginTransmission done externally

  DACBus->write(reg_address);
  DACBus->write(MSB);
  DACBus->write(LSB);

  return true;

  //endTransmission done externally

}