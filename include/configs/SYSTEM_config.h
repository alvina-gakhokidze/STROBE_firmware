#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/semphr.h"



#define CURR_UPP_LIMIT 0.9

#define CURR_LOW_LIMIT 0.1


#define FREQ_UPP_LIMIT 100.0

#define FREQ_LOW_LIMIT 0.0

#define POWER_RESISTOR_VALUE 5.0 // in ohms
// i need to define special case for if frequency is 0
