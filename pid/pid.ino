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
// Define the PID variables
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp=2.0, Ki=5.0, Kd=1.0;
const byte numChars = 32; // Adjusted to accommodate larger input strings
char receivedChars[numChars];
char tempChars[numChars];    
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
// SoftwareSerial for communication with another device
SoftwareSerial mySerial(RX_PIN, TX_PIN);
// Servo setup
Servo throttleServo;
Servo dir;
boolean newData = false;

void throttle_init () {
  throttleServo.attach (2);

  // ESC init sequence
  throttleServo.writeMicroseconds  ( THROTTLE_MAXFORWARD );
  delay (25);
  throttleServo.writeMicroseconds  ( THROTTLE_MAXREVERSE );
  delay (25);
  throttleServo.writeMicroseconds  ( THROTTLE_IDLE );
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;

    // The start and end of new data is specified by < and > respectively
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (mySerial.available() > 0 && newData == false) {
        rc = mySerial.read();

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

void parseData() {
    // Split the received data into its components
    if (newData) {
      strcpy(tempChars, receivedChars);
      char * strtokIndx = strtok(tempChars, ","); // isLeft
      Input = atof(strtokIndx);
      newData = false;
    }

    // You might need to add logic here to handle isForward and percentageThrottle if used
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
    Setpoint = Serial.parseFloat();
    Serial.print("Setpoint: ");
    Serial.println(Setpoint);
  }

  // Check for current speed input
  if (mySerial.available()) {
    Serial.print("Current Speed: ");
    recvWithStartEndMarkers();
    parseData();
    Serial.println(Input);
  }

  // Compute PID
  myPID.Compute();
  
  int intOut = int(Output);
  Serial.println(intOut);
  throttleServo.writeMicroseconds(intOut);
}
