#include <PID_v1_bc.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// Define RX and TX pins for the SoftwareSerial
#define RX_PIN 4
#define TX_PIN 5
// Throttle ranges
#define THROTTLE_MAXREVERSE 1000
#define THROTTLE_MAXFORWARD 2000
#define THROTTLE_IDLE 1500

#define MAXLEFT 180
#define MAXRIGHT 0
#define MIDDLE 90
// Define the PID variables
double Setpoint, Input, Output;

// Track elapsed time from last movement change
unsigned long movementStartTime = 0;
unsigned long movementElapsedTime = 0;

// Specify the links and initial tuning parameters
double Kp = 2.0, Ki = 5.0, Kd = 1.0;
const byte numChars = 32; // Adjusted to accommodate larger input strings
char receivedChars[numChars];
char tempChars[numChars];
const byte numChars2 = 32; // Adjusted to accommodate larger input strings
char receivedChars2[numChars];
char tempChars2[numChars];
PID speedPid(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
// SoftwareSerial for communication with another device
SoftwareSerial mySerial(RX_PIN, TX_PIN);
// Servo setup
Servo throttleServo;
Servo seteeringServo;
boolean newData = false;
boolean newData2 = false;

int degrees = MIDDLE;

void throttle_init()
{
  throttleServo.attach(2);

  // ESC init sequence
  throttleServo.writeMicroseconds(THROTTLE_MAXFORWARD);
  delay(25);
  throttleServo.writeMicroseconds(THROTTLE_MAXREVERSE);
  delay(25);
  throttleServo.writeMicroseconds(THROTTLE_IDLE);
}

// Function to set a custom PWM value
void steering_set(int deg)
{
  seteeringServo.write(deg);
}

// Function to turn the wheels left at percentage specified by argument (e.g. 20,50,100)
void steering_left_percent(int pct)
{
  // Alterations to this formula are found at multiple points in the project
  // This formula ensures the calculation of the right PWM value from the constants set at the start of the program
  // Firstly we calculate the absolute diference in PWM value from the middle point and get the wanted percantage value
  // Secondly, we want to  turn left (so the range of PWM is (1500,2000>), therefore we add the percentage value to the middle value.
  seteeringServo.write(int(double(MIDDLE + (double(abs(MIDDLE - MAXLEFT)) * (double(pct) / 100)))));
}

// Function to turn the wheels right at percentage specified by argument (e.g. 20,50,100)
void steering_right_percent(int pct)
{
  // Alterations to this formula are found at multiple points in the project
  // This formula ensures the calculation of the right PWM value from the constants set at the start of the program
  // Firstly we calculate the absolute diference in PWM value from the middle point and get the wanted percantage value
  // Secondly, we want to  turn right (so the range of PWM is <1000,1500)), therefore we subtract the percentage value to the middle value.
  seteeringServo.write(int(double(MIDDLE - (double(abs(MIDDLE - MAXRIGHT)) * (double(pct) / 100)))));
}

// Function to immediately set the servo to middle state
void steering_straight()
{
  seteeringServo.write(MIDDLE);
}

void reset_movement() 
{

}

void recvWithStartEndMarkersHall()
{
  static boolean recvInProgress = false;
  static byte ndx = 0;

  // The start and end of new data is specified by < and > respectively
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (mySerial.available() > 0 && newData2 == false)
  {
    rc = mySerial.read();

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        receivedChars2[ndx] = rc;
        ndx++;
        if (ndx >= numChars2)
        {
          ndx = numChars2 - 1;
        }
      }
      else
      {
        receivedChars2[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData2 = true;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

void parseDataHall()
{
  // Split the received data into its components
  if (newData2)
  {
    strcpy(tempChars2, receivedChars2);
    newData2 = false;

    char *strtokIndx;                     // this is used by strtok() as an index
    strtokIndx = strtok(tempChars2, ","); // get the first part - the thrust
    Input = atof(strtokIndx);
  }
}

void recvWithStartEndMarkersControl()
{
  static boolean recvInProgress = false;
  static byte ndx = 0;

  // The start and end of new data is specified by < and > respectively
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial1.available() > 0 && newData == false)
  {
    rc = Serial1.read();

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars)
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

void parseDataControl()
{
  // Split the received data into its components
  if (newData)
  {
    strcpy(tempChars, receivedChars);
    newData = false;

    char *strtokIndx; // this is used by strtok() as an index
    // get the second part - km/h
    strtokIndx = strtok(tempChars, ","); // get the first part - the thrust
    Setpoint = atof(strtokIndx);
    // get the third part - turn
    strtokIndx = strtok(NULL, ",");
    degrees = atoi(strtokIndx);
  }
}

void setup()
{
  // Start the serial communication
  Serial.begin(115200);
  Serial1.begin(115200);
  mySerial.begin(115200);

  throttle_init();
  seteeringServo.attach(3);
  // Initialize the PID
  speedPid.SetMode(AUTOMATIC);
  speedPid.SetOutputLimits(1500, 2000); // Limits for servo control

  Setpoint = 0; // Initial desired speed
}

void loop()
{ 
  // Variable that track state if there was any input from serial
  bool serialControlReceived = false;

  // Check for setpoint input
  if (Serial1.available())
  {
    Serial.print("Setpoint: ");
    recvWithStartEndMarkersControl();
    parseDataControl();
    //Serial.println(Setpoint);
    serialControlReceived = true;
    movementStartTime = millis();
  }

  // Check for current speed input
  if (mySerial.available())
  {
    Serial.print("Current Speed: ");
    recvWithStartEndMarkersHall();
    parseDataHall();
    Serial.println(Input);
  }

  if (!serialControlReceived) {
    movementElapsedTime = millis() - movementStartTime;
  }

  steering_set(degrees);

  // Compute PID
  speedPid.Compute();

  int intOut = int(Output);
  // If the time between new speed is more than 2500 milliseconds, we set the car to zero speed
  if (movementElapsedTime > 2500)
  {
    intOut = 1500;  
  }
  if (intOut < 1580 && Setpoint == 0 && Input == 0.00)
  {
    intOut = 1500;
  }

  throttleServo.writeMicroseconds(intOut);
}
