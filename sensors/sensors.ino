#include <SoftwareSerial.h>
#include "TFMini.h"
#include <math.h>


#define diameterCm 12
#define millisToHours 3600000.00

#define HALL_PIN_FL 2
#define HALL_PIN_FR 3
#define HALL_PIN_RL 4
#define HALL_PIN_RR 5

char buffer[30];

TFMini tfmini;

SoftwareSerial SerialTFMini(4,5); //The only value that matters here is the first one, 2, Rx
// serial(1) = pin12=RX, pin13=TX
// serial(2) = pin16=RX green, pin17=TX white

unsigned int HighByte = 0;
unsigned int LowByte  = 0;
unsigned int Len  = 0;

const float wheelCircumference = M_PI * diameterCm;
volatile float distanceFinal = 0;


volatile unsigned long startTime_fl = 0;
volatile unsigned long endTime_fl = 0;
volatile float speedFinal_fl = 0;

void magnet_detect_fl() {
  endTime_fl = millis();
  unsigned long elapsedTime = endTime_fl - startTime_fl;
  float speed = ((float)wheelCircumference / 100000.00) / ((float)elapsedTime / (float)millisToHours);
  //char buf[7];
  startTime_fl = endTime_fl;

  Serial.print("Speed_fl: ");
  Serial.print(speed);
  speedFinal_fl = speed;
  Serial.println(" km/h");
  //dtostrf(speed,6,2,buf);
}


volatile unsigned long startTime_fr = 0;
volatile unsigned long endTime_fr = 0;
volatile float speedFinal_fr = 0;

void magnet_detect_fr() {
  endTime_fr = millis();
  unsigned long elapsedTime = endTime_fr - startTime_fr;
  float speed = ((float)wheelCircumference / 100000.00) / ((float)elapsedTime / (float)millisToHours);
  //char buf[7];
  startTime_fr = endTime_fr;

  Serial.print("Speed_fr: ");
  Serial.print(speed);
  speedFinal_fr = speed;
  Serial.println(" km/h");
  //dtostrf(speed,6,2,buf);
}


volatile unsigned long startTime_rl = 0;
volatile unsigned long endTime_rl = 0;
volatile float speedFinal_rl = 0;

void magnet_detect_rl() {
  endTime_rl = millis();
  unsigned long elapsedTime = endTime_rl - startTime_rl;
  float speed = ((float)wheelCircumference / 100000.00) / ((float)elapsedTime / (float)millisToHours);
  //char buf[7];
  startTime_rl = endTime_rl;

  Serial.print("Speed_rl: ");
  Serial.print(speed);
  speedFinal_rl = speed;
  Serial.println(" km/h");
  //dtostrf(speed,6,2,buf);
}


volatile unsigned long startTime_rr = 0;
volatile unsigned long endTime_rr = 0;
volatile float speedFinal_rr = 0;

void magnet_detect_rr() {
  endTime_rr = millis();
  unsigned long elapsedTime = endTime_rr - startTime_rr;
  float speed = ((float)wheelCircumference / 100000.00) / ((float)elapsedTime / (float)millisToHours);
  //char buf[7];
  startTime_rr = endTime_rr;

  Serial.print("Speed_rr: ");
  Serial.print(speed);
  speedFinal_rr = speed;
  Serial.println(" km/h");
  //dtostrf(speed,6,2,buf);
}

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

  // Step 3: Initialize the TF Mini sensor
  tfmini.begin(&SerialTFMini);   
}

void lidarLoop() 
{
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


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  while (!Serial);
  while (!Serial1);
  Serial.println("Initializing....");
  lidarSetup();
  //ultraSetup();
  /*pinMode(HALL_PIN_FL, INPUT_PULLUP);
  pinMode(HALL_PIN_FR, INPUT_PULLUP);
  pinMode(HALL_PIN_RL, INPUT_PULLUP);
  pinMode(HALL_PIN_RR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN_FL), magnet_detect_fl, FALLING);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN_FR), magnet_detect_fr, FALLING);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN_RL), magnet_detect_rl, FALLING);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN_RR), magnet_detect_rr, FALLING);*/
}

void loop() {
  lidarLoop();
  //ultraLoop();
  /*Serial.print("Distance: ");
  Serial.println(distanceFinal);*/
  int len = snprintf(buffer, sizeof(buffer), "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f", distanceFinal, speedFinal_fl,speedFinal_fr,speedFinal_rl,speedFinal_rr);
  Serial1.write(buffer, len);
  Serial.write(buffer, len);
}