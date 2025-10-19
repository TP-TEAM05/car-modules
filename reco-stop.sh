#!/bin/bash

# ReCo script to kill all processes

# Kills all ROS processes 
kill -9 $(ps -aux | grep ros |awk '{print $2}' | head -6)

# Kills all rpi_controller processes
sudo kill -9 $(ps -aux | grep 'rpi_controller' | awk '{print $2}' | head -3)

echo "RECO nodes killed at: $(date)" >> /home/ubuntu/log/reco.log