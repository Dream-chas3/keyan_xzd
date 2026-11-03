#!/usr/bin/env python3
"""Read-only ArUco pose peek.

Prints, for the selected marker, the four values the operator asked for:
  forward / left / depth (metres) and the nose-to-edge angle (degrees).

It only subscribes to /aruco_poseresult and /aruco_target_id. It never
publishes setpoints, never changes flight mode and never arms anything.

Frame convention (same as Safina_land_test.cpp):
  camera optical: x right, y down, z forward (looking down)
  body:           x forward, y left, z up
  forward = -p.y + camera_offset_x
  left    = -p.x + camera_offset_y
  depth   =  p.z - camera_offset_z

Angle convention:
  The detector publishes the image angle of the marker edge corner0->corner1.
  A square marker is 4-fold symmetric, so "nose parallel to an edge" is
  satisfied when that angle is 0 or +/-90 degrees. The meaningful alignment
  error is therefore the published angle folded into [-45, +45] degrees;
  that is what 'nose_edge_err' prints. 'edge_raw' is the unmodified value.
"""
import argparse
import math
import sys

import rospy
from geometry_msgs.msg import PoseStamped
from std_msgs.msg import Int32


def yaw_of(q):
    return math.atan2(2.0 * (q.w * q.z + q.x * q.y),
                      1.0 - 2.0 * (q.y * q.y + q.z * q.z))


def fold90(deg):
    """Shortest signed rotation to the nearest multiple of 90 degrees."""
    return (deg + 45.0) % 90.0 - 45.0


def main():
    ap = argparse.ArgumentParser(description='Print marker position and nose-edge angle.')
    ap.add_argument('--camera-x', type=float, default=0.10,
                    help='camera forward offset from the body centre, m (default 0.10)')
    ap.add_argument('--camera-y', type=float, default=0.0,
                    help='camera left offset from the body centre, m')
    ap.add_argument('--camera-z', type=float, default=0.0,
                    help='camera up offset from the body centre, m')
    ap.add_argument('--hz', type=float, default=5.0, help='print rate, Hz')
    ap.add_argument('--id-topic', default='/aruco_target_id')
    ap.add_argument('--pose-topic', default='/aruco_poseresult')
    args = ap.parse_args()

    rospy.init_node('aruco_pose_peek', anonymous=True)
    latest = {'id': -1, 'pose': None, 'stamp': None}

    def on_id(msg):
        latest['id'] = msg.data

    def on_pose(msg):
        latest['pose'] = msg
        latest['stamp'] = rospy.get_time()

    rospy.Subscriber(args.id_topic, Int32, on_id)
    rospy.Subscriber(args.pose_topic, PoseStamped, on_pose)

    print('frame: forward/left/depth relative to the body centre; '
          'camera offset x=%.3f y=%.3f z=%.3f m' %
          (args.camera_x, args.camera_y, args.camera_z))
    print(' id   forward(m)   left(m)   depth(m)   edge_raw(deg)   nose_edge_err(deg)')
    last_quiet = 0.0
    while not rospy.is_shutdown():
        pose = latest['pose']
        fresh = pose is not None and rospy.get_time() - latest['stamp'] < 1.0
        if fresh:
            p = pose.pose.position
            if p.z > 0.0:
                forward = -p.y + args.camera_x
                left = -p.x + args.camera_y
                depth = p.z - args.camera_z
                raw = math.degrees(yaw_of(pose.pose.orientation))
                err = fold90(raw)
                print('%3d %+11.3f %+9.3f %10.3f %+15.1f %+20.1f' %
                      (latest['id'], forward, left, depth, raw, err))
                sys.stdout.flush()
            elif rospy.get_time() - last_quiet > 1.0:
                print('  -   no configured marker selected')
                sys.stdout.flush()
                last_quiet = rospy.get_time()
        elif rospy.get_time() - last_quiet > 1.0:
            print('  -   no marker message (is start_aruco_vision.sh running?)')
            sys.stdout.flush()
            last_quiet = rospy.get_time()
        rospy.sleep(1.0 / max(args.hz, 0.5))


if __name__ == '__main__':
    main()
