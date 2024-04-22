import sys, os
import serial
import struct
import logging
from datetime import datetime


tty = "COM3"

ser = serial.Serial(tty, 115200, timeout=2, xonxoff=False, rtscts=False, dsrdtr=False)



ser.flushInput()
ser.flushOutput()
print("Hello UBX, let's get the data!")

#147.175.56.137
rawMessage = bytearray()
mlcounter = 0

globalGroundSpeed = 0
globalCourse = 0

print_hpposllh = 1
print_velned = 1

now = datetime.now()
nowstr = now.strftime("%Y-%m-%d_%H-%M-%S")
logname = "./" + nowstr + "_gps.log"
logging.basicConfig(filename=logname, filemode='w', format='%(asctime)s - %(levelname)s - %(message)s', level=logging.INFO)

def parse_msg():
    if len(rawMessage) == 42:
        if rawMessage[3] == 0x12:
            parse_velned()
        if rawMessage[3] == 0x14:
            parse_hpposllh()



def parse_velned():
    if len(rawMessage) == 42:
        if rawMessage[3] == 0x12:
            bytes_gSpeed = rawMessage[26:30]
            gSpeed = struct.unpack('<l', bytes_gSpeed)

            bytes_gSAcc = rawMessage[34:38]
            gSAcc = struct.unpack('<l', bytes_gSAcc)

            bytes_course = rawMessage[30:34]
            course = struct.unpack('<l', bytes_course)

            bytes_gCAcc = rawMessage[38:42]
            gCAcc = struct.unpack('<l', bytes_gCAcc)

            now = datetime.now()
            current_time = now.strftime("%Y/%m/%d %H:%M:%S")

            global globalGroundSpeed
            global globalCourse

            if gSpeed[0] < 50000:
                globalGroundSpeed = gSpeed[0]
                globalCourse = course[0]


            if print_velned == 1:
                print("# # # # # # # # # # # # # # #")
                print("NAV_VELNED found")
                print("Date Time:", current_time)
                print("Ground speed:", gSpeed[0] * 3.6 / 10)
                print("Ground speed accuracy:", gSAcc[0])
                print("Course:", course[0] / 100000)
                print("Course accuracy:", gCAcc[0] / 10000)


def parse_hpposllh():
    if len(rawMessage) == 42:
        if rawMessage[3] == 0x14:
            bytes_lon = rawMessage[14:18]
            bytes_lat = rawMessage[18:22]
            bytes_hellip = rawMessage[22:26]
            bytes_hmsl = rawMessage[26:30]

            bytes_lonHp = rawMessage[30]
            if bytes_lonHp > 127:
                bytes_lonHp -= 256
            
            bytes_latHp = rawMessage[31]
            if bytes_latHp > 127:
                bytes_latHp -= 256

            bytes_hHp = rawMessage[32]
            if bytes_hHp > 127:
                bytes_hHp -= 256
            
            bytes_hMSLHp = rawMessage[33]
            if bytes_hMSLHp > 127:
                bytes_hMSLHp -= 256

            now = datetime.now()
            current_time = now.strftime("%Y/%m/%d %H:%M:%S")

            bytes_hAcc = rawMessage[34:38]
            hAcc = struct.unpack('<l', bytes_hAcc)

            bytes_vAcc = rawMessage[38:42]
            vAcc = struct.unpack('<l', bytes_vAcc)

            lat = struct.unpack('<l', bytes_lat)
            lon = struct.unpack('<l', bytes_lon)
            hellip = struct.unpack('<l', bytes_hellip)
            hmsl = struct.unpack('<l', bytes_hmsl)

            logging.info(";LAT;" + str(lat[0] / 10000000) + ";LON;" + str(lon[0] / 10000000) + ";HMSL(mm);" + str(hmsl[0]) + ";GSPEED(km/h);" + str((globalGroundSpeed / 100) * 3.6) + ";CRS;" + str(globalCourse / 100000) + ";HACC(mm);" + str(hAcc[0]/10))
            print("som tu")
            if print_hpposllh == 1:

                print("# # # # # # # # # # # # # # #")
                print("NAV_HPPOSLLH found")
                print("Date Time:", current_time)
                print("Lat:", lat[0] / 10000000, "Lon:", lon[0] / 10000000, "LatHP:", bytes_latHp, "LonHP:", bytes_lonHp)
                print("Height (ellipsoid):", hellip[0], "mm HP:", bytes_hHp / 10, "mm")
                print("Height (MSL):", hmsl[0], "mm HP:", bytes_hMSLHp / 10, "mm")
                print("hAcc:", hAcc[0] / 10, "mm vAcc:", vAcc[0] / 10, "mm")


if __name__ == "__main__": 

    try:      
    #infinite loop that reads serial interface char by char
        while True:
            bytesToRead = ser.inWaiting()
            lastByte = ser.read(1)

            if lastByte[0] == 0xb5:
                mlcounter = 0
                parse_msg()
                rawMessage.clear()
                rawMessage.append(lastByte[0])
                mlcounter += 1
            else:
                if mlcounter < 42:
                    rawMessage.append(lastByte[0])
                    mlcounter += 1
    except KeyboardInterrupt : 
        print("Interupted by User")
        try: 
            sys.exit(0)
        except: 
            os._exit(0)
