#!/usr/bin/env python3
"""人物跟踪 · 香橙派只读遥测桥接

ROS -> HTTP JSON，只订阅话题，不发布任何消息、不调用任何服务。
默认端口 8769，PC 端 person_track_station.py 通过 HTTP GET 轮询。

订阅：
  /mavros/state
  /mavros/local_position/pose
  /mavros/local_position/velocity_local
  /mavros/extended_state
  /mavros/rc/in
  /Safina_track
  /Safina_target_id
  /mavros/battery

用法：
  source /opt/ros/noetic/setup.bash
  python3 track_telemetry_bridge.py --port 8769
"""
import argparse
import collections
import json
import math
import resource
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

VERSION = "person-track-bridge-1.0"


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
                hz = (len(samples) - 1) / (samples[-1] - samples[0]) if len(samples) > 1 else 0.0
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


def attach_ros(cache, up_max_pwm, mid_max_pwm):
    import rospy
    from geometry_msgs.msg import PoseStamped, TwistStamped
    from mavros_msgs.msg import ExtendedState, RCIn, State
    from sensor_msgs.msg import BatteryState
    from std_msgs.msg import Float32MultiArray, Int32

    rospy.init_node("track_telemetry_bridge", disable_signals=True)

    def state_cb(msg):
        cache.put("state", {
            "mode": msg.mode,
            "armed": bool(msg.armed),
            "connected": bool(msg.connected),
        })

    def pose_cb(msg):
        cache.put("pose", pose_data(msg))

    def velocity_cb(msg):
        cache.put("velocity", {
            "linear": xyz(msg.twist.linear),
            "angular": xyz(msg.twist.angular),
        })

    def extended_cb(msg):
        cache.put("extended", {"landed_state": int(msg.landed_state)})

    def rc_cb(msg):
        pwm = int(msg.channels[8]) if len(msg.channels) > 8 else None
        if pwm is None:
            swa = "unknown"
        elif pwm <= up_max_pwm:
            swa = "up"
        elif pwm <= mid_max_pwm:
            swa = "mid"
        else:
            swa = "down"
        cache.put("rc", {"pwm9": pwm, "swa": swa, "channels": len(msg.channels)})

    def track_cb(msg):
        data = list(msg.data)
        record = {
            "state": int(data[0]) if len(data) > 0 else -1,
            "x_offset": float(data[1]) if len(data) > 1 else 0.0,
            "y_offset": float(data[2]) if len(data) > 2 else 0.0,
            "id": int(data[3]) if len(data) > 3 else -1,
            "distance_m": float(data[4]) if len(data) > 4 else 0.0,
            "distance_source": int(data[5]) if len(data) > 5 else 0,
            "distance_valid": bool(data[6] > 0.5) if len(data) > 6 else False,
        }
        cache.put("track", record)

    def target_cb(msg):
        cache.put("target", {"id": int(msg.data)})

    def battery_cb(msg):
        cache.put("battery", {
            "voltage": float(msg.voltage),
            "percentage": float(msg.percentage),
        })

    subs = [
        rospy.Subscriber("/mavros/state", State, state_cb, queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/mavros/local_position/pose", PoseStamped, pose_cb, queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/mavros/local_position/velocity_local", TwistStamped, velocity_cb,
                         queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/mavros/extended_state", ExtendedState, extended_cb, queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/mavros/rc/in", RCIn, rc_cb, queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/Safina_track", Float32MultiArray, track_cb, queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/Safina_target_id", Int32, target_cb, queue_size=1, tcp_nodelay=True),
        rospy.Subscriber("/mavros/battery", BatteryState, battery_cb, queue_size=1, tcp_nodelay=True),
    ]
    return subs


def make_handler(cache):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_GET(self):
            if self.path.split("?")[0] == "/api/state":
                body = json.dumps(cache.snapshot(), ensure_ascii=False, allow_nan=False).encode("utf-8")
                self.send_body(body, "application/json; charset=utf-8")
                return
            if self.path.split("?")[0] == "/api/health":
                self.send_body(b'{"ok":true}', "application/json")
                return
            self.send_error(404)

        def send_body(self, body, content_type):
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            try:
                self.wfile.write(body)
            except (BrokenPipeError, ConnectionResetError):
                pass

    return Handler


def main():
    parser = argparse.ArgumentParser(description="人物跟踪只读遥测桥接")
    parser.add_argument("--bind", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8769)
    parser.add_argument("--up-max-pwm", type=int, default=1211,
                        help="SWA 上档最大 PWM，默认 1211（上档≈1075）")
    parser.add_argument("--mid-max-pwm", type=int, default=1757,
                        help="SWA 中档最大 PWM，默认 1757（中档≈1494，下档≈1913）")
    args = parser.parse_args()

    cache = Cache()
    attach_ros(cache, args.up_max_pwm, args.mid_max_pwm)

    server = ThreadingHTTPServer((args.bind, args.port), make_handler(cache))
    server.daemon_threads = True
    thread = threading.Thread(target=server.serve_forever, kwargs={"poll_interval": 0.3}, daemon=True)
    thread.start()

    print("人物跟踪桥接已启动: http://%s:%d/api/state" % (args.bind, args.port))
    print("只读：不发布话题、不调用服务、不订阅图像。Ctrl+C 退出。")
    try:
        import rospy
        rospy.spin()
    except KeyboardInterrupt:
        pass
    finally:
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    main()
