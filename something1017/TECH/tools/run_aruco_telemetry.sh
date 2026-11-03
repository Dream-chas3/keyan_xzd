#!/usr/bin/env bash
set -eo pipefail
source /opt/ros/noetic/setup.bash
export ROS_MASTER_URI=http://127.0.0.1:11311
export ROS_IP=127.0.0.1
unset ROS_HOSTNAME
exec /usr/bin/python3 "$(dirname -- "${BASH_SOURCE[0]}")/aruco_telemetry_bridge.py" "$@"
