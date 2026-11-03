#!/usr/bin/env python3
"""航点规划 · 香橙派只读遥测桥接

ROS -> HTTP JSON。只订阅话题，不发布任何消息、不调用任何服务、不控制飞机。
PC 端 waypoint_station.py 通过 HTTP GET /api/state 轮询。

订阅：
  /mavros/state
  /mavros/local_position/pose
  /mavros/local_position/velocity_local
  /mavros/rc/in
  /mavros/battery
  /mavros/extended_state
  /mavros/setpoint_raw/local
  /mavros/setpoint_raw/target_local
  /mavros/vision_pose/pose
  /Odometry
  /waypoint_mission/status
  /waypoint_mission/photo_done

用法：
  source /opt/ros/noetic/setup.bash
  python3 waypoint_telemetry_bridge.py --port 8770 \
      --mission-file ~/something1017/TECH_DEMO/src/waypoint_mission/missions/abc_photo.yaml
"""
import argparse
import collections
import json
import math
import os
import resource
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

VERSION = "waypoint-bridge-1.0"
DEFAULT_MISSION = os.path.expanduser(
    "~/something1017/TECH_DEMO/src/waypoint_mission/missions/abc_photo.yaml")


def clean(obj):
    if isinstance(obj, float) and not math.isfinite(obj):
        return None
    if isinstance(obj, dict):
        return {k: clean(v) for k, v in obj.items()}
    if isinstance(obj, (list, tuple)):
        return [clean(v) for v in obj]
    return obj


def xyz(v):
    return [v.x, v.y, v.z]


def quaternion(q):
    return [q.x, q.y, q.z, q.w]


def yaw_of_quat(q):
    x, y, z, w = q
    n = sum(t * t for t in q)
    if n <= 1e-9:
        return 0.0
    return math.atan2(2.0 * (w * z + x * y), n - 2.0 * (y * y + z * z))


def pose_data(msg):
    p = msg.pose
    q = quaternion(p.orientation)
    return {
        "position": xyz(p.position),
        "quaternion": q,
        "yaw": yaw_of_quat(q),
        "frame": msg.header.frame_id,
        "stamp": msg.header.stamp.to_sec(),
    }


def setpoint_data(msg):
    return {
        "frame": int(msg.coordinate_frame),
        "type_mask": int(msg.type_mask),
        "position": xyz(msg.position),
        "velocity": xyz(msg.velocity),
        "acceleration_or_force": xyz(msg.acceleration_or_force),
        "yaw": float(msg.yaw),
        "yaw_rate": float(msg.yaw_rate),
        "header_frame": msg.header.frame_id,
        "header_stamp": msg.header.stamp.to_sec(),
    }


def parse_mission_status(raw):
    d = {}
    for part in raw.split():
        if "=" in part:
            k, v = part.split("=", 1)
            d[k] = v
    for k in ("state_age", "pose_age", "odom_age", "dist", "yaw", "yaw_err"):
        if k in d:
            try:
                d[k] = float(d[k])
            except ValueError:
                pass
    for k in ("rc_slot", "rc_pwm", "rssi"):
        if k in d:
            try:
                d[k] = int(d[k])
            except ValueError:
                pass
    for k in ("armed", "connected", "gate_ready", "rc_fresh", "pose_fresh", "odom_fresh"):
        if k in d:
            d[k] = d[k] == "1"
    if "target" in d:
        try:
            d["target"] = [float(x) for x in d["target"].split(",")]
        except ValueError:
            pass
    if "wp" in d:
        try:
            i, n = d["wp"].split("/")
            d["wp_index"] = int(i)
            d["wp_count"] = int(n)
        except ValueError:
            pass
    return d


