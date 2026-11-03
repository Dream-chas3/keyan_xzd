#!/bin/bash
set -e

PIDS=()

cleanup(){
        for pid in "${PIDS[@]}"; do
                if kill -0 "$pid" 2>/dev/null; then
                        kill "$pid"
                fi
        done
        wait
}

trap cleanup TERM INT HUP

source "/opt/ros/noetic/setup.bash"
TECH_WS="/home/orangepi/something1017/TECH"
if [[ -f "$TECH_WS/setup.bash" ]]; then
        source "$TECH_WS/setup.bash"
elif [[ -f "$TECH_WS/devel/setup.bash" ]]; then
        source "$TECH_WS/devel/setup.bash"
else
        echo "Cannot find TECH/setup.bash or TECH/devel/setup.bash" >&2
        exit 1
fi

# The camera was verified with the Orbbec package built in TECH_DEMO and its
# matching SDK. Keep that environment isolated to the camera process so the
# other sensors continue using TECH and ROS standard image/depth topics remain
# unchanged.
CAMERA_WS="/home/orangepi/something1017/TECH_DEMO"
CAMERA_SDK_DIR="$CAMERA_WS/src/OrbbecSDK_ROS1-2-main/SDK/lib/arm64"
if [[ ! -f "$CAMERA_WS/devel/setup.bash" ]]; then
        echo "Missing verified Orbbec workspace: $CAMERA_WS/devel/setup.bash" >&2
        exit 1
fi
if [[ ! -f "$CAMERA_SDK_DIR/libOrbbecSDK.so" ]]; then
        echo "Missing Orbbec SDK library: $CAMERA_SDK_DIR/libOrbbecSDK.so" >&2
        exit 1
fi

# source "~/.bashrc"
# Advertise ROS nodes over Wi-Fi so a desktop RViz can connect to their XMLRPC/TCPROS ports.
# ROS_IP can be explicitly set by the service environment; otherwise use the active wlan0 address.
ROS_WIFI_INTERFACE="${ROS_WIFI_INTERFACE:-wlan0}"
if [[ -z "${ROS_IP:-}" ]]; then
        ROS_IP="$(ip -4 -o addr show dev "$ROS_WIFI_INTERFACE" scope global 2>/dev/null | awk 'NR == 1 {split($4, a, "/"); print a[1]}')"
fi
if [[ -n "${ROS_IP:-}" ]]; then
        export ROS_IP
        unset ROS_HOSTNAME
else
        export ROS_HOSTNAME="$(hostname)"
        echo "WARNING: $ROS_WIFI_INTERFACE has no IPv4 address; remote ROS clients may not connect." >&2
fi
export ROS_MASTER_URI=http://localhost:11311/

sleep 3
roslaunch mavros px4.launch &
PIDS+=($!)


sleep 3
rm -f /dev/shm/orbbec_device.lock
(
        source "/opt/ros/noetic/setup.bash"
        source "$CAMERA_WS/devel/setup.bash"
        export LD_LIBRARY_PATH="$CAMERA_SDK_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
        exec roslaunch orbbec_camera gemini_330_series.launch \
                color_width:=1280 color_height:=720 \
                depth_width:=848 depth_height:=480 \
                enable_color:=true enable_depth:=true enable_point_cloud:=true \
                enable_left_ir:=false enable_right_ir:=false
) &
PIDS+=($!)

sleep 3
roslaunch usb_cam usb_cam.launch &
PIDS+=($!)


sleep 25
roslaunch livox_ros_driver2 msg_MID360s.launch &
PIDS+=($!)



for pid in "${PIDS[@]}"; do
        wait "$pid" || {
                exit 1
        }
done
