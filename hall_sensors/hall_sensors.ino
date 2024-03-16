#include <cmath> // For M_PI
char buffer[30];
// Define sensor pins
const int hallSensorPin1 = 2;
const int hallSensorPin2 = 3;
const int hallSensorPin3 = 4;
const int hallSensorPin4 = 5;

// Variables to store the last trigger time
volatile unsigned long lastTriggerTime1 = 0;
volatile unsigned long lastTriggerTime2 = 0;
volatile unsigned long lastTriggerTime3 = 0;
volatile unsigned long lastTriggerTime4 = 0;

// Variables to store the last valid speed
volatile float lastValidSpeed1 = 0.0;
volatile float lastValidSpeed2 = 0.0;
volatile float lastValidSpeed3 = 0.0;
volatile float lastValidSpeed4 = 0.0;

// Wheel diameter in meters (0.12m)
const float wheelDiameter = 0.12;
// Calculate circumference (C = pi * d) using M_PI for Pi
const float wheelCircumference = M_PI * wheelDiameter;

// Maximum allowed speed change in km/h to filter inaccuracies
const float maxSpeedChange = 10.0;


void ISR_sensor1() {
  calculateSpeed(lastTriggerTime1, lastValidSpeed1, 1);
  int len = snprintf(buffer, sizeof(buffer), "%0.2f,%0.2f,%0.2f,%0.2f", lastValidSpeed1, lastValidSpeed2,lastValidSpeed3,lastValidSpeed4);
  Serial.write(buffer);
  Serial.println();
}

void ISR_sensor2() { calculateSpeed(lastTriggerTime2, lastValidSpeed2, 2); }
void ISR_sensor3() { calculateSpeed(lastTriggerTime3, lastValidSpeed3, 3); }
void ISR_sensor4() { calculateSpeed(lastTriggerTime4, lastValidSpeed4, 4); }

void setup() {
  Serial.begin(9600);
  pinMode(hallSensorPin1, INPUT_PULLUP);
  pinMode(hallSensorPin2, INPUT_PULLUP);
  pinMode(hallSensorPin3, INPUT_PULLUP);
  pinMode(hallSensorPin4, INPUT_PULLUP);
  while (!Serial);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin1), ISR_sensor1, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin2), ISR_sensor2, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin3), ISR_sensor3, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin4), ISR_sensor4, FALLING);
}

void loop() {
}

void calculateSpeed(volatile unsigned long &lastTriggerTime, volatile float &lastValidSpeed, int sensorPin) {
  unsigned long currentTime = micros();
  float timeElapsed = (currentTime - lastTriggerTime) / 1000000.0; // Time in seconds
  if (timeElapsed == 0) return; // Prevent division by zero

  float speedKmPerH = (wheelCircumference / timeElapsed) * 3.6;

  // Check if speed change is within the realistic threshold
  if (abs(speedKmPerH - lastValidSpeed) <= maxSpeedChange || lastValidSpeed == 0.0) {
    lastValidSpeed = speedKmPerH; // Update last valid speed
  }
  lastTriggerTime = currentTime; // Always update last trigger time
}
