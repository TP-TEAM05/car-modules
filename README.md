# ReCo Automotive

## RO2 setup

### Requirements:
- SD card with Ubuntu server 22.04 64-bit installed (using RPi Imager)
- enabled SSH & internet connection (configured in RPi Imager)

### ROS2 Humble installation

The installation of ROS2 Humble is done by `rpi-setup.sh` script. The script needs to be located in `/home/ubuntu/` directory. The script can be transferred to rpi using SCP or whatever file transfer you prefer. Below we show an example using SCP.

```
scp <path_to_rpi-setup.sh> ubuntu@<rpi_ip>:/home/ubuntu/
```

After you get the script on RPI do following.

```
chmod +x rpi-setup.sh
./rpi-setup.sh
```
Press ENTER in case the screen about service restart occurs.
After installation, make sure the ROS is installed.

```
source .bashrc
ros2
```

### ROS Workspace

**NOTE:** Copy scripts `reco-run.sh`, `reco-start.sh` and `reco-stop.sh` to `/home/ubuntu/` directory.

After you have the scripts copied, change their permissions.

```
chmod +x reco-start.sh
chmod +x reco-run.sh
chmod +x reco-stop.sh
```

#### Start
Copy ROS workspace `ros2_ws` to `/home/ubuntu/` directory. It can be either `.zip` or the whole directory. After run following:

```
./reco-start.sh
source /home/ubuntu/.bashrc
```

The workspace and the required packages are installed now.

#### Run

Now, proceed to following commands:

```
./reco-run.sh <IP_backend> <port_backend>
```
Both scripts require sudo permissions, so you will be requested for password.

After executing the script, the workspace archive is unzipped (if there is any) and all the ReCo processes are run in the background.

The script has optional argument `-v` which prints out the output of the nodes. By default it runs in silent mode.

#### Stop
To stop all the ReCo processes, execute the script `reco-stop.sh`

```
./reco-stop.sh
```

The script identifies all the processes related to ReCo and kills them.

### Troubleshooting

What may happen is, the ROS won't install properly due to some issues with network connection or some other similar issues. In that case, it is necessary to uninstall the ROS and proceed the installation manually.

Here are commands that may help.

```
sudo apt remove ~nros-humble-* && sudo apt autoremove

sudo apt install ros-humble-ros-base -y
sudo apt install ros-dev-tools -y
source /home/ubuntu/.bashrc
```