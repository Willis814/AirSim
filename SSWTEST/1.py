import airsim
import time
# Connect to AirSim simulator
client = airsim.MultirotorClient()
client.confirmConnection()

client.enableApiControl(True)
# unlock
client.armDisarm(True)
# Async methods returns Future. Call join() to wait for task to complete.
client.takeoffAsync().join()
time.sleep(2)
# 移动飞行器到新的位置
client.moveToPositionAsync(10, 20, -4, 2).join()
client.moveToPositionAsync(-10, 50, -6, 2).join()
"""# Verify the new camera position
new_camera_info = client.simGetCameraInfo("front_center")
print("New Camera Pose: X={}, Y={}, Z={}".format(
    new_camera_info.pose.position.x_val,
    new_camera_info.pose.position.y_val,
    new_camera_info.pose.position.z_val
))
"""