class MissionFile:
    """Reload the mission YAML when its mtime changes. Read-only."""

    def __init__(self, path):
        self.path = path
        self.mtime = 0.0
        self.data = {"path": path, "waypoints": [], "error": "not loaded"}

    def snapshot(self):
        try:
            mtime = os.path.getmtime(self.path)
            if mtime != self.mtime:
                self._load()
                self.mtime = mtime
        except Exception as e:  # noqa: BLE001 - serve the error to the station
            self.data = {"path": self.path, "waypoints": [], "error": str(e)}
        return self.data

    def _load(self):
        import yaml  # PyYAML is present on the aircraft image
        with open(self.path, "r", encoding="utf-8") as f:
            root = yaml.safe_load(f) or {}
        m = root.get("mission", root) if isinstance(root, dict) else {}
        waypoints = []
        for i, w in enumerate(m.get("waypoints", []) or []):
            if not isinstance(w, dict):
                continue
            waypoints.append({
                "index": i,
                "name": str(w.get("name", "wp%d" % i)),
                "x": float(w.get("x", 0.0)),
                "y": float(w.get("y", 0.0)),
                "z": float(w.get("z", 0.0)),
                "yaw": w.get("yaw", "auto"),
                "hold": float(w.get("hold", 0.0)),
                "photo": bool(w.get("photo", False)),
                "gimbal_pitch": w.get("gimbal_pitch"),
                "gimbal_yaw": w.get("gimbal_yaw"),
                "speed": w.get("speed"),
                "tolerance": w.get("tolerance"),
                "timeout": w.get("timeout"),
            })
        self.data = {
            "path": self.path,
            "name": str(m.get("name", "mission")),
            "frame": str(m.get("frame", "local")),
            "takeoff_z": m.get("takeoff_z"),
            "return_to_start": bool(m.get("return_to_start", True)),
            "auto_land": bool(m.get("auto_land", True)),
            "max_speed": m.get("max_speed"),
            "max_z_speed": m.get("max_z_speed"),
            "geofence_radius": m.get("geofence_radius"),
            "waypoints": waypoints,
            "error": None,
        }


class Cache:
    def __init__(self):
        self.lock = threading.Lock()
        self.latest = {}
        self.rates = {}
        self.sequence = 0
        self.started = time.monotonic()

    def put(self, key, data, now=None):
        now = time.monotonic() if now is None else now
        with self.lock:
            times = self.rates.setdefault(key, collections.deque(maxlen=500))
            times.append(now)
            self.latest[key] = (now, clean(data))
            self.sequence += 1

    def snapshot(self):
        now = time.monotonic()
        with self.lock:
            topics = {}
            for key, (at, value) in self.latest.items():
                samples = [t for t in self.rates[key] if now - 3.0 <= t <= now]
                hz = ((len(samples) - 1) / (samples[-1] - samples[0])
                      if len(samples) > 1 else 0.0)
                topics[key] = {
                    "age": max(0.0, now - at),
                    "hz": round(hz, 2),
                    "data": value,
                }
            usage = resource.getrusage(resource.RUSAGE_SELF)
            elapsed = max(1.0, now - self.started)
            return {
                "version": VERSION,
                "server_time": time.time(),
                "sequence": self.sequence,
                "read_only": True,
                "topics": topics,
                "bridge": {
                    "uptime_s": round(elapsed, 1),
                    "cpu_average_percent_one_core": round(
                        100.0 * (usage.ru_utime + usage.ru_stime) / elapsed, 2),
                    "rss_mib": round(usage.ru_maxrss / 1024.0, 2),
                    "images_subscribed": False,
                    "publishers": 0,
                    "services": 0,
                },
            }


