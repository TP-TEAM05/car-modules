from serial import Serial
import pynmeagps
stream = Serial('COM12', 115200, timeout=3)
nmr = pynmeagps.NMEAReader(stream)
while True:
    (raw_data, parsed_data) = nmr.read()
    parsed: pynmeagps.NMEAMessage = parsed_data
    if (parsed.msgID == "RMC"):
        print(parsed.lat, parsed.lon)
