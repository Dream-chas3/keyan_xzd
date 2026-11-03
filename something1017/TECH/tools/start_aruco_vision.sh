#!/usr/bin/env bash
# Start only RTSP capture, ArUco detection and the image viewer.
set -eo pipefail

TOOLS_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM_DIR="$(cd -- "$TOOLS_DIR/../.." && pwd)"
source /opt/ros/noetic/setup.bash
source "$PLATFORM_DIR/TECH_DEMO/devel/setup.bash"
export ROS_MASTER_URI=http://localhost:11311/
if [[ -z "${ROS_IP:-}" ]]; then
    ROS_IP="$(ip -4 -o addr show dev wlan0 scope global | awk 'NR == 1 {split($4, a, "/"); print a[1]}')"
fi
if [[ -n "$ROS_IP" ]]; then
    export ROS_IP
    unset ROS_HOSTNAME
fi

LOG_DIR="$HOME/.ros/log/aruco_vision"
mkdir -p "$LOG_DIR"
PIDS=()
VIEWER_PID=""
cleanup() {
    trap - EXIT INT TERM
    for pid in "${PIDS[@]}"; do
        kill -INT "$pid" 2>/dev/null || true
    done
    if [[ -n "${VIEWER_PID:-}" ]]; then
        kill -INT "$VIEWER_PID" 2>/dev/null || true
    fi
    wait || true
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

has_publisher() {
    python3 - "$1" <<'PY'
import sys
import rosgraph
publishers, _, _ = rosgraph.Master('/aruco_vision_launcher').getSystemState()
sys.exit(0 if any(topic == sys.argv[1] and nodes for topic, nodes in publishers) else 1)
PY
}

wait_for_image() {
    python3 - "$1" <<'PY'
import sys
import rospy
from sensor_msgs.msg import Image
rospy.init_node('aruco_image_check', anonymous=True)
try:
    image = rospy.wait_for_message(sys.argv[1], Image, timeout=15)
    print('{}: {}x{} {}'.format(sys.argv[1], image.width, image.height, image.encoding))
except rospy.ROSException:
    sys.exit('No image on {}. Check ~/.ros/log/aruco_vision/'.format(sys.argv[1]))
PY
}

if ! has_publisher /rtsp_cam/image_raw; then
    echo 'Starting RTSP camera. Logs: ~/.ros/log/aruco_vision/rtsp.log'
    roslaunch rtsp_test rtsp_test.launch >"$LOG_DIR/rtsp.log" 2>&1 &
    PIDS+=("$!")
fi
wait_for_image /rtsp_cam/image_raw

if ! has_publisher /aruco_imgresult; then
    echo 'Starting ArUco detection. Logs: ~/.ros/log/aruco_vision/aruco.log'
    roslaunch aruco_test Safina_aruco.launch >"$LOG_DIR/aruco.log" 2>&1 &
    PIDS+=("$!")
fi
wait_for_image /aruco_imgresult

if [[ -n "${DISPLAY:-}" && "${1:-}" != "--no-view" ]]; then
    # rqt_image_view occasionally opens with a blank canvas on the software
    # renderer (the first instance); a later instance paints fine. The viewer
    # is deliberately NOT part of the wait set: closing it must never tear
    # down the RTSP/detector nodes. A short delay lets the detector publish
    # a few frames before the window is created.
    ( sleep 2; QT_XCB_GL_INTEGRATION=none LIBGL_ALWAYS_SOFTWARE=1 QT_OPENGL=software \
        exec rqt_image_view /aruco_imgresult ) >"$LOG_DIR/viewer.log" 2>&1 &
    VIEWER_PID=$!
    echo "Viewer starting. If its window is blank, close it and run: rqt_image_view /aruco_imgresult"
fi
echo 'Vision is ready. No arming, flight modes or flight controllers are started.'
if ((${#PIDS[@]})); then
    echo 'Keep this terminal open. Ctrl+C stops only processes started here.'
    wait -n "${PIDS[@]}"
elif [[ -n "${VIEWER_PID:-}" ]]; then
    wait "$VIEWER_PID"
else
    echo 'Existing RTSP and ArUco processes are being reused.'
fi
