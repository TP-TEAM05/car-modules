char buffer[30];
volatile float distanceFinal = 0;
volatile float speedFinal = 0;
void setup() {
  // put your setup code here, to run once:
  // Initialize the Serial communication with external device (e.g. Raspberry Pi) at baud rate 115 200 (modify at the risk of increasing system latency)
  Serial1.begin(115200);
  Serial.begin(115200);
  while (!Serial1);
  while (!Serial);
  distanceFinal = 10;
  speedFinal = 20;
}

void loop() {
  // put your main code here, to run repeatedly:
  int len = snprintf(buffer, sizeof(buffer), "%0.2f,%0.2f\n", distanceFinal, speedFinal);
  Serial1.write(buffer, len);
  Serial.println("10,20");
  delay(1000);
}