def attach_ros(cache):
    import rospy
    from geometry_msgs.msg import PoseStamped, TwistStamped
    from mavros_msgs.msg import ExtendedState, PositionTarget, RCIn, State
    from nav_msgs.msg import Odometry
    from sensor_msgs.msg import BatteryState
    from std_msgs.msg import String

    rospy.init_node("waypoint_telemetry_bridge", disable_signals=True)

    def state_cb(msg):
        cache.put("state", {
            "mode": msg.mode,
            "armed": bool(msg.armed),
            "connected": bool(msg.connected),
            "guided": bool(msg.guided),
            "manual_input": bool(msg.manual_input),
            "system_status": int(msg.system_status),
        })

    def pose_cb(msg):
        cache.put("pose", pose_data(msg))

    def vision_cb(msg):
        cache.put("vision_pose", pose_data(msg))

    def velocity_cb(msg):
        cache.put("velocity", {"linear": xyz(msg.twist.linear),
                               "angular": xyz(msg.twist.angular)})

    def rc_cb(msg):
        channels = list(msg.channels)
        cache.put("rc", {
            "rssi": int(msg.rssi),
            "channels": channels,
            "channel5": channels[4] if len(channels) > 4 else None,
        })

    def battery_cb(msg):
        cache.put("battery", {
            "voltage": float(msg.voltage),
            "percentage": float(msg.percentage),
            "current": float(msg.current),
            "power_supply_status": int(msg.power_supply_status),
        })

    def extended_cb(msg):
        cache.put("extended_state", {"landed_state": int(msg.landed_state)})

    def setpoint_cb(msg):
        cache.put("setpoint", setpoint_data(msg))

    def target_cb(msg):
        cache.put("target_local", setpoint_data(msg))

    def odom_cb(msg):
        q = quaternion(msg.pose.pose.orientation)
        cache.put("odometry", {
            "position": xyz(msg.pose.pose.position),
            "yaw": yaw_of_quat(q),
            "frame": msg.header.frame_id,
            "stamp": msg.header.stamp.to_sec(),
        })

    def status_cb(msg):
        raw = msg.data
        cache.put("mission_status", {"raw": raw, "parsed": parse_mission_status(raw)})

    def photo_cb(msg):
        cache.put("photo", {"result": msg.data})

    rospy.Subscriber("/mavros/state", State, state_cb, queue_size=10)
    rospy.Subscriber("/mavros/local_position/pose", PoseStamped, pose_cb, queue_size=10)
    rospy.Subscriber("/mavros/vision_pose/pose", PoseStamped, vision_cb, queue_size=10)
    rospy.Subscriber("/mavros/local_position/velocity_local", TwistStamped,
                     velocity_cb, queue_size=10)
    rospy.Subscriber("/mavros/rc/in", RCIn, rc_cb, queue_size=10)
    rospy.Subscriber("/mavros/battery", BatteryState, battery_cb, queue_size=10)
    rospy.Subscriber("/mavros/extended_state", ExtendedState, extended_cb, queue_size=10)
    rospy.Subscriber("/mavros/setpoint_raw/local", PositionTarget,
                     setpoint_cb, queue_size=10)
    rospy.Subscriber("/mavros/setpoint_raw/target_local", PositionTarget,
                     target_cb, queue_size=10)
    rospy.Subscriber("/Odometry", Odometry, odom_cb, queue_size=10)
    rospy.Subscriber("/waypoint_mission/status", String, status_cb, queue_size=10)
    rospy.Subscriber("/waypoint_mission/photo_done", String, photo_cb, queue_size=10)

    rospy.loginfo("waypoint telemetry bridge subscribed; read-only")
    rospy.spin()


def handler(cache, mission):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def _json(self, obj):
            body = json.dumps(clean(obj), ensure_ascii=False, allow_nan=False).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(body)

        def do_GET(self):
            if self.path.split("?")[0] in ("/api/state", "/state", "/"):
                snapshot = cache.snapshot()
                snapshot["mission"] = mission.snapshot()
                self._json(snapshot)
            elif self.path.split("?")[0] == "/health":
                body = b"ok"
                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            else:
                self.send_error(404)
    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8770)
    parser.add_argument("--mission-file", default=DEFAULT_MISSION)
    args = parser.parse_args()

    cache = Cache()
    mission = MissionFile(os.path.expanduser(args.mission_file))
    server = ThreadingHTTPServer((args.host, args.port), handler(cache, mission))
    server.daemon_threads = True
    threading.Thread(target=server.serve_forever, daemon=True).start()
    print("航点规划桥接已启动: http://%s:%d/api/state" % (args.host, args.port), flush=True)
    print("mission file: %s" % mission.path, flush=True)
    try:
        attach_ros(cache)
    except Exception as e:  # noqa: BLE001 - let systemd restart
        print("bridge ROS error: %s" % e, flush=True)
        raise
    finally:
        server.shutdown()


if __name__ == "__main__":
    main()
