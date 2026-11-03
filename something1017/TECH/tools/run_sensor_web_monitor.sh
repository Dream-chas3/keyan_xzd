#!/usr/bin/env bash
set -eo pipefail

tools_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
tech_dir="$(cd "$tools_dir/.." && pwd)"

source /opt/ros/noetic/setup.bash
if [[ -f "$tech_dir/setup.bash" ]]; then
  source "$tech_dir/setup.bash"
elif [[ -f "$tech_dir/devel/setup.bash" ]]; then
  source "$tech_dir/devel/setup.bash"
else
  echo "Cannot find TECH/setup.bash or TECH/devel/setup.bash" >&2
  exit 1
fi

export ROS_MASTER_URI="${ROS_MASTER_URI:-http://localhost:11311/}"
if [[ -z "${ROS_IP:-}" ]]; then
  ros_interface="${ROS_WIFI_INTERFACE:-wlan0}"
  ROS_IP="$(ip -4 -o addr show dev "$ros_interface" scope global 2>/dev/null | awk 'NR == 1 {split($4, a, "/"); print a[1]}')"
fi
if [[ -n "${ROS_IP:-}" ]]; then
  export ROS_IP
  unset ROS_HOSTNAME
else
  export ROS_HOSTNAME="${ROS_HOSTNAME:-$(hostname)}"
  echo "WARNING: ROS_IP unavailable; remote ROS subscribers may not connect." >&2
fi
exec python3 "$tools_dir/sensor_web_monitor.py"
