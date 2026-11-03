#!/usr/bin/env bash
set -e
source /opt/ros/noetic/setup.bash
export ROS_MASTER_URI=http://localhost:11311/
export ROS_HOSTNAME=orangepi5ultra
exec /usr/bin/python3 /home/orangepi/something1017/TECH/tools/waypoint_telemetry_bridge.py \
  --host 0.0.0.0 --port 8770 \
  --mission-file /home/orangepi/something1017/TECH_DEMO/src/waypoint_mission/missions/abc_photo.yaml
