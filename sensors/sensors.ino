#include <SoftwareSerial.h>
#include "TFMini.h"
#include <cmath> // For M_PI
#include <TinyGPS++.h>


#define diameterCm 12
#define millisToHours 3600000.00

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

volatile float distanceFinal = 0.0;

// Default is the position of FIIT STU
volatile float gpsLon = 17.071734;
volatile float gpsLat = 48.153435;

char buffer[60];

TFMini tfmini;

SoftwareSerial SerialTFMini(6,7); //The only value that matters here is the first one, 2, Rx
// serial(1) = pin12=RX, pin13=TX
// serial(2) = pin16=RX green, pin17=TX white

TinyGPSPlus gps;

SoftwareSerial GPSSerial(8,9);


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


void ISR_sensor1() {
  calculateSpeed(lastTriggerTime1, lastValidSpeed1, 1);
  /*int len = snprintf(buffer, sizeof(buffer), "%0.2f,%0.2f,%0.2f,%0.2f", lastValidSpeed1, lastValidSpeed2,lastValidSpeed3,lastValidSpeed4);
  Serial.write(buffer);
  Serial.println();*/
}

void ISR_sensor2() { calculateSpeed(lastTriggerTime2, lastValidSpeed2, 2); }
void ISR_sensor3() { calculateSpeed(lastTriggerTime3, lastValidSpeed3, 3); }
void ISR_sensor4() { calculateSpeed(lastTriggerTime4, lastValidSpeed4, 4); }


void getTFminiData(int* distance, int* strength) {
  static char i = 0;
  char j = 0;
  int checksum = 0; 
  static int rx[9];
  if(SerialTFMini.available())
  {  
    // Serial.println( "tfmini serial available" );
    rx[i] = SerialTFMini.read();
    if(rx[0] != 0x59) {
      i = 0;
    } else if(i == 1 && rx[1] != 0x59) {
      i = 0;
    } else if(i == 8) {
      for(j = 0; j < 8; j++) {
        checksum += rx[j];
      }
      if(rx[8] == (checksum % 256)) {
        *distance = rx[2] + rx[3] * 256;
        *strength = rx[4] + rx[5] * 256;
      }
      i = 0;
    } else 
    {
      i++;
    } 
  }  
}


void lidarSetup() {  
  
  // Step 1: Initialize hardware serial port (serial debug port)
  
  // wait for serial port to connect. Needed for native USB port only

  // Step 2: Initialize the data rate for the SoftwareSerial port
  SerialTFMini.begin(TFMINI_BAUDRATE);
  while (!SerialTFMini);

  // Step 3: Initialize the TF Mini sensor
  tfmini.begin(&SerialTFMini);   
}

void lidarLoop() {
  int distance = 0;
  int strength = 0;

  getTFminiData(&distance, &strength);
  while(!distance) {
    getTFminiData(&distance, &strength);
    if(distance) {
      distanceFinal = distance;
    }
  }
  delay(25);
}


void displayGPSInfo() {
  // Display basic information
  //Serial.print(F("Location: ")); 
  if (gps.location.isValid()) {
    gpsLat = gps.location.lat();
    gpsLon = gps.location.lng();
    /*Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);*/
  } else {
    gpsLat = 48.153435;
    gpsLon = 17.071734;
    //Serial.print(F("INVALID"));
  }
/*
  //Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  } else {
    Serial.print(F("INVALID"));
  }

  //Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    //Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    //Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    //Serial.print(gps.time.centisecond());
  } else {
    //Serial.print(F("INVALID"));
  }

  //Serial.println();*/
}

void gpsLoop() {
  while (GPSSerial.available() > 0) {
    if (gps.encode(GPSSerial.read())) {
      displayGPSInfo(); // Once we get a full sentence, display the info
    }
  }

  // If we haven't received any data in a while, show a message.
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  GPSSerial.begin(9600);
  while (!Serial);
  while (!Serial1);
  pinMode(hallSensorPin1, INPUT_PULLUP);
  pinMode(hallSensorPin2, INPUT_PULLUP);
  pinMode(hallSensorPin3, INPUT_PULLUP);
  pinMode(hallSensorPin4, INPUT_PULLUP);
  lidarSetup();
  //ultraSetup();
  attachInterrupt(digitalPinToInterrupt(hallSensorPin1), ISR_sensor1, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin2), ISR_sensor2, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin3), ISR_sensor3, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin4), ISR_sensor4, FALLING);
  Serial.println("Initializing....");
}

void loop() {
  lidarLoop();
  //ultraLoop();
  gpsLoop();
  int len = snprintf(buffer, sizeof(buffer), "<0.00,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.6f,%0.6f>", distanceFinal, lastValidSpeed1, lastValidSpeed2, lastValidSpeed3,lastValidSpeed4, gpsLon, gpsLat);
  Serial1.write(buffer, len);
  Serial.write(buffer, len);
  Serial.println();
}