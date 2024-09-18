

import socket
import serial
import time
import threading


ser = serial.Serial ("/dev/ttyAMA0", 115200)

# Define the IP address and port to listen on
HOST = '192.168.20.228'
PORT = 12345

# Create a socket for listening
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))
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
            data, addr = sock.recvfrom(13)
            value = data.decode()
            
            value = data.decode()
            print(f"Received value: {value}")
            ser.write(data) #Send data via serial

            
except KeyboardInterrupt:
    # Cleanup when the program is terminated
    sock.close()


