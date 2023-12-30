import setup_path 
import airsim
import cv2

drone = airsim.MultirotorClient()
drone.confirmConnection()
drone.enableApiControl(True)


#起飞
drone.takeoffAsync().join()

drone.moveToPositionAsync(-5, 5, -10, 5, vehicle_name="drone_1").join()
print("arrived!")
drone.moveToPositionAsync(5, -5, -20, 5, vehicle_name="drone_1").join()
print("arrived!")
drone.moveToPositionAsync(8, 15, -10, 5, vehicle_name="drone_1").join()