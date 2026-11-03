import cv2
import numpy as np

img = cv2.imread("test.png")  # 从bag里截图一张
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

corners_x, corners_y = 13, 9  # 或你实际的内角点
flags = cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_NORMALIZE_IMAGE
ret, corners = cv2.findChessboardCorners(gray, (corners_x, corners_y), flags)

print("found:", ret)
if ret:
    cv2.drawChessboardCorners(img, (corners_x, corners_y), corners, ret)
    cv2.imshow("corners", img)
    cv2.waitKey(0)
