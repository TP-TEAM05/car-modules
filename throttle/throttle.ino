/*
This is a firmware code used by the students of Faculty of Informatics and Information Technologies at Slovak Technical University (FIIT STU), used at ReCo project.
We disclose the use and modification of the Arduino code by Fabien Papleux at https://github.com/fpapleux/mcqueen. 
The author's program was edited to work with the pinout of Raspberry Pi Pico and communicate via Serial with another device that sends instructions.
As the original code is under the GNU General Public License v2.0, our code also adheres to this license.
*/

#include <Servo.h>

// The servo responds to PWM signal in range range 1000 - 2000
#define THROTTLE_MAXREVERSE  1000
#define THROTTLE_MAXFORWARD  2000
#define THROTTLE_IDLE        1500
// pin 2 on Raspberry Pi Pico
#define THROTTLEPIN          2

// The servo responds to PWM signal in range range 1000 - 2000
#define MAXLEFT   2000
#define MAXRIGHT  1000
#define MIDDLE    1500
// pin 3 on Raspberry Pi Pico
#define DIRPIN   19

//Object to abstract the directional servo
Servo dir;
//Object to abstract the throttle servo
Servo throttle;

boolean newData = false;

// Holds the last sent PWM value
int throttle_pulse;

const byte numChars = 13;
char receivedChars[numChars];
char tempChars[numChars];     

int isLeft = 0;
int isForward = 0;

int percentageTurn = 0;
int percentageThrottle = 0;

// Initialize the ESC, this sequence was found as-is, DO NOT MODIFY
void throttle_init () {
  throttle.attach (THROTTLEPIN);

  // ESC init sequence
  throttle_set ( THROTTLE_MAXFORWARD );
  delay (25);
  throttle_set ( THROTTLE_MAXREVERSE );
  delay (25);
  throttle_set ( THROTTLE_IDLE );

  throttle_pulse = THROTTLE_IDLE;
}

// Function to set a custom PWM value
void throttle_set (int pulse) {
  throttle.writeMicroseconds (pulse);
  throttle_pulse = pulse;
}

// Function to move forward at percentage specified by argument (e.g. 20,50,100)
void throttle_forwardPct (int pct) {

  // Alterations to this formula are found at multiple points in the project
  // This formula ensures the calculation of the right PWM value from the constants set at the start of the program
  // Firstly we calculate the absolute diference in PWM value from the idle point and get the wanted percantage value
  // Secondly, we want to move forward (so the range of PWM is (1500,2000>), therefore we add the percentage value to the idle value.
  int newPulse = int( double(THROTTLE_IDLE + ( double(abs(THROTTLE_IDLE - THROTTLE_MAXFORWARD))*(double(pct)/100) )));

  throttle.writeMicroseconds ( newPulse );
  throttle_pulse = newPulse;
}

// Function to move backwards at percentage specified by argument (e.g. 20,50,100)
void throttle_reversePct (int pct) {
  // Alterations to this formula are found at multiple points in the project
  // This formula ensures the calculation of the right PWM value from the constants set at the start of the program
  // Firstly we calculate the absolute diference in PWM value from the idle point and get the wanted percantage value
  // Secondly, we want to move backwards (so the range of PWM is <1000,1500)), therefore we subtract the percentage value to the idle value.
  int newPulse = int( double(THROTTLE_IDLE - ( double(abs(THROTTLE_IDLE - THROTTLE_MAXREVERSE))*(double(pct)/100) )));

  throttle.writeMicroseconds ( newPulse );
  throttle_pulse = newPulse;
}

// Function to immediately set the servo to idle state
void throttle_idle () {
  throttle.writeMicroseconds ( THROTTLE_IDLE );
  throttle_pulse = THROTTLE_IDLE;
}

// Work in Progress (WIP) function to apply brakes (put the servo in reverse for a brief period of time) 
void throttle_stop_WIP () {
  throttle.writeMicroseconds ( THROTTLE_IDLE );
  throttle_pulse = THROTTLE_IDLE;
}



// Initialize the ESC, DO NOT MODIFY
void dir_init () {
  dir.attach (DIRPIN);
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


// Function to parse the data received by Serial communication
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;

    // The start and end of new data is specified by < and > respectively
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial1.available() > 0 && newData == false) {
        rc = Serial1.read();

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


// Function to split the data into its parts, the data format is <isLeft,isForward,percentageThrottle,degreestToTurn>, e.g. <1,1,25,100> - we turn left at maximum rotation of wheels and forward at 25% throttle
void parseData() {
    char * strtokIndx; // this is used by strtok() as an index
    // get the first part - isLeft
    strtokIndx = strtok(tempChars,","); // this continues where the previous call left off 
    isLeft = atoi(strtokIndx); 
    // get the second part - isForward
    strtokIndx = strtok(NULL,",");      
    isForward = atoi(strtokIndx); 
    // get the third part - throttle
    strtokIndx = strtok(NULL,",");      // get the first part - the thrust
    percentageThrottle = atoi(strtokIndx); 
    // get the fourth part - turn
    strtokIndx = strtok(NULL, ","); 
    percentageTurn = atoi(strtokIndx);     
}



// Function runs once at the start of the program
void setup()
{
  // Initialize both servos
  dir_init ();
  throttle_init();

  // Initialize the Serial communication with external device (e.g. Raspberry Pi) at baud rate 115 200 (modify at the risk of increasing system latency)
  Serial1.begin(115200);
  // Initalize the Serial commication with connected Serial Monitor (e.g. in Arduino IDE)
  Serial.begin(9600);
}





// Function runs with every loop of the program
void loop()
{
    // Wait to receive data
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars); // this temporary copy is necessary to protect the original data, because strtok() used in parseData() replaces the commas with \0
        parseData(); // split the data into its parts
        Serial.print(isLeft);
        Serial.print(",");
        Serial.print(isForward);
        Serial.print(",");
        Serial.print(percentageThrottle);
        Serial.print(",");
        Serial.println(percentageTurn);
      
        // If the turn is set to 0, go straight
        if (percentageTurn == 0) {
          dir_straight();
        }

        // If the isForward is set to 0, we go backwards
        if (isForward == 0) {
          throttle_reversePct(percentageThrottle);
        } else  if (isForward == 1) {
          // else we go forward
          throttle_forwardPct(percentageThrottle);
        }
        newData = false;
    }
  
}