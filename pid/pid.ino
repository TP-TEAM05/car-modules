#include <PID_v1_bc.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// Define RX and TX pins for the SoftwareSerial
#define RX_PIN 4
#define TX_PIN 5
// Throttle ranges
#define THROTTLE_MAXREVERSE  1000
#define THROTTLE_MAXFORWARD  2000
#define THROTTLE_IDLE        1500


#define MAXLEFT   2000
#define MAXRIGHT  1000
#define MIDDLE    1500
// Define the PID variables
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp=2.0, Ki=5.0, Kd=1.0;
const byte numChars = 32; // Adjusted to accommodate larger input strings
char receivedChars[numChars];
char tempChars[numChars];    
const byte numChars2 = 32; // Adjusted to accommodate larger input strings
char receivedChars2[numChars];
char tempChars2[numChars];    
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
// SoftwareSerial for communication with another device
SoftwareSerial mySerial(RX_PIN, TX_PIN);
// Servo setup
Servo throttleServo;
Servo dir;
boolean newData = false;
boolean newData2 = false;


int isLeft = 0;

int percentageTurn = 0;

void throttle_init () {
  throttleServo.attach (2);

  // ESC init sequence
  throttleServo.writeMicroseconds  ( THROTTLE_MAXFORWARD );
  delay (25);
  throttleServo.writeMicroseconds  ( THROTTLE_MAXREVERSE );
  delay (25);
  throttleServo.writeMicroseconds  ( THROTTLE_IDLE );
}


// Function to set a custom PWM value
void dir_set (int pulse) {
  dir.writeMicroseconds (pulse);
}

// Function to turn the wheels left at percentage specified by argument (e.g. 20,50,100)
void dir_leftPct (int pct) {
  // Alterations to this formula are found at multiple points in the project
  // This formula ensures the calculation of the right PWM value from the constants set at the start of the program
  // Firstly we calculate the absolute diference in PWM value from the middle point and get the wanted percantage value
  // Secondly, we want to  turn left (so the range of PWM is (1500,2000>), therefore we add the percentage value to the middle value.
  dir.writeMicroseconds ( int( double(MIDDLE + ( double(abs(MIDDLE-MAXLEFT))*(double(pct)/100) ))) );
}

// Function to turn the wheels right at percentage specified by argument (e.g. 20,50,100)
void dir_rightPct (int pct) {
  // Alterations to this formula are found at multiple points in the project
  // This formula ensures the calculation of the right PWM value from the constants set at the start of the program
  // Firstly we calculate the absolute diference in PWM value from the middle point and get the wanted percantage value
  // Secondly, we want to  turn right (so the range of PWM is <1000,1500)), therefore we subtract the percentage value to the middle value.
  dir.writeMicroseconds ( int( double(MIDDLE - ( double(abs(MIDDLE-MAXRIGHT))*(double(pct)/100) ))) );
}

// Function to immediately set the servo to middle state
void dir_straight () {
  dir.writeMicroseconds ( MIDDLE );
}




void recvWithStartEndMarkersHall() {
    static boolean recvInProgress = false;
    static byte ndx = 0;

    // The start and end of new data is specified by < and > respectively
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (mySerial.available() > 0 && newData2 == false) {
        rc = mySerial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars2[ndx] = rc;
                ndx++;
                if (ndx >= numChars2) {
                    ndx = numChars2 - 1;
                }
            }
            else {
                receivedChars2[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData2 = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseDataHall() {
    // Split the received data into its components
    if (newData2) {
      strcpy(tempChars2, receivedChars2);
      newData2 = false;

      char * strtokIndx; // this is used by strtok() as an index
      strtokIndx = strtok(tempChars2,",");      // get the first part - the thrust
      Input = atof(strtokIndx);     
    }
}


void recvWithStartEndMarkersControl() {
    static boolean recvInProgress = false;
    static byte ndx = 0;

    // The start and end of new data is specified by < and > respectively
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseDataControl() {
    // Split the received data into its components
    if (newData) {
      strcpy(tempChars, receivedChars);
      newData = false;

      char * strtokIndx; // this is used by strtok() as an index
      // get the first part - isLeft
      strtokIndx = strtok(tempChars,","); // this continues where the previous call left off 
      isLeft = atoi(strtokIndx); 
      // get the second part - km/h
      strtokIndx = strtok(NULL,",");      // get the first part - the thrust
      Setpoint = atof(strtokIndx); 
      // get the third part - turn
      strtokIndx = strtok(NULL, ","); 
      percentageTurn = atoi(strtokIndx);     
    }
}

void setup() {
  // Start the serial communication
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect
  mySerial.begin(115200);

  throttle_init();
  dir.attach (3);
  // Initialize the PID
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(1500, 2000); // Limits for servo control

  Setpoint = 0; // Initial desired speed
}

void loop() {
  // Check for setpoint input
  if (Serial.available()) {
    Serial.print("Current Speed: ");
    recvWithStartEndMarkersControl();
    parseDataControl();
    Serial.println(Setpoint);
  }

  // Check for current speed input
  if (mySerial.available()) {
    Serial.print("Current Speed: ");
    recvWithStartEndMarkersHall();
    parseDataHall();
    Serial.println(Input);
  }

    // If the turn is set to 0, go straight
  if (percentageTurn == 0) {
    dir_straight();
  }
  
  // If the isLeft is set to 0, we go right
  if (isLeft == 0) {
    dir_leftPct(percentageTurn);
  } else if (isLeft == 1) {
    // else we go left
    dir_rightPct(percentageTurn);  
  }

  // Compute PID
  myPID.Compute();
  
  int intOut = int(Output);
  Serial.println(intOut);
  throttleServo.writeMicroseconds(intOut);
}
