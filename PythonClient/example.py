import setup_path 
import airsim
import cv2

drone = airsim.MultirotorClient()
drone.confirmConnection()
drone.enableApiControl(True)

# car = airsim.CarClient()
# car.confirmConnection()
# car.enableApiControl(True)
print(drone.listVehicles())
#drone.takeoffAsync().join()
# drone.takeoffAsync().join()
drone.simDestroyVehicle("test3_drone")
print(drone.listVehicles())
# drone.moveToPositionAsync(-5, 5, -10, 5, vehicle_name="drone_1").join()
# print("arrived!")