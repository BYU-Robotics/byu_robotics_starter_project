# This launch file starts the joystick driver and the teleop translator node for controlling a robot using a joystick.
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    return LaunchDescription([
        # Declare a launch argument for the joystick device
        Node(
            package='joy',
            executable='joy_node',
            name='controller_driver',
        ),

        # Declare a launch argument for the teleop translator node
        Node(
            package='controller_cpp',
            executable='teleop',
            name='teleop_translator',
        )
    ])