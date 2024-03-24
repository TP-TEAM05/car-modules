import socket
import serial
import RPi.GPIO as GPIO
import time
import threading


ser = serial.Serial ("/dev/serial0", 115200)
#serRecieve = serial.Serial ("/dev/ttyAMA0", 115200)

# Define the IP address and port to listen on
HOST = '192.168.1.49'
PORT = 12345

# Create a socket for listening
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))
#sock.settimeout(0.001)
data = bytes("<0,0,0,0>", "utf-8")


# setup multithreading for function recieve
#recieveThread = threading.Thread(target=recieve)
try:  
#    recieveThread.daemon = True  # Daemonize thread
#    recieveThread.start()
    print("Ready!")
    while True:
            #dist = distance()
            #print ("Measured Distance = %.1f cm" % dist)
            #time.sleep(0.001)
            data, addr = sock.recvfrom(13)
            value = data.decode()
            print(f"Received value: {value}")
            ser.write(data) #Send data via serial
            #serRecieve.write(data)
            
            """
            try:
                data, addr = sock.recvfrom(13)
                value = data.decode()
                
            except socket.timeout:
                pass
            value = data.decode()
            ser.write(data) #Send data via serial
            print(f"Received value: {value}")
            """
            
except KeyboardInterrupt:
    # Cleanup when the program is terminated
    GPIO.cleanup()
    sock.close()
    ser.close()
    #serRecieve.close()
#    recieveThread.join()
    print("Done")

