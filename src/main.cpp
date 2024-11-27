#include <Arduino.h>


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
    }
  }
}
