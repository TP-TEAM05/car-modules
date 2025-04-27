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

#define SAFETY_TIMEOUT 1000 // 1 second timeout for safety

// Define the PID variables
double SpeedSetpoint, SpeedInput, PwmOutput;

unsigned int LastControlsUpdate = 0; // Last time the setpoint was updated

// Used for counting how many times in a row was detected 0 as input
int inputZeroCount = 0;
int setpointZeroCount = 0;

// Treshold when the speed is reset
#define ZERO_INPUT_TRESHOLD 10
#define ZERO_SETPOINT_TRESHOLD ZERO_INPUT_TRESHOLD
#define LAST_INPUT_TRESHOLD 2500
#define LAST_SETPOINT_TRESHOLD LAST_INPUT_TRESHOLD

// Track time from last serial input
unsigned long startBrakingTime = 0;

// Specify the links and initial tuning parameters
double Kp = 2.0, Ki = 5.0, Kd = 1.0;
const byte numChars2 = 32; // Adjusted to accommodate larger input strings
char receivedChars2[numChars2];
char tempChars2[numChars2];
PID speedPid(&SpeedInput, &PwmOutput, &SpeedSetpoint, Kp, Ki, Kd, DIRECT);
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

void reset_movement()
{
  SpeedSetpoint = 0;
  throttleServo.writeMicroseconds(THROTTLE_IDLE);
  return THROTTLE_IDLE;
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
    SpeedInput = atof(strtokIndx);
  }
}

void receiveControls()
{
  static bool reading = false; // true while inside '< … >'
  static String payload = "";  // collects the characters

  while (Serial1.available())
  {
    char c = Serial1.read();

    // start of frame -----------------------------------------------------------
    if (c == '<')
    {
      reading = true;
      payload = ""; // reset buffer
      continue;     // don't store the '<'
    }

    // end of frame -------------------------------------------------------------
    if (c == '>' && reading)
    {
      reading = false;

      // ── 1. echo the received string on a new line
      Serial.println(payload); // e.g. "3.00,90"

      // ── 2. parse the two comma‑separated values
      int commaPos = payload.indexOf(',');
      if (commaPos > 0)
      { // basic sanity check
        String speedStr = payload.substring(0, commaPos);
        String angleStr = payload.substring(commaPos + 1);

        SpeedSetpoint = speedStr.toFloat(); // "3.00"  →  3.00
        degrees = angleStr.toInt();         // "90"    →  90

        LastControlsUpdate = millis();

        if (SpeedSetpoint == 0.00)
        {
          setpointZeroCount++;
        }
        else
        {
          setpointZeroCount = 0;
        }
      }
      return; // finished with this frame
    }

    // inside frame – accumulate characters ------------------------------------
    if (reading)
    {
      payload += c; // grow the string
    }
  }
}

void setup()
{
  // Start the serial communication
  Serial.begin(115200);
  Serial1.begin(115200);  // Serial1 is for receiving control data
  mySerial.begin(115200); // Serial for receiving speed data

  Serial1.setTimeout(200); // 200ms timeout for Serial1

  throttle_init();
  seteeringServo.attach(3);
  steering_set(MIDDLE);
  // Initialize the PID
  speedPid.SetMode(AUTOMATIC);
  speedPid.SetOutputLimits(1500, 2000); // Limits for servo control

  SpeedSetpoint = 0; // Initial desired speed
}

void loop()
{

  // Check for controls data
  receiveControls();

  // Check for current speed input from hall sensors
  if (mySerial.available())
  {
    recvWithStartEndMarkersHall();
    parseDataHall();
  }

  steering_set(degrees);

  // Compute PID
  speedPid.Compute();

  int intOut = int(PwmOutput);

  if (intOut < 1580 && SpeedSetpoint == 0 && SpeedInput == 0.00)
  {
    reset_movement();
    return;
  }

  if (setpointZeroCount > ZERO_SETPOINT_TRESHOLD || SpeedSetpoint > ZERO_INPUT_TRESHOLD)
  {
    reset_movement();
    setpointZeroCount = 0;
    inputZeroCount = 0;
    return;
  }

  // Automatically reset to idle, when not received any data for 1 second
  if (millis() - LastControlsUpdate > SAFETY_TIMEOUT)
  {
    reset_movement();
    return;
  }

  // if (SpeedSetpoint == 0 && SpeedInput > 1 && startBrakingTime == 0)
  // {
  //   startBrakingTime = millis();
  //   intOut = THROTTLE_IDLE;
  // }
  // else if (SpeedSetpoint == 0 && SpeedInput > 1 && millis() - startBrakingTime < 1000)
  // {
  //   int pwmDiff = THROTTLE_MAXFORWARD - intOut;
  //   intOut = THROTTLE_IDLE - pwmDiff / 2;
  // }
  // else
  // {
  //   inputZeroCount = 0;
  //   startBrakingTime = 0;
  // }

  throttleServo.writeMicroseconds(intOut);
}
