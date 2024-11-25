#include <Arduino.h>
#include <Wire.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"


// #define ACCEL_PIN 
// #define TEMP_PIN

#define HEART_PIN 21

int signal = 0;
int threshold = 550;
int BPM = 0;
int beatCount = 0;
unsigned long time = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  pinMode(HEART_PIN, INPUT);

  Serial.println("Pulse sensor connected");
}

void loop() {
  signal = analogRead(HEART_PIN);
  
  if (signal > threshold) {
    unsigned long currTime = millis();
    if ((currTime - time) > 300) {
      time = currTime;
      beatCount ++;
    }
  }
  
  if (millis() - time > 10000) {
    BPM = (time * 60) / 10;
    Serial.print("Heart Rate: ");
    Serial.print(BPM);
    Serial.println(" BPM");

    time = 0;
  }
  
  delay(100);
}
