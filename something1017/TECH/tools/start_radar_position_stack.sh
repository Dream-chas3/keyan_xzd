#!/usr/bin/env bash
set -o pipefail

tools_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
tech_dir="$(cd "$tools_dir/.." && pwd)"
log_dir="${HOME}/.ros/log/radar_position_stack"
fastlio_pid=""
bridge_pid=""

log() {
  printf '[radar-position] %s\n' "$*"
}

estimator_has_horizontal_position() {
  local status="$1"
  grep -qE 'pos_horiz_(rel|abs)_status_flag: True' <<<"$status" && \
    ! grep -q 'const_pos_mode_status_flag: True' <<<"$status"
}

report_flight_state() {
  local state="$1"
  local mode armed guided
  mode="$(sed -n 's/^mode: "\(.*\)"$/\1/p' <<<"$state" | head -1)"
  armed="$(sed -n 's/^armed: \(.*\)$/\1/p' <<<"$state" | head -1)"
  guided="$(sed -n 's/^guided: \(.*\)$/\1/p' <<<"$state" | head -1)"
  log "PX4 state: mode=${mode:-unknown}, armed=${armed:-unknown}, guided=${guided:-unknown}. This script does not change flight state."
}

stop_children() {
  trap - INT TERM HUP EXIT
  for pid in "$bridge_pid" "$fastlio_pid"; do
    if [[ -n "$pid" ]] && kill -0 "$pid" 2>/dev/null; then
      kill -INT "$pid" 2>/dev/null || true
    fi
  done
  for pid in "$bridge_pid" "$fastlio_pid"; do
    if [[ -n "$pid" ]]; then
      wait "$pid" 2>/dev/null || true
    fi
  done
}

fail() {
  log "ERROR: $*"
  exit 1
}

trap stop_children INT TERM HUP EXIT

source /opt/ros/noetic/setup.bash || fail "Cannot source ROS Noetic."
if [[ -f "$tech_dir/setup.bash" ]]; then
  source "$tech_dir/setup.bash" || fail "Cannot source $tech_dir/setup.bash."
elif [[ -f "$tech_dir/devel/setup.bash" ]]; then
  source "$tech_dir/devel/setup.bash" || fail "Cannot source $tech_dir/devel/setup.bash."
else
  fail "Cannot find TECH/setup.bash or TECH/devel/setup.bash."
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
  log "WARNING: ROS_IP unavailable; remote ROS subscribers may not connect."
fi
mkdir -p "$log_dir" || fail "Cannot create log directory $log_dir."

command -v timeout >/dev/null || fail "The timeout command is required."
systemctl is-active --quiet Safina_sensor.service || \
  fail "Safina_sensor.service is not active; check it manually, do not start a duplicate Livox driver."
rosnode list >/dev/null 2>&1 || fail "ROS master is unavailable at $ROS_MASTER_URI."

nodes="$(rosnode list 2>/dev/null)"
grep -Fxq /livox_lidar_publisher2 <<<"$nodes" || fail "Livox node /livox_lidar_publisher2 is missing."
grep -Fxq /mavros <<<"$nodes" || fail "MAVROS node /mavros is missing."

state="$(timeout 5 rostopic echo -n 1 /mavros/state 2>/dev/null)" || \
  fail "Cannot read /mavros/state."
grep -q 'connected: True' <<<"$state" || fail "MAVROS is not connected to PX4."
grep -q 'armed: False' <<<"$state" || fail "PX4 is armed. Disarm and secure the aircraft before starting this stack."
report_flight_state "$state"

timeout 4 rostopic echo -n 1 /livox/lidar >/dev/null 2>&1 || fail "No recent /livox/lidar message."
timeout 4 rostopic echo -n 1 /livox/imu >/dev/null 2>&1 || fail "No recent /livox/imu message."

if grep -Fxq /laserMapping <<<"$nodes"; then
  log "Reusing the existing /laserMapping node."
else
  log "Starting FAST-LIO; keep this terminal open."
  roslaunch fast_lio mapping_mid360.launch >"$log_dir/fast_lio.log" 2>&1 &
  fastlio_pid=$!
fi

odom_ready=0
for _ in $(seq 1 60); do
  if [[ -n "$fastlio_pid" ]]; then
    kill -0 "$fastlio_pid" 2>/dev/null || fail "FAST-LIO exited; see $log_dir/fast_lio.log."
  fi
  if timeout 3 rostopic echo -n 1 /Odometry >/dev/null 2>&1; then
    odom_ready=1
    break
  fi
  sleep 1
