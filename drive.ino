#include <Servo.h>

#define THROTTLE_MAXREVERSE  1000
#define THROTTLE_MAXFORWARD  2000
#define THROTTLE_IDLE        1500
#define THROTTLEPIN          2

#define MAXLEFT   2000
#define MAXRIGHT  1000
#define MIDDLE    1500
#define DIRPIN    3

Servo dir;

boolean newData = false;
Servo throttle;
int throttle_pulse;

const byte numChars = 13;
char receivedChars[numChars];
char tempChars[numChars];     

int isLeft = 0;
int isForward = 0;

int degreesToTurn = 0;
int parsedThrottle = 0;

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

void throttle_set (int pulse) {
  throttle.writeMicroseconds (pulse);
  throttle_pulse = pulse;
}

void throttle_forwardPct (int pct) {
  int newPulse = int( double(THROTTLE_IDLE + ( double(abs(THROTTLE_IDLE - THROTTLE_MAXFORWARD))*(double(pct)/100) )));

  throttle.writeMicroseconds ( newPulse );
  throttle_pulse = newPulse;
}

void throttle_reversePct (int pct) {
  int newPulse = int( double(THROTTLE_IDLE - ( double(abs(THROTTLE_IDLE - THROTTLE_MAXREVERSE))*(double(pct)/100) )));
/*
  if (throttle_pulse >= THROTTLE_IDLE) {
    throttle_idle ();
    delay (100);
    throttle.writeMicroseconds ( newPulse );
    delay (100);
    throttle_idle ();
    delay (100);
  }*/
  throttle.writeMicroseconds ( newPulse );
  throttle_pulse = newPulse;
}

void throttle_idle () {
  throttle.writeMicroseconds ( THROTTLE_IDLE );
  throttle_pulse = THROTTLE_IDLE;
}

void throttle_stop () {
  /*
  int i;
  if (throttle_pulse > THROTTLE_IDLE) {


    for (i=throttle_pulse; i > THROTTLE_IDLE; i-=10) {
      throttle_set (i);
      delay (20);
    }
  }
  else if (throttle_pulse < THROTTLE_IDLE) {
    for (i=throttle_pulse; i < THROTTLE_IDLE; i+=10) {
      throttle_set (i);
      delay (20);
    }
  }
  */
  throttle.writeMicroseconds ( THROTTLE_IDLE );
  throttle_pulse = THROTTLE_IDLE;
}




void dir_init () {
  dir.attach (DIRPIN);
}

void dir_set (int pulse) {
  dir.writeMicroseconds (pulse);
}

void dir_leftPct (int pct) {
  dir.writeMicroseconds ( int( double(MIDDLE + ( double(abs(MIDDLE-MAXLEFT))*(double(pct)/100) ))) );
}

void dir_rightPct (int pct) {
  dir.writeMicroseconds ( int( double(MIDDLE - ( double(abs(MIDDLE-MAXRIGHT))*(double(pct)/100) ))) );
}

void dir_straight () {
  dir.writeMicroseconds ( MIDDLE );
}


long o;

void setup()
{
  dir_init ();
  throttle_init();
  o = 0;
  Serial1.begin(115200);
  Serial.begin(9600);
}


void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
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

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the thrust
    isLeft = atoi(strtokIndx); 
    
    strtokIndx = strtok(NULL,",");      // get the first part - the thrust
    isForward = atoi(strtokIndx); 
        // convert this part to an integer

    strtokIndx = strtok(NULL,",");      // get the first part - the thrust
    parsedThrottle = atoi(strtokIndx); 

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    degreesToTurn = atoi(strtokIndx);     // convert this part to an integer


}




void loop()
{


    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        Serial.print(isLeft);
        Serial.print(",");
        Serial.print(isForward);
        Serial.print(",");
        Serial.print(parsedThrottle);
        Serial.print(",");
        Serial.println(degreesToTurn);
        /*if (isThrust == 0 && degreesToTurn == 0) {
          digitalWrite(LED_BUILTIN, LOW);
        } 
        if (isThrust == 1 && degreesToTurn == 180) {
          digitalWrite(LED_BUILTIN, HIGH);
        }
        if (isThrust == 1) {
          driveServo.writeMicroseconds(1580);
        } else if (isThrust == 0) {
           driveServo.writeMicroseconds(1500);
        }*/
      
        if (parsedThrottle == 0) {
          throttle_stop();
        }
        if (degreesToTurn == 0) {
          dir_straight();
        }
        
        if (isLeft == 0) {
          dir_leftPct(degreesToTurn);
        } else if (isLeft == 1) {
          dir_rightPct(degreesToTurn);  
        }
        if (isForward == 0) {
          throttle_reversePct(parsedThrottle);
        } else  if (isForward == 1) {
          throttle_forwardPct(parsedThrottle);
        }
        newData = false;
    }
  
}