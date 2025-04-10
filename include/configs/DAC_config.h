
/**
 * This file defines the registers and pins needed to communicate with the DAC and control the LEDs
 */

#define BLUE_INT 2
#define RED_INT 1

#define BLUE_SDA 9
#define BLUE_SCL 10

#define RED_SDA 47
#define RED_SCL 48

#define DEBUG_RED 39
#define DEBUG_BLUE 41

#define I2C_FREQUENCY 100000

#define DAC_ADDRESS 0b1001000 // address 000 and read/write bit set to write to child

#define GEN_CONFIG_REGISTER 0b11010001 //D1h
#define GEN_CONFIG_WRITE_MSB 0b00010001 //user-defined, reference datasheet
#define GEN_CONFIG_WRITE_LSB 0b11101000 //user-defined, reference datasheet

#define GEN_CONFIG_ON_MSB 0b00010001 //user-defined, reference datasheet
#define GEN_CONFIG_ON_LSB 0b11100000 //user-defined, reference datasheet

#define GEN_CONFIG_OFF_MSB 0b00010001
#define GEN_CONFIG_OFF_LSB 0b11110000

#define DAC_DATA_REGISTER 0b00100001 //21h
#define DAC_DATA_ON_MSB 0b11111111 //temp, for testing
#define DAC_DATA_ON_LSB 0b11111111 //temp, for testing
#define DAC_DATA_HALF_POWER_MSB 0b11111000
#define DAC_DATA_HALF_POWER_LSB 0b00001111
#define DAC_DATA_ON 0b00000011
#define DAC_DATA_OFF 0b00000000

#define TRIGGER_REGISTER 0b11010011 //D3h
#define TRIGGER_MSB 0b00000000
#define TRIGGER_LSB 0b00010000
#define SAVE_TO_NVM_MSB 0b00000000
#define SAVE_TO_NVM_LSB 0b00011000
#define RELOAD_NVM_MSB 0b00000000
#define RELOAD_NVM_LSB 0b00101000

#define DAC_RESOLUTION 10
#define DAC_5V 1023


