import time
import pygame
import socket
import json


# Define the IP address and port to send data to localhost
HOST = '192.168.20.222'

PORT = 12345

# Initialize the game controller
pygame.init()
controller = pygame.joystick.Joystick(0)
controller.init()

# Initialize the socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
isLeft = 0
turnThrottle = 0
speed = 0
step = 1

newIter= False

try:
    while True:
        newIter = True
        for event in pygame.event.get():    
            #print(event)
            if event.type == pygame.JOYBUTTONDOWN:
                if event.button == 5:
                    speed += step
                elif event.button == 4:
                    if speed > 0:
                        speed -= step
                elif event.button == 2:
                    speed = 0
                    isLeft = 0
                    turnThrottle = 95
            if event.type == pygame.JOYAXISMOTION:
                # left stick
                if event.axis == 0:
                    if event.value < 0:
                        isLeft = 1
                        # turnThrottle from 91-180, remap from -1 to 0 to 91 to 180
                        turnThrottle = int(95 + (90 * event.value))

                    elif event.value > 0:
                        isLeft = 0
                        turnThrottle = int(95 + (90 * event.value))
                    else:
                        isLeft = 0
                        turnThrottle = 95

                
            if newIter and (event.type == pygame.JOYBUTTONDOWN or event.type == pygame.JOYAXISMOTION):
                # send the data on socket in string format <isLeft,isForward, turnThrottle, isForward, throttle>
        
                data = f"<{speed},{turnThrottle}>\r"
                json_data = json.dumps({"updateVehicleDecision": {"message": data}})


                sock.sendto(json_data.encode(), (HOST, PORT))
                print(data)
                newIter = False
                    #time.sleep(0.1)

except KeyboardInterrupt:
    # Cleanup when the program is terminated
    controller.quit()
    sock.close()
