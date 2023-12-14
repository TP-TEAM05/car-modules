#include <SoftwareSerial.h>
#include "TFMini.h"
#include <math.h>


#define diameterCm 12
#define millisToHours 3600000.00

#define HALL_PIN 2 

TFMini tfmini;

SoftwareSerial SerialTFMini(1,0); //The only value that matters here is the first one, 2, Rx
// serial(1) = pin12=RX, pin13=TX
// serial(2) = pin16=RX green, pin17=TX white
SoftwareSerial SerialWrite(4,5);

unsigned int HighByte = 0;
unsigned int LowByte  = 0;
unsigned int Len  = 0;

const float wheelCircumference = M_PI * diameterCm;


volatile unsigned long startTime = 0;
volatile unsigned long endTime = 0;

volatile float distanceFinal = 0;
volatile float speedFinal = 0;

void magnet_detect() {
  endTime = millis();
  unsigned long elapsedTime = endTime - startTime;
  float speed = ((float)wheelCircumference / 100000.00) / ((float)elapsedTime / (float)millisToHours);
  char buf[7];
  startTime = endTime;

  Serial.print("Speed: ");
  Serial.print(speed);
  speedFinal = speed;
  Serial.println(" km/h");
  dtostrf(speed,6,2,buf);
  SerialWrite.write("<1,>");
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
      Serial.print(distance);
      distanceFinal = distance;
      Serial.print("cm\t");
      Serial.print("strength: ");
      Serial.println(strength);
    }
  }
  SerialWrite.write("<0,>");
  delay(1000);
}

void ultraSetup() {
  SerialWrite.begin(9600);
}

void ultraLoop() {
  SerialWrite.flush();
  SerialWrite.write(0X55);                           // trig US-100 begin to measure the distance
  delay(500);                                  
  if (SerialWrite.available() >= 2)                  // check receive 2 bytes correctly
  {
    HighByte = SerialWrite.read();
    LowByte  = SerialWrite.read();
    Len  = HighByte * 256 + LowByte;          // Calculate the distance
    if ((Len > 1) && (Len < 10000))
    {
      Serial.print("Distance: ");
      Serial.print(Len, DEC);          
      Serial.println("mm");                  
    }
  }
  delay(300);                                    
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Initializing....");
  lidarSetup();
  //ultraSetup();
  pinMode(HALL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), magnet_detect, FALLING);
  SerialWrite.begin(115200);
}

void loop() {
  lidarLoop();
  //ultraLoop();
}