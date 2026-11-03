#!/usr/bin/env bash
set -e
source /opt/ros/noetic/setup.bash
exec python3 "$(dirname -- "${BASH_SOURCE[0]}")/track_telemetry_bridge.py" --port 8769 "$@"
