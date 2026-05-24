from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():


    return LaunchDescription([
        Node(
            package='joy',
            executable='joy_node',
            name='controller_driver',
            output='screen'
        ),
        Node(
            package='controller_cpp',
            executable='teleop',
            name='teleop_translator',
            output='screen'
        )
    ])