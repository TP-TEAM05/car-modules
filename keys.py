import serial
from sshkeyboard import listen_keyboard
import time

ser = serial.Serial ("/dev/serial0", 9600)    #Open port with baud rate

global last_key
global speed_val
global angle_val

last_key = ""
speed_val = 100 # 0 - 200, 100 - stop 0-100 reverse, 100-200 forward
angle_val = 100 # 0 - 200, 100 - straight, 0 - left, 200 - right
#<is_left,is_forward,throttle,angle> - in percentages
write_data = "<1,1,0,0>"

speed_step = 5
angle_step = 25
last_time = time.time()

def handle_forward():
    global last_key
    global speed_val
    global last_time
    # print(f"FWD - latency {time.time() - last_time}")
    # last_time = time.time()
    last_key = "FORWARD"
    speed_val += speed_step
    if speed_val > 200:
        speed_val = 200

def handle_backward():
    global last_key
    global speed_val
    global last_time
    # print(f"BACK - latency {time.time() - last_time}")
    # last_time = time.time()
    last_key = "BACKWARD"
    speed_val -= speed_step
    if speed_val < 0:
        speed_val = 0

def handle_left():
    global last_key
    global angle_val
    global last_time
    # print(f"L - latency {time.time() - last_time}")
    # last_time = time.time()
    last_key = "LEFT"
    angle_val -= angle_step
    if angle_val < 0:
        angle_val = 0

def handle_right():
    global last_key
    global angle_val
    global last_time
    # print(f"R - latency {time.time() - last_time}")
    # last_time = time.time()
    last_key = "RIGHT"
    angle_val += angle_step
    if angle_val > 200:
        angle_val = 200

def handle_brake():
    global last_key
    global speed_val
    last_key = "BRAKE"
    speed_val = 100

def handle_write():
    global write_data
    global speed_val
    global angle_val

    if angle_val > 100:
        is_left = 0
        angle = angle_val - 100
    else:
        is_left = 1
        angle = 100 - angle_val
    
    if speed_val > 100:
        is_forward = 1
        throttle = speed_val - 100
    else:
        is_forward = 0
        throttle = 100 - speed_val

    write_data = f"<{str(is_left)},{str(is_forward)},{str(throttle)},{str(angle)}>"
    ser.write(write_data.encode()) #Send data via serial


def print_state():
    print(f"[KEY]: {last_key} \n [SPEED]: {speed_val} \n [ANGLE]: {angle_val} \n[WRITE_DATA]: {write_data}")


def press(key):
    if key == "up" or key == "w":
        handle_forward()
    elif key == "down" or key == "s":
        handle_backward()
    elif key == "left" or key == "a":
        handle_left()
    elif key == "right" or key == "d" :
        handle_right()
    elif key == "space":
        handle_brake()
    
    handle_write()
    print_state()

listen_keyboard(on_press=press,delay_second_char=0,
    delay_other_chars=0 )