import serial
import threading
import time

# Function to send data over UART0
def send_uart0():
    with serial.Serial('/dev/ttyAMA1', 115200) as ser:
        while True:
            #ser.write(b'<0,0,0,0>\n')
            #print("Sent 'test' to UART0")
            time.sleep(1)  # Send every 1 second

# Function to receive data over UART2
def receive_uart2():
    with serial.Serial('/dev/ttyAMA0', 115200) as ser:
        print("Recieve thread started")
        while True:
            if ser.in_waiting > 0:
                print("Data available on UART2")
                received_data = ser.read().decode('utf-8')
                print(f"Received from UART2: {received_data}")

# Create a thread for receiving data from UART2
receive_thread = threading.Thread(target=receive_uart2)
receive_thread.daemon = True  # Daemonize thread
receive_thread.start()

# Main execution (sending data over UART0)
send_uart0()
