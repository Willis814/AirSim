import airsim
import time
# connect to the AirSim simulator
client = airsim.MultirotorClient()
client.confirmConnection()
# get control
client.enableApiControl(True)
# unlock
client.armDisarm(True)
# Async methods returns Future. Call join() to wait for task to complete.
client.takeoffAsync().join()
client.takeoffAsync().join()
time.sleep(10)
#client.landAsync().join()
# lock
client.armDisarm(False)
# release control
client.enableApiControl(False)