done
[[ "$odom_ready" == 1 ]] || fail "No /Odometry within 60 seconds; see $log_dir/fast_lio.log."

log "FAST-LIO is publishing /Odometry."
nodes="$(rosnode list 2>/dev/null)"
if grep -Fxq /vision2mavros <<<"$nodes"; then
  log "Reusing the existing /vision2mavros node."
else
  log "Starting the PX4 vision-pose bridge."
  roslaunch to_mavros vision2mavros.launch >"$log_dir/vision_bridge.log" 2>&1 &
  bridge_pid=$!
fi

vision_ready=0
for _ in $(seq 1 30); do
  if [[ -n "$fastlio_pid" ]]; then
    kill -0 "$fastlio_pid" 2>/dev/null || fail "FAST-LIO exited; see $log_dir/fast_lio.log."
  fi
  if [[ -n "$bridge_pid" ]]; then
    kill -0 "$bridge_pid" 2>/dev/null || fail "Vision bridge exited; see $log_dir/vision_bridge.log."
  fi
  if timeout 3 rostopic echo -n 1 /mavros/vision_pose/pose >/dev/null 2>&1; then
    vision_ready=1
    break
  fi
  sleep 1
done
[[ "$vision_ready" == 1 ]] || fail "No /mavros/vision_pose/pose within 30 seconds; see $log_dir/vision_bridge.log."

vision_info="$(timeout 5 rostopic info /mavros/vision_pose/pose 2>/dev/null)" || \
  fail "Cannot inspect /mavros/vision_pose/pose connections."
awk '/^Subscribers:/{found=1; next} found' <<<"$vision_info" | \
  grep -qE '(^|[[:space:]])/mavros([[:space:]]|$)' || \
  fail "MAVROS is not subscribed to /mavros/vision_pose/pose; the pose is not being forwarded by the ROS MAVROS plugin."

estimator_status="$(timeout 3 rostopic echo -n 1 /mavros/estimator_status 2>/dev/null || true)"
if estimator_has_horizontal_position "$estimator_status"; then
  log "PX4 reports a valid horizontal position estimate."
else
  log "WARNING: PX4 has no valid horizontal position estimate yet. The ROS sensor/vision chain will remain running; do not arm or take off."
  log "Inspect /mavros/estimator_status and PX4 EKF/external-vision parameters."
fi

log "ROS localization data chain is running. This script does not arm, change flight mode, or send flight setpoints."
log "Logs: $log_dir"
last_estimator_state="unknown"

while true; do
  nodes="$(rosnode list 2>/dev/null)" || fail "ROS master became unavailable; stopping the stack."
  grep -Fxq /laserMapping <<<"$nodes" || fail "FAST-LIO node disappeared; stopping the vision bridge."
  grep -Fxq /vision2mavros <<<"$nodes" || fail "Vision bridge disappeared; stopping FAST-LIO."
  vision_info="$(timeout 5 rostopic info /mavros/vision_pose/pose 2>/dev/null)" || \
    fail "Cannot inspect /mavros/vision_pose/pose connections."
  awk '/^Subscribers:/{found=1; next} found' <<<"$vision_info" | \
    grep -qE '(^|[[:space:]])/mavros([[:space:]]|$)' || \
    fail "MAVROS stopped subscribing to /mavros/vision_pose/pose; stopping the localization stack."
  estimator_status="$(timeout 3 rostopic echo -n 1 /mavros/estimator_status 2>/dev/null || true)"
  if estimator_has_horizontal_position "$estimator_status"; then
    if [[ "$last_estimator_state" != "ready" ]]; then
      log "PX4 horizontal position estimate is valid. This still is not a takeoff authorization."
      last_estimator_state="ready"
    fi
  else
    if [[ "$last_estimator_state" != "not-ready" ]]; then
      log "WARNING: PX4 horizontal position estimate is not valid; keeping ROS data nodes running. Do not arm or take off."
      last_estimator_state="not-ready"
    fi
  fi
  if [[ -n "$fastlio_pid" ]]; then
    kill -0 "$fastlio_pid" 2>/dev/null || fail "FAST-LIO launch exited; stopping the vision bridge."
  fi
  if [[ -n "$bridge_pid" ]]; then
    kill -0 "$bridge_pid" 2>/dev/null || fail "Vision bridge launch exited; stopping FAST-LIO."
  fi
  timeout 3 rostopic echo -n 1 /Odometry >/dev/null 2>&1 || \
    fail "/Odometry stopped; stopping the vision bridge and FAST-LIO."
  sleep 1
done
