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
  echo "Cannot find TECH ROS environment" >&2
  exit 1
fi
export ROS_MASTER_URI="${ROS_MASTER_URI:-http://localhost:11311/}"
# This subscriber is on the same computer as the ROS master; no Wi-Fi IP needed.
export ROS_IP=127.0.0.1
unset ROS_HOSTNAME
exec python3 "$tools_dir/flight_monitor.py" "$@"
