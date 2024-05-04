#!/bin/bash

# Create ReCo nodes

if [ -e "ros2_ws.zip" ]; then
        # If the workspace is in zip
        sudo apt install unzip -y
        echo "[+] Unzip installed..."

        # Unzip the workspace
        unzip -qq ros2_ws.zip
        echo "[+] Workspace unzipped successfully..."
        rm ros2_ws.zip
    else
        echo "[+] No workspace to unzip..."
    fi

# Source the directory permanently
echo 'source /home/ubuntu/ros2_ws/install/setup.bash' >> /home/ubuntu/.bashrc
# Apply changes
source /home/ubuntu/.bashrc

# Requirements
echo "[+] Installing requirements..."
sudo apt install nlohmann-json3-dev -y
sudo apt-get install rpi.gpio -y
echo "[+] Requirements installed successfully."
echo "[+] ROS workspace is ready!"

echo "RECO nodes created at: $(date)" >> /home/ubuntu/log/reco.log