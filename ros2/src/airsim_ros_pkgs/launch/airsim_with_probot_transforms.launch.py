import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import launch_ros.actions

from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    airsim_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('airsim_ros_pkgs'), 'launch/airsim_node.launch.py')
        )
    )

    world_enu_to_map_pub = launch_ros.actions.Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='world_enu_to_map_pub',
        arguments=['0', '0', '0', '0', '0', '0', 'world_enu', 'map']
    )

    probot_odom_to_base_link_pub = launch_ros.actions.Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='probot_odom_to_base_link_pub',
        arguments=['0', '0', '0', '0', '0', '0', 'Probot/odom_local_ned', 'base_link']
    )

    # Create the launch description and populate
    ld = LaunchDescription()

    ld.add_action(airsim_node)
    ld.add_action(world_enu_to_map_pub)
    ld.add_action(probot_odom_to_base_link_pub)

    return ld
