# Boat Microcontroller Firmware
## Commands to run
YOU NEED MICROROS INSTALLED ON YOUR COMPUTER SEPERATELY
```bash
cd ./microros
source install/setup.bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 # change this to the port of your microcontroller if needed
```
## Roadmap
- [ ] Implement timer system to send messages at fixed rate instead of having a delay
- [ ] Implement watchdog that checks if heartbeat is recieved from main system
- [ ] Implement PID Controller to take values from /cmd_vel (which will likely be velocities or RPMs) and use hall sensor data to track true RPM and then adjust PWM accordingly
- [ ] Implement better error handling especially for the ROS2 functions
- [ ] Implement current checking to make sure motor current isn't spiking
- [ ] Implement kill switch

### Possible Changes
- [ ] Maybe Implement a service to change settings on the IMUs, such as filter bandwidth and accelerometer range
- [ ] Possibly implement motor specific commands instead of seperating the values from 1 topic
- [ ] Possibly implement a return message to go back to system, return heartbeat (possibly not neccesary as will always be reporting position)
