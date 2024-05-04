#!/bin/bash

# Raspberry Pi Prepare Script for ReCo automotive

# Set locale
sudo apt update && sudo apt install locales -y
echo "[+] Locales installed..."
sudo locale-gen en_US en_US.UTF-8
echo "[+] Locale-gen done..."
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

echo "[+] Update-locale done..."

# Add ROS2 repository to system

# Ensure Ubuntu Universe repository is enabled

sudo apt install software-properties-common -y
echo "[+] Software-properties-common done..."
sudo add-apt-repository -y universe
echo "[+] Add-apt-repository universe done..."

# Add RO2 GPG key

sudo apt update && sudo apt install curl -y
echo "[+] Curl install done..."
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "[+] GPG-key add done..."

# Add repository to sources list
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
echo "[+] Repo added to sources list..."

sudo apt update
sudo apt upgrade -y
#enter treba
echo "[+] Update,upgrade done..."

sudo apt install ros-humble-ros-base -y
echo "[+] ROS base install done..."
sudo apt install ros-dev-tools -y
echo "[+] ROS dev-tools install done..."

source /opt/ros/humble/setup.bash
echo 'source /opt/ros/humble/setup.bash' >> /home/ubuntu/.bashrc
source /home/ubuntu/.bashrc

# dopisat do MD, ze aky je troubleshooting ak sa nenainstaluje ros korektne
mkdir /home/ubuntu/log
touch /home/ubuntu/log/reco.log