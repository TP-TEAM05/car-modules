import socket
import base64
import sys
import time
from serial import Serial
import pynmeagps
import threading

# Initialization for serial connection
tty = "COM12"
ser = Serial(tty, 115200, timeout=3, xonxoff=False, rtscts=False, dsrdtr=False)
ser.flush()

# NTRIP settings
bs = "rgeerg"
server = "ergerg"
port = "ergreg"
mountpoint = "ergerg"
username = "ergerg"
password = "fgweraerger"

def getHTTPBasicAuthString(username, password):
    inputstring = username + ':' + password
    pwd_bytes = base64.encodebytes(inputstring.encode("utf-8"))
    pwd = pwd_bytes.decode("utf-8").replace('\n','')
    return pwd

pwd = getHTTPBasicAuthString(username, password)

header = f"GET /{mountpoint} HTTP/1.0\r\n" +\
         "User-Agent: NTRIP u-blox\r\n" +\
         "Accept: */*\r\n" +\
         f"Authorization: Basic {pwd}\r\n" +\
         "Connection: close\r\n\r\n"

def ntrip_client():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        global ntrip_socket
        ntrip_socket = s
        ntrip_socket.connect((server, int(port)))
        ntrip_socket.send(header.encode('utf-8'))
        resp = ntrip_socket.recv(1024)
        if resp.startswith(b"STREAMTABLE"):
            print("Invalid or No Mountpoint")
            exit()
        elif resp.startswith(b"ICY 200 OK"):
            print("All good")
        
        
            while True:
                try:
                    data = ntrip_socket.recv(1024)
                    if not data:
                        break
                    ser.write(data)
                    print("Written RTK")
                except Exception as e:
                    print(f"Error in NTRIP client: {e}")
                    # reestablish connection
                    while True:
                        try:
                            ntrip_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                            ntrip_socket.connect((server, int(port)))
                            ntrip_socket.send(header.encode('utf-8'))
                            resp = ntrip_socket.recv(1024)
                            if resp.startswith(b"ICY 200 OK"):
                                print("Reconnected")
                                break
                        except Exception as e:
                            print(f"Error in NTRIP client: {e}")
                            time.sleep(5)

                    

                

def nmea_reader():
    nmr = pynmeagps.NMEAReader(ser)
    while True:
        try:
            raw_data, parsed_data = nmr.read()
            parsed: pynmeagps.NMEAMessage = parsed_data
            if (parsed.msgID == "RMC"):
                print(parsed.lat, parsed.lon)
        except Exception as e:
            print(f"Error in NMEA reader: {e}")


try:
    # Threads setup
    thread_ntrip = threading.Thread(target=ntrip_client)
    thread_nmea = threading.Thread(target=nmea_reader)

    # Start threads
    thread_ntrip.start()
    thread_nmea.start()

except KeyboardInterrupt:
    print("Exiting...")
    ser.close()
    # Join threads to the main thread
    thread_ntrip.join()
    thread_nmea.join()
