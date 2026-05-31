# Boat Microcontroller Firmware
## How to interact with MicroROS from ROS2
#### These are not the steps to download the firmware to the microcontroller, these are only the steps to get ROS2 to recognize the data being set by MicroROS
*YOU NEED MICROROS INSTALLED ON YOUR COMPUTER SEPERATELY*
```bash
# run this in every new terminal to configure it to recognize the microros commands
cd ./microros
source install/setup.bash
```
```bash
# run this from anywhere
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 # change this to the port of your microcontroller if needed
```
## Roadmap
### Future Additions
- [ ] Implement connection over wifi, bluetooth, etc not just the serial line
- [ ] Implement PID Controller to take values from /cmd_vel (which will likely be velocities or RPMs) and use hall sensor data to track true RPM and then adjust PWM accordingly
- [ ] Implement better error handling especially for the ROS2 functions
- [ ] Implement current checking to make sure motor current isn't spiking
- [ ] Implement kill switch

### Possible Changes
- [ ] Possibly Implement a service to change settings on the IMUs, such as filter bandwidth and accelerometer range
- [ ] Possibly implement motor specific commands instead of seperating the values from 1 topic
- [ ] Possibly implement a return message to go back to system, return heartbeat (possibly not neccesary as will always be reporting position)
