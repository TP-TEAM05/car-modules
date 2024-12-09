#!/bin/bash

# Requirements
echo "[+] Installing requirements..."
sudo apt install nlohmann-json3-dev -y
sudo apt-get install rpi.gpio -y
sudo apt install libcurl4-openssl-dev -y
sudo apt install libboost-all-dev -y
echo "[+] Requirements installed successfully."

#  Check if the -r or --remove flag is passed  and remove the workspace based on the flag
if [ "$1" == "-r" ] || [ "$1" == "--remove" ]; then
    echo "[+] Removing ROS2 workspace..."
    rm -rf ~/ros2_ws
    echo "[+] ROS2 workspace removed."
fi

# Prepare ROS2 workspace
git clone git@github.com:ReCoFIIT/ros2_ws.git ~/ros2_ws
cd ~/ros2_ws

colcon build

# Source the directory permanently
echo 'source /home/ubuntu/ros2_ws/install/setup.bash' >> /home/ubuntu/.bashrc
# Apply changes
source /home/ubuntu/.bashrc

colcon build

echo "[+] ROS workspace is ready!"
echo "RECO nodes created at: $(date)" >> /home/ubuntu/log/reco.log