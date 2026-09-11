# launcher

This package acts as the top-level orchestration layer (master bringup) for the entire robot workspace. It aggregates independent launch files from separate packages so the entire vehicle can be started using a single terminal command.

## How to Launch Everything

To build the workspace and launch all included subsystems simultaneously, run the following commands from the root of your ROS 2 workspace:

```bash
cd ~/robotics/ros2_ws
colcon build
source install/setup.bash
ros2 launch launcher launch.py
```

## How to Add Your Own Package Launch File

To keep development isolated, do not add individual nodes directly to this master package. Instead, create a launch file inside your own package, and then include it here using IncludeLaunchDescription.
# Step 1: Update launcher/launch/launch.py

Open the master launch script and append your package's launch directory and file to the execution list:
```python
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    
    # Installation share paths for each existing packages
    controller_dir = PathJoinSubstitution([FindPackageShare('controller_cpp'), 'launch'])

    # 1. ADD YOUR SHARE PATH FOR YOUR PACKAGE HERE
    YOUR_PACKAGE_dir = PathJoinSubstitution([FindPackageShare('YOUR_PACKAGE_NAME'), 'launch'])

    return LaunchDescription([
        # Existing subsystems
        IncludeLaunchDescription(
            PathJoinSubstitution([controller_dir, 'teleop_launch.py'])
        ),
        
        # 2. ADD YOUR LAUNCH FILE HERE
        IncludeLaunchDescription(
            PathJoinSubstitution([YOUR_PACKAGE_dir, 'YOUR_LAUNCH_FILE.py'])
        ),
    ])
```

# Step 2: Ensure Your CMakeLists.txt or setup.py is Configured

For the master launcher to discover your files, your personal package must be actively installing its launch/ directory during the build process:

If your package is C++ (CMakeLists.txt), add this right above ament_package():

```CMake
  install(DIRECTORY launch
    DESTINATION share/${PROJECT_NAME}
  )
```
If your package is Python (setup.py), ensure your data_files includes:

```Python
  (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*.py'))),
```
Once verified, anyone on the team can run colcon build and bring up the updated network instantly!