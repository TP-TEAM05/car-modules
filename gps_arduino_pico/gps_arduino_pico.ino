#include <TinyGPS++.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

// Create a TinyGPS++ object
TinyGPSPlus gps;

SoftwareSerial GPSSerial(8,9);

void setup() {
  Serial.begin(9600);   // Start serial communication with the computer  // Start serial communication with the GPS module on GPSSerial
  GPSSerial.begin(9600);
  Serial.println(F("Device is starting..."));
  Serial.println(F("GPS Module test with TinyGPS++ library"));
}

void loop() {
  // Continuously feed all available characters from the GPS module to the TinyGPS++ object
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

void displayGPSInfo() {
  // Display basic information
  Serial.print(F("Location: ")); 
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
