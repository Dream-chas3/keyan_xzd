#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rospy
import cv2
import numpy as np
from sensor_msgs.msg import Image
from cv_bridge import CvBridge

class ReprojErrorCalculator:
    def __init__(self):
        self.bridge = CvBridge()

        # 你的标定结果
        self.K = np.array([[596.1666643552264, 0.0,               635.4095239289288],
                           [0.0,               596.6036618869597, 370.5063645187318],
                           [0.0,               0.0,               1.0]])
        self.D = np.array([0.032200304138862976,
                           -0.027728108819793484,
                           0.0007210288262456767,
                           0.0007695699661571941,
                           0.0])

        # 棋盘参数：与标定命令 --size 13x9 一致（13 列 × 9 行 内角点）
        self.corners_x = 13
        self.corners_y = 9
        self.square_size = 0.02  # m

        # 棋盘 3D 点
        self.objp = np.zeros((self.corners_x * self.corners_y, 3), np.float32)
        self.objp[:, :2] = np.mgrid[0:self.corners_x, 0:self.corners_y].T.reshape(-1, 2)
        self.objp *= self.square_size

        self.total_err = 0.0
        self.total_points = 0
        self.frame_count = 0

        self.sub_img = rospy.Subscriber("/usb_cam/image_raw",
                                        Image, self.image_cb,
                                        queue_size=1)

    def image_cb(self, msg):
        img = self.bridge.imgmsg_to_cv2(msg, desired_encoding="bgr8")
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        flags = cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_NORMALIZE_IMAGE
        ret, corners = cv2.findChessboardCorners(
            gray,
            (self.corners_x, self.corners_y),
            flags
        )

        if not ret:
            rospy.loginfo_throttle(2, "Chessboard not found in current frame.")
            return

        criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER,
                    30, 0.001)
        corners_sub = cv2.cornerSubPix(
            gray,
            corners,
            (11, 11),
            (-1, -1),
            criteria
        )

        ok, rvec, tvec = cv2.solvePnP(
            self.objp,
            corners_sub,
            self.K,
            self.D,
            flags=cv2.SOLVEPNP_ITERATIVE
        )
        if not ok:
            rospy.logwarn("solvePnP failed on this frame.")
            return

        proj, _ = cv2.projectPoints(self.objp, rvec, tvec, self.K, self.D)
        proj = proj.reshape(-1, 2)
        imgp = corners_sub.reshape(-1, 2)

        err = cv2.norm(imgp, proj, cv2.NORM_L2) / len(imgp)

        self.total_err += err * len(imgp)
        self.total_points += len(imgp)
        self.frame_count += 1

        rms = self.total_err / self.total_points

        rospy.loginfo("Frame %d: reproj err = %.4f px, global RMS = %.4f px",
                      self.frame_count, err, rms)

def main():
    rospy.init_node("reprojection_error_calculator")
    ReprojErrorCalculator()
    rospy.loginfo("Reprojection error calculator started.")
    rospy.spin()

if __name__ == '__main__':
    main()
#rosrun camera_calibration cameracalibrator.py --size 13x9 --square 0.02 image:=/usb_cam/image_raw 