#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

char ssid[] = "Ucinet Mobile Access"; 
char pass[] = ""; 
// Name of the server we want to connect to
const char kHostname[] = "worldtimeapi.org";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

// #define ACCEL_PIN 
// #define TEMP_PIN

#define HEART_PIN 32

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

  Serial.println("Pulse sensor connected!"); 
  Serial.println("Calibrating pulse sensor...");

  int reading = 0;
  const int numReadings = 100;
  int min = 4095;
  int max  = 0;
  int desired = millis() + 2000;
  //for (int i = 0; i < numReadings; i++){   // Take avg pulse rate w/o finger
  while (millis() < desired){
      reading = analogRead(HEART_PIN);
      if (reading < min){
        min = reading;
      }
      if (reading > max){
        max = reading;
      }
  }
  threshold = ((max - min) / 2) + min;
  Serial.print("Threshold:");
  Serial.println(threshold);
  Serial.println("Calibration complete.");
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

      WiFiClient c;
      HttpClient http(c);

      char result[20];
      sprintf (result, "%.1d", BPM);
      char urlParam [30];
      strcpy(urlParam, "/?var=");
      strcat (urlParam, result);

      err = http.get("3.145.7.151", 5000, urlParam, NULL);

      

      if (err == 0) {
        Serial.println("startedRequest ok");

        err = http.responseStatusCode();
        if (err >= 0) {
          Serial.print("Got status code: ");
          Serial.println(err);
          
          // Usually you'd check that the response code is 200 or a
          // similar "success" code (200-299) before carrying on,
          // but we'll print out whatever response we get


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
            // Whilst we haven't timed out & haven't reached the end of the body
            while ((http.connected() || http.available()) &&
                    ((millis() - timeoutStart) < kNetworkTimeout)) {
              if (http.available()) {
                c = http.read();
                // Print out this character
                Serial.print(c);

                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
              } else {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
              }
            }
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
  }
}