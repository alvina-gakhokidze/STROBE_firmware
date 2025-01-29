
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

