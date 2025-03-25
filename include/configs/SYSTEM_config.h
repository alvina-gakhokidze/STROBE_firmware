#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <esp_now.h>
#include <WiFi.h>

#define CURR_UPP_LIMIT 900

#define CURR_LOW_LIMIT 100


#define FREQ_UPP_LIMIT 200.0

#define FREQ_LOW_LIMIT 0.0

#define DEFAULT_LOW_PERIOD_US 100000
// average sip lasts 100ms. this will make sure led (mostly) doesn't flash during that

#define POWER_RESISTOR_VALUE 5.0 // in ohms
// i need to define special case for if frequency is 0


#define DEBUG_LED 41