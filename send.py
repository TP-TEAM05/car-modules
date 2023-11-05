import time
import pygame
import socket


# Define the IP address and port to send data to localhost
HOST = '127.0.0.1'
PORT = 12345

# Initialize the game controller
pygame.init()
controller = pygame.joystick.Joystick(0)
controller.init()

# Initialize the socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
isLeft = 0
turnThrottle = 0
isForward = 0
throttle = 0


try:
    while True:
        for event in pygame.event.get():
            if event.type == pygame.JOYAXISMOTION:

                # axis 0 and 1 is the left stick and will control left and right
                # axis 2 and 3 is the right stick and will control forward and backward
        
                # right stick
                if event.axis == 3:
                    if event.value < 0:
                        isForward = 1
                        throttle = int((round(abs(event.value), 2) * 100) // 2)
                    elif event.value > 0:
                        isForward = 0
                        throttle = int((round(abs(event.value), 2) * 100) // 2)
                    else:
                        isForward = 0
                        throttle = 0

                # left stick
                if event.axis == 0:
                    if event.value < 0:
                        isLeft = 1
                        turnThrottle = int(round(abs(event.value), 2) * 100)
                    elif event.value > 0:
                        isLeft = 0
                        turnThrottle = int(round(abs(event.value), 2) * 100)
                    else:
                        isLeft = 0
                        turnThrottle = 0

                

                # send the data on socket in string format <isLeft, turnThrottle, isForward, throttle>
                data = f"<{isLeft},{isForward},{turnThrottle},{throttle}>"
                sock.sendto(data.encode(), (HOST, PORT))
                
                
        # when no events send default values
        #data = f"<{isLeft},{turnThrottle},{isForward},{throttle}>"
        #sock.sendto(data.encode(), (HOST, PORT))


except KeyboardInterrupt:
    # Cleanup when the program is terminated
    controller.quit()
    sock.close()
