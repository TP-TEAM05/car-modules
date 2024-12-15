#!/bin/bash

# Run ROS2 nodes necessary for ReCo automotive
# Usage: ./reco-run.sh <backend_ip> <backend_port> [-v]
# If -v is specified, the output of programs is printed to the console

# Get the current IP address and write it to config file
# ip_addr_device=$(ip addr show wlan0 | awk '$1 == "inet" {print $2}' | cut -d'/' -f1)

source /home/ubuntu/.bashrc

if [ -z "$1" ]; then
        echo "No IP provided."
        exit 1
    fi

echo $1 > /home/ubuntu/ros2_ws/src/car_to_backend/src/udp_client_config
echo "[+] Backend IP set successfully."

if [ -z "$2" ]; then
        echo "No port provided."
        exit 1
    fi

echo $2 >> /home/ubuntu/ros2_ws/src/car_to_backend/src/udp_client_config
echo "[+] Backend port set successfully."

if [ -z "$3" ]; then
        verbose=0
    else
        verbose=1
    fi

source /home/ubuntu/.bashrc

if [ "$verbose" -eq 1 ]; then
    # Verbose is on, output is printed out
        /opt/ros/humble/bin/ros2 run car_to_backend serial_pub &
        echo "[+] ROS2: serial_pub --> started"
        /opt/ros/humble/bin/ros2 run car_to_backend gps_pub &
        echo "[+] ROS2: gps_pub --> started"
        /opt/ros/humble/bin/ros2 run car_to_backend udp_sub &
        echo "[+] ROS2: udp_sub --> started"

        python3 /home/ubuntu/rpi_controller_pid.py &
    else
    # Verbose is off, output is not printed out
        /opt/ros/humble/bin/ros2 run car_to_backend serial_pub 2>/dev/null &
        echo "[+] ROS2: serial_pub --> started"
        /opt/ros/humble/bin/ros2 run car_to_backend gps_pub 1>/dev/null 2>/dev/null &
        echo "[+] ROS2: gps_pub --> started"
        /opt/ros/humble/bin/ros2 run car_to_backend udp_sub 2>/dev/null &
        echo "[+] ROS2: udp_sub --> started"

        python3 /home/ubuntu/rpi_controller_pid.py 1>/dev/null 2>/dev/null &
        echo "[+] rpi_controller_pid.py --> started"
    fi

echo "RECO nodes started at: $(date)" >> /home/ubuntu/log/reco.log