#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DHT20.h"
#include <SparkFunLSM6DSO.h>

// Pins and Constants
#define TEMP_PIN_SDA 22
#define TEMP_PIN_SCL 21
//#define STEP_PIN_SDA 10
//#define STEP_PIN_SCL 12
#define HEART_PIN 32

char ssid[] = "Panda Express"; 
char pass[] = "Orangechicken315";

void connectToWiFi();
void calibratePulseSensor();
void calibrateStepSensor();
void readPulseSensor();
void readTemperature();
void countSteps();
void sendDataToServer();


DHT20 DHT;
LSM6DSO myIMU;

int step = 0, count = 0, beatCount = 0, BPM = 0, threshold = 0;
bool startMonitor = false;
const int monitoringPeriod = 10000;
int offset = 0;
unsigned long lastBeatTime = 0;

void setup(){
  Serial.begin(9600);
  delay(1000);

  Wire.begin(TEMP_PIN_SDA, TEMP_PIN_SCL);
  DHT.begin();
  Serial.println("Hello World!");
  // if (!myIMU.begin() || !myIMU.initialize(BASIC_SETTINGS)) {
  //   Serial.println("Failed to initialize LSM6DSO!");
  //   while (1);
  // }

  // Connect to WiFi
  connectToWiFi();

  // Calibrate Sensors
  calibratePulseSensor();
  //calibrateStepSensor();

}

void loop() {
  readPulseSensor();
  readTemperature();
  //countSteps();
  sendDataToServer();
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
}

void calibratePulseSensor() {
  Serial.println("Pulse sensor connected!"); 
  Serial.println("Calibrating pulse sensor...");

  int reading = 0;
  const int numReadings = 100;
  int min_pulse = 4095;
  int max_pulse  = 0;
  int desired = millis() + 2000;
  while (millis() < desired){
      reading = analogRead(HEART_PIN);
      if (reading < min_pulse){
        min_pulse = reading;
      }
      if (reading > max_pulse){
        max_pulse = reading;
      }
  }
  threshold = ((max_pulse - min_pulse) / 2) + min_pulse;
  Serial.print("Threshold:");
  Serial.println(threshold);
  Serial.println("Calibration complete.");
  Serial.println("Place finger on sensor. Monitoring will begin in 10 seconds.");
}

void calibrateStepSensor() {
  int min1 = myIMU.readFloatGyroX();
  int min2 = myIMU.readFloatGyroY();
  int min = sqrt(pow(min1,2) + pow(min2,2));
  Serial.println("MOVE");
  delay(5000);

  int max1= myIMU.readFloatGyroX();
  int max2 = myIMU.readFloatGyroY();
  int max = sqrt(pow(max1,2) + pow(max2,2));
  step = abs(max - min);
  delay(1000);
}

void readPulseSensor() {
  static unsigned long startTime = millis(); // Time when monitoring started
  static int heartSignal1 = 0;              // Previous signal value
  static int heartSignal2 = 0;              // Current signal value
  static unsigned long lastSampleTime = millis(); // Last time a sample was taken
  static unsigned long lastBeatTime = 0;    // Last time a valid heartbeat was detected

  if (!startMonitor && millis() - startTime >= 10000) {
    Serial.println("Begin monitoring");
    startMonitor = true;
    beatCount = 0;
    lastBeatTime = 0;
    startTime = millis();
  }

  if (startMonitor){
    unsigned long currTime = millis();

    if (currTime - lastSampleTime >= 2) {
      lastSampleTime = currTime;

      heartSignal1 = analogRead(HEART_PIN);

      // Debug output
      Serial.print("HeartSignal1: ");
      Serial.print(heartSignal1);
      Serial.print(" | BeatCount: ");
      Serial.println(beatCount);

      if ((heartSignal1 >= threshold) && (currTime - lastBeatTime > 300)) {
        lastBeatTime = currTime;
        beatCount++;
        Serial.println("Heartbeat detected!");
      }
    }

    if (currTime - startTime >= monitoringPeriod) {
      BPM = beatCount * 6;
      sendDataToServer();
      Serial.print("Total Beats: ");
      Serial.println(beatCount);
      Serial.print("Heart Rate: ");
      Serial.print(BPM);
      Serial.println(" BPM");

      Serial.println("Monitoring complete. Restarting...");
      startMonitor = false;
      startTime = millis();
          int err = 0;

    }
  }
}

void readTemperature() {
  DHT.read();
  float temperature = DHT.getTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);
}

void countSteps() {
  int prev1= myIMU.readFloatGyroX();
  int prev2 = myIMU.readFloatGyroY();
  int prev = sqrt(pow(prev1,2) + pow(prev2,2));
  delay(1000);

  int curr1= myIMU.readFloatGyroX();
  int curr2 = myIMU.readFloatGyroY();
  int curr = sqrt(pow(curr1,2) + pow(curr2,2));
  if (abs(curr-prev) >= step){
    count = count + 1;
  }

  Serial.println(count);
  std::string print_count = std::to_string(count);
  delay(20);
}

void sendDataToServer() {
  WiFiClient c;
  HttpClient http(c);

  // Format and send data
  char urlParam[50];
  sprintf(urlParam, "/update_step?temp=%.1f&bpm=%d", DHT.getTemperature(), beatCount * 6);

  int err = http.get("3.145.7.151", 5000, urlParam, NULL);
  if (err != 0) Serial.println("Failed to send data.");
}
