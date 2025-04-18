#include <SoftwareSerial.h>
#include "TFMini.h"
#include <cmath> // For M_PI
#include "voltage.h"

#define diameterCm 12
#define millisToHours 3600000.00
#define HALL_RR 2
#define HALL_RL 3
#define HALL_FR 4
#define HALL_FL 5

const int hallSensorPin1 = HALL_RR;
const int hallSensorPin2 = HALL_RL; 
const int hallSensorPin3 = HALL_FR; 
const int hallSensorPin4 = HALL_FL;

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

// Voltmeter variables
volatile float adc0voltage = 0.0;
volatile float adc1voltage = 0.0;
volatile float adc2voltage = 0.0;

// Wheel diameter in meters (0.12m)
const float wheelDiameter = 0.12;
// Calculate circumference (C = pi * d) using M_PI for Pi
const float wheelCircumference = M_PI * wheelDiameter;

// Maximum allowed speed change in km/h to filter inaccuracies
const float maxSpeedChange = 10.0;

volatile float distanceFinal = 0.0;

unsigned int HighByte1 = 0;
unsigned int LowByte1 = 0;
unsigned int HighByte2 = 0;
unsigned int LowByte2 = 0;
volatile float rearLen = 0.0;
volatile float frontLen = 0.0;

// Timing variables to control measurement and delay without blocking
unsigned long lastMeasurementTime1 = 0;
unsigned long lastMeasurementTime2 = 0;
const long measurementInterval = 10; // Time between measurements
const long readDelay = 50; // Delay after triggering measurement
bool waitingForRead1 = false; // State to track if we are waiting to read data
bool waitingForRead2 = false; // State to track if we are waiting to read data

char buffer[200];
char buffer2[80];

TFMini tfmini;

SoftwareSerial SerialTFMini(6,7); //The only value that matters here is the first one, 2, Rx
// serial(1) = pin12=RX, pin13=TX
// serial(2) = pin16=RX green, pin17=TX white


SoftwareSerial HallSerial(10,11);
SoftwareSerial UltraSerial1(12,13); 
SoftwareSerial UltraSerial2(14,15);


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


void ISR_sensor1() { calculateSpeed(lastTriggerTime1, lastValidSpeed1, 1); }
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

void ultraLoop1() {
  unsigned long currentTime = millis();
  
  // Check if it's time to take a new measurement
  if (!waitingForRead1 && currentTime - lastMeasurementTime1 >= measurementInterval) {
    lastMeasurementTime1 = currentTime;
    UltraSerial1.flush();
    UltraSerial1.write(0X55); // Trigger US-100 to measure distance
    waitingForRead1 = true;
  }

  // Wait for readDelay time before attempting to read data
  if (waitingForRead1 && currentTime - lastMeasurementTime1 >= readDelay) {
    if (UltraSerial1.available() >= 2) { // Check if 2 bytes are available to read
      
      HighByte1 = UltraSerial1.read();
      LowByte1 = UltraSerial1.read();
      frontLen = HighByte1 * 256 + LowByte1; // Calculate the distance
      
      waitingForRead1 = false; // Reset the read waiting flag
    }
    else {
      // No data available yet, check again in the next loop iteration
      waitingForRead1 = false; // Allow re-triggering the measurement if no data was read
    }
  }
}

void ultraLoop2() {
  unsigned long currentTime = millis();
  
  // Check if it's time to take a new measurement
  if (!waitingForRead2 && currentTime - lastMeasurementTime2 >= measurementInterval) {
    lastMeasurementTime2 = currentTime;
    UltraSerial2.flush();
    UltraSerial2.write(0X55); // Trigger US-100 to measure distance
    waitingForRead2 = true;
  }

  // Wait for readDelay time before attempting to read data
  if (waitingForRead2 && currentTime - lastMeasurementTime2 >= readDelay) {
    if (UltraSerial2.available() >= 2) { // Check if 2 bytes are available to read
      
      HighByte2 = UltraSerial2.read();
      LowByte2 = UltraSerial2.read();
      rearLen = HighByte2 * 256 + LowByte2; // Calculate the distance
      
      waitingForRead2 = false; // Reset the read waiting flag
    }
    else {
      // No data available yet, check again in the next loop iteration
      waitingForRead2 = false; // Allow re-triggering the measurement if no data was read
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);
  HallSerial.begin(115200);
  UltraSerial1.begin(9600);
  UltraSerial2.begin(9600);
  while (!Serial1);
  while (!UltraSerial1);
  while (!UltraSerial2);
  pinMode(hallSensorPin1, INPUT_PULLUP);
  pinMode(hallSensorPin2, INPUT_PULLUP);
  pinMode(hallSensorPin3, INPUT_PULLUP);
  pinMode(hallSensorPin4, INPUT_PULLUP);
  lidarSetup();
  initVoltmeter();
  attachInterrupt(digitalPinToInterrupt(hallSensorPin1), ISR_sensor1, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin2), ISR_sensor2, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin3), ISR_sensor3, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin4), ISR_sensor4, FALLING);
}


void loop() {
  lidarLoop();
  ultraLoop1();
  ultraLoop2();
  readVoltages();


  volatile unsigned long currentTime = micros(); // Get current time once for efficiency

  // Check if more than 2 seconds have passed since the last trigger for each sensor
  if (currentTime - lastTriggerTime1 > 2000000) {
    lastValidSpeed1 = 0.0; // Reset the last valid speed if no trigger within 2 seconds
    lastValidSpeed2 = 0.0; // Reset the last valid speed if no trigger within 2 seconds
    lastValidSpeed3 = 0.0; // Reset the last valid speed if no trigger within 2 seconds
    lastValidSpeed4 = 0.0; // Reset the last valid speed if no trigger within 2 seconds
  }
  
  volatile float meanSpeed;
  // For mean speed we are using only rear sensor as only those wheels are driven
  meanSpeed = (lastValidSpeed1 + lastValidSpeed2) / 2;

    
  int len = snprintf(buffer, sizeof(buffer), "<%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f>\r\n", frontLen / 10, rearLen / 10, distanceFinal, lastValidSpeed1, lastValidSpeed1, lastValidSpeed2,lastValidSpeed2, meanSpeed, adc0voltage, adc1voltage, adc2voltage);
  int len2 = snprintf(buffer2, sizeof(buffer2), "<%0.2f,>\r",meanSpeed);
  HallSerial.write(buffer2, len2);
  Serial.write(buffer, len);
}

