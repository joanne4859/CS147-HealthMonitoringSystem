# CS147-HealthMoinitoringSystem

## Motivation

Monitoring the health of patients, especially those with physical disabilities or elderly individuals, can be challenging. Frequent trips to healthcare facilities can both be strenuous and inconvenient, potentially impacting their well-being. A home-based health monitoring system provides a practical solution, allowing for consistent tracking of vital signs such as body temperature, heart-rate, and respiratory rate. Moreover, if any metrics cross thresholds, alerts can be sent to an SMS system to alert the user of any potential danger. This system aims to reduce the burden of transportation while ensuring timely health updates, which can contribute to better overall care. 

## Solution

Our solution is to create a system of wireless sensors to monitor  vital signs such as body temperature, heart rate, and respiratory rate. We will utilize  a temperature sensor, a pulse heartbeat sensor, and a combination of an accelerometer and gyroscope. The temperature sensor will track body temperature, while the pulse sensor monitors heart rate.  The accelerometer and gyroscope will measure the rise and fall of the chest or diaphragm as a person breathes in and out. The frequency of these movements will calculate and determine the respiratory rate. 

The data from the sensors will be wirelessly transmitted to an ESP32 via Bluetooth. All collected metrics will be displayed in real-time on a user-friendly website or app, with easy-to-view graphs to help users track trends over time. This will allow for remote monitoring and visualization of vital signs.

## Components
* ESP32 Microcontroller
* LSM6DSO32 Accelerometer and Gyroscope
* Pulse Sensor
* MAX30205 Temperature Sensor

## Key Features
* Graphical displays of data on a website
* Data gathering from sensors
