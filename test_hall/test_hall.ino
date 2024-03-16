#define diameterCm 12
#define millisToHours 3600000.00
#include <math.h>

const float wheelCircumference = M_PI * diameterCm;


volatile unsigned long startTime = 0;
volatile unsigned long endTime = 0;

void magnet_detect() {
  endTime = millis();
  unsigned long elapsedTime = endTime - startTime;
  float speed = ((float)wheelCircumference / 100000.00) / ((float)elapsedTime / (float)millisToHours);

  startTime = endTime;

  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.println(" km/h");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Hello");
  
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), magnet_detect, FALLING);
}

void loop() {}
