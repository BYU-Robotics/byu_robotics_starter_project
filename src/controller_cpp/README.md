# controller_cpp

This package handles manual teleoperation for the robot. It captures raw hardware inputs from a connected gamepad/controller and translates them into standard kinematic velocity commands.

## Node Interface: `ControllerNode`

The node acts as a bridge between the physical controller driver and the downstream motor mixing systems.

### Subscribed Topics
*   **`joy`** (`sensor_msgs/msg/Joy`)
    *   Listens to the raw joystick array stream broadcasted by the hardware driver layer.

### Published Topics
*   **`telop`** (`geometry_msgs/msg/Twist`)
    *   Outputs the calculated target velocities. 
    *   `linear.x`: Forward/Reverse speed (ranges from `-1.0` to `1.0`).
    *   `angular.z`: Turning/Yaw rate (ranges from `-1.0` to `1.0`).

## Hardware Control Mapping

The node parses the incoming controller arrays using the standard Linux gamepad indices:
*   **Left Analog Stick (Vertical / Axis 1):** Maps directly to `linear.x` (Forward/Reverse).
*   **Right Analog Stick (Horizontal / Axis 3):** Maps directly to `angular.z` (Left/Right Steering).

## Built-in Fault Tolerance

The node includes native **Array Boundary Protection** inside its callback loop. 
*   If a controller drops frames or is disconnected (causing the incoming `axes` vector size to drop below 4 elements), the node catches the fault, explicitly overrides both output velocities to `0.0`, and logs a `"Controller Error"` message to the terminal to prevent the vehicle from running away.

## How to Run and Test This Package

### 1. Build the Package
Navigate to the root of your workspace and compile:
```bash
cd ~/robotics/ros2_ws/
colcon build --packages-select controller_cpp
source install/setup.bash
```

### 2. Launch the Node Configuration
You can launch this node alongside its automatic background hardware driver using the provided launch script:
```bash
ros2 launch controller_cpp teleop_launch.py
```

### 3. Verify the Outputs
To verify that data is translating correctly over the network, open a separate terminal tab and listen to the command stream:
```bash
ros2 topic echo /telop
```