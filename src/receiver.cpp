/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp32-esp8266/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct board_params {
    bool redLEDOn;
    bool blueLEDOn;
    bool powerOn;
    bool frequencyOn;
    bool manualMode;
    bool redLEDFlashingEnabled;
    bool blueLEDFlashingEnabled;
    float redLEDFrequency;
    float redLEDPower;
    float blueLEDFrequency;
    float blueLEDPower;
} board_params;

// typedef struct test_struct {
//     int x;
//     int y;
//   } test_struct;

//Create a struct_message called myData
test_struct myData;

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Bytes received: ");
    Serial.println(len);
    
    if(myData.redLEDOn)
    {
        Serial.printf("Red LED Enabled\n");
    }
    if(myData.blueLEDOn)
    {
        Serial.printf("Blue LED Enabled\n");
    }

    if(myData.manualMode == true)
    {
        Serial.printf("Manual Mode Enabled\n");

        if(myData.redLEDOn)
        {
            if(!myData.redLEDFlashingEnabled) Serial.printf("Red LED Flashing Disabled\n");
        }
        if(myData.blueLEDOn)
        {
            if(!myData.blueLEDFlashingEnabled) Serial.printf("Blue LED Flashing Disabled\n");
        }
    }
    else{
        Serial.printf("Control Mode Enabled\n");

        if(myData.powerOn && myData.frequencyOn)
        {
            Serial.printf("Both Power and Frequency Control Enabled\n");
        }
        else if(myData.powerOn && !myData.frequencyOn)
        {
            Serial.printf("Power Control enabled\n");

            if(myData.redLEDOn)
            {
                Serial.printf("Red LED Frequency: %lf \n", myData.redLEDFrequency);
            }
            if(myData.blueLEDOn)
            {
                Serial.printf("Blue LED Frequency: %lf \n", myData.blueLEDFrequency);
            }
        }
        else if(myData.frequencyOn && !myData.powerOn)
        {
            Serial.printf("Frequency Control enabled\n");

            if(myData.redLEDOn)
            {
                Serial.printf("Red LED Power: %lf \n", myData.redLEDPower);
            }
            if(myData.blueLEDOn)
            {
                Serial.printf("Blue LED Power: %lf \n", myData.blueLEDPower);
            }
        }
    }

    Serial.println();
}
 
void setup() {

    delay(3000);
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.printf("everything succesfull!\n"); // nice
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // this is called only once
}
 
void loop() {

}
