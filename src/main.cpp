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

char ssid[] = "Panda Express"; 
char pass[] = "Orangechicken315"; 
// // Name of the server we want to connect to
// const char kHostname[] = "worldtimeapi.org";
// // Path to download (this is the bit after the hostname in the URL
// // that you want to download
// const char kPath[] = "/api/timezone/Europe/London.txt";

// // Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// // Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

#define STEP_PIN_SDA 25
#define STEP_PIN_SCL 26

#define TEMP_PIN_SDA 21
#define TEMP_PIN_SCL 22

#define HEART_PIN 32

DHT20 DHT;
LSM6DSO myIMU;

int step = 0;
int count = 0;

int offset = 0;
unsigned long lastBeatTime = 0;
int BPM = 0;
int beatCount = 0;
bool startMonitor = false;
const int monitoringPeriod = 10000;
int threshold = 0;

void setup(){
  Serial.begin(9600);
  delay(1000);
  
  pinMode(HEART_PIN, INPUT);

  Wire.begin(TEMP_PIN_SDA, TEMP_PIN_SCL);
  Wire.begin(STEP_PIN_SDA, STEP_PIN_SCL);

  // **************************************************************

  count = 0;

  Wire.begin();
  delay(10);
  if( myIMU.begin() )
    Serial.println("Ready.");
  else { 
    Serial.println("Could not connect to IMU.");
    Serial.println("Freezing");
  }

   if( myIMU.initialize(BASIC_SETTINGS) )
    Serial.println("Loaded Settings.");

  int min1 = myIMU.readFloatGyroX();
  int min2 = myIMU.readFloatGyroY();
  int min = sqrt(pow(min1,2) + pow(min2,2));
  Serial.println("MOVE");
  delay(5000);

  int max1= myIMU.readFloatGyroX();
  int max2 = myIMU.readFloatGyroY();
  int max = sqrt(pow(max1,2) + pow(max2,2));
  step = abs(max - min);
  Serial.println("Starting loop");
  delay(1000);

  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
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

  Wire.begin();
  DHT.begin();

  // **************************************************************

  Serial.println("Pulse sensor connected!"); 
  Serial.println("Calibrating pulse sensor...");

  int reading = 0;
  const int numReadings = 100;
  int min_pulse = 4095;
  int max_pulse  = 0;
  int desired = millis() + 2000;
  //for (int i = 0; i < numReadings; i++){   // Take avg pulse rate w/o finger
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
  // Serial.print("Connecting to ");
  // Serial.println(ssid);

  // WiFi.begin(ssid, pass);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());
  // Serial.println("MAC address: ");
  // Serial.println(WiFi.macAddress());
  Serial.println("Place finger on sensor. Monitoring will begin in 10 seconds.");
}

void loop(){
  

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

  // **************************************************************
  // Temperature
  DHT.read();

  int err = 0;

  WiFiClient c;
  HttpClient http(c);

  float temperature = DHT.getTemperature();

  char result[20];
  sprintf (result, "%.1f,%.1f", temperature);
  char urlParam [30];
  strcpy(urlParam, "/?var=");
  strcat (urlParam, result);

  err = http.get("3.145.7.151", 5000, urlParam, NULL);

  // **************************************************************
  // Step count

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

  // **************************************************************
  if (err == 0) {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0) {
      Serial.print("Got status code: ");
      Serial.println(err);

      err = http.skipResponseHeaders();
      if (err >= 0) {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");

        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;

      } else {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    } else {
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  } else {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();
}