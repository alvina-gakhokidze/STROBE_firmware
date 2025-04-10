/**
 * This is the main config file. It defines all the libraries needed and hardware limits for the PCBs
 */

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <esp_now.h>
#include <WiFi.h>

#define CURR_UPP_LIMIT 900 //mA

#define CURR_LOW_LIMIT 100 //mA

#define FREQ_UPP_LIMIT 100.0 //Hz

#define FREQ_LOW_LIMIT 0.0 //Hz

#define DEFAULT_LOW_PERIOD_US 100000 //us

#define POWER_RESISTOR_VALUE 5.0 // in ohms

#define DEBUG_LED_BLUE 41
#define DEBUG_LED_RED 42
// pins 39, 40, 41, and 42 can all be used as toggle pins for firmware debugging

