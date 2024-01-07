

import socket
import serial
import RPi.GPIO as GPIO
import time
import threading
#GPIO Mode (BOARD / BCM)
GPIO.setmode(GPIO.BOARD)

#set GPIO Pins
PIN_TRIGGER = 7
PIN_ECHO = 11
#set GPIO direction (IN / OUT)
GPIO.setup(PIN_TRIGGER, GPIO.OUT)
GPIO.setup(PIN_ECHO, GPIO.IN)
GPIO.output(PIN_TRIGGER, GPIO.LOW)



ser = serial.Serial ("/dev/serial0", 115200)

# Define the IP address and port to listen on
HOST = '192.168.20.239'
PORT = 12345

# Create a socket for listening
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))
sock.settimeout(0.001)
isForward = 0
data = bytes("<0,0,0,0>", "utf-8")
try:
    print("Ready!")
    while True:
            #dist = distance()
            #print ("Measured Distance = %.1f cm" % dist)
            #time.sleep(0.001)
            """
            data, addr = sock.recvfrom(1024)
            value = data.decode()
            ser.write(data) #Send data via serial
            print(f"Received value: {value}")
            """
            try:
                data, addr = sock.recvfrom(13)
                value = data.decode()
                isForward = int(value[3])
                
            except socket.timeout:
                pass
            value = data.decode()
            ser.write(data) #Send data via serial
            print(f"Received value: {value}")

            
except KeyboardInterrupt:
    # Cleanup when the program is terminated
    GPIO.cleanup()
    sock.close()


