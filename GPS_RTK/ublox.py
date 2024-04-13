from serial import Serial
import pyrtkgps.ublox as ublox
from pyubx2.ubxreader import UBXReader

# connect to the uart1 port with baudrate 38400
uart1 = Serial("COM12", 115200)

# split the uart1 stream into a stream for each protocol (UBX, RTCM, NMEA)
virtual_streams = ublox.StreamMuxDemux(uart1)

# load the chip configuration from a YAML file (or use a YAML string)
config_file = open("./base_station.yml", "r")

# serialize the configuration file
sr = ublox.UBXSerializer
serialized_config = sr.serialize(config_file)

# close the configuration file
config_file.close()

# write serialized config to the ublox chip 
virtual_streams.UBX.write(serialized_config)

# print UBX output
ubr = UBXReader(virtual_streams.UBX)
while True:
    raw, msg = ubr.read()
    print(msg)