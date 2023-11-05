import socket
import serial

ser = serial.Serial ("/dev/serial0", 9600)

# Define the IP address and port to listen on
HOST = '127.0.0.1'
PORT = 12345

# Create a socket for listening
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))

try:
    while True:
        data, addr = sock.recvfrom(1024)
        value = data.decode()
        ser.write(data) #Send data via serial
        print(f"Received value: {value}")
except KeyboardInterrupt:
    # Cleanup when the program is terminated
    sock.close()
