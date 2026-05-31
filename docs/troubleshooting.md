# Common Problems, their Possible Causes, and Solutions

## 3D Model Not Moving in RViz
**Problem:** 3D model in RViz is doesn't rotate despite rotating sensors and boat. It stays completely stationary even when topics are being published to with sensor data   
**Cause:** Generally our IMU is publishing with a reliability policy of 'Best Effort' which doesn't guarentee their arrival but is quicker which we want for the IMU data. RViz is generally configured by default to look for the policy of 'Reliable' and ignores any data that doesn't meet that standard.  
**Solution:** In RViz, under the IMU display, click the dropdown on topic, click on reliability policy and switch it to match what the IMU is outputting (this should be best effort)

## Inconsistancies in Movement in RViz
**Problem:** 3D model is under rotating consistantly, this may manifest when you rotate the boat or sensor around 360 degrees in the real world, but the model only turns 3/4 of this distance.  
**Cause:** This problem comes because the 50hz rate that we calculate the IMU data at means that a lot of data is being sent over the serial port. If our baud rate is too low, not all of the data gets sent through in time.   
**Solution:** Increase our Baud rate on both the firmware (first in the initialization of the serial port and in the platform.io config file) and tell ROS2 this when we run the micro_ros_agent by adding add -b <BAUD_RATE> to the end of the command
```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 921600 
```
---
**Problem:** 3d model is moving inconsistantly, moving it back and forth does not bring it back to original position  
**Cause:** Every IMU when manufactured has slight defects and problems, this leads to them not being 100% accurate and generally reporting a value slightly off of the true number. While it is only a very slight difference, the position detection integrates these values which greatly increase their effect in the final output. This bias must be accounted for.  
**Solution:** Run the gyro bias calculation program (configured as a seperate environment inside the boat_brain_firmware folder) and hardcode the values for those specific IMUs in the firmware 
