import airsim
import math
import time
from PIL import Image
import matplotlib as plt
import numpy as np
import cv2


def takeoff_and_fly(client):
    # 启用API控制
    client.enableApiControl(True)

    # 解锁无人机
    client.armDisarm(True)

    # 起飞到指定高度
    client.takeoffAsync().join()

    # 等待无人机起飞
    time.sleep(5)

    # 飞行到目标位置
    client.moveToPositionAsync(20, 0, -5, 5).join()

    # 降落
    client.landAsync().join()

    # 解锁并禁用API控制
    client.armDisarm(False)
    client.enableApiControl(False)


def main():
    # 连接到AirSim模拟器
    client = airsim.MultirotorClient()
    client.confirmConnection()
    # get control
    client.enableApiControl(True)
    # unlock
    client.armDisarm(True)

    # 起飞并飞行
    takeoff_and_fly(client)

    # 停止模拟器
    client.simPause(True)
    client.stopAsync().join()


if __name__ == "__main__":
    main()
