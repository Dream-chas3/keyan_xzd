#!/usr/bin/env python3
"""Read-only ROS flight telemetry, browser dashboard and durable JSONL recorder.

No ROS publishers, service calls, process launchers or FCU commands. ROS imports
are confined to attach_ros(), so the recorder and HTTP API can be tested offline.
"""
import argparse
import collections
import csv
import hashlib
import io
import json
import math
import os
from pathlib import Path
import queue
import shutil
import threading
import time
from datetime import datetime
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit

TESTS = {"aruco": "ArUco 精准降落", "track": "人物跟踪", "waypoint": "Offboard 航点"}
TOPICS = {
    "state": "/mavros/state", "pose": "/mavros/local_position/pose",
    "velocity": "/mavros/local_position/velocity_local", "odom": "/Odometry",
    "vision": "/mavros/vision_pose/pose", "aruco": "/aruco_poseresult",
    "command": "/mavros/setpoint_raw/local", "position_command": "/mavros/setpoint_position/local",
    "target": "/mavros/setpoint_raw/target_local", "track": "/Safina_track",
    "extended": "/mavros/extended_state", "battery": "/mavros/battery",
    "rc": "/mavros/rc/in", "status_text": "/mavros/statustext/recv",
}
CSV_FIELDS = ["time", "mode", "armed", "x", "y", "z", "yaw", "vx", "vy", "vz",
              "marker_x", "marker_y", "marker_z", "marker_error", "marker_yaw",
              "cmd_vx", "cmd_vy", "cmd_vz", "cmd_yaw_rate", "cmd_frame", "cmd_mask",
              "pose_age", "marker_age", "cmd_age", "track_state", "track_x", "track_y"]
CSV_FIELDS += ["goal_x", "goal_y", "goal_z", "goal_error"]


def clean(value):
    if isinstance(value, float) and not math.isfinite(value):
        return None
    if isinstance(value, dict):
        return {k: clean(v) for k, v in value.items()}
    if isinstance(value, (list, tuple)):
        return [clean(v) for v in value]
    return value


def vector(v):
    return [v.x, v.y, v.z]


def yaw_degrees(q):
    norm = sum(v*v for v in (q.x, q.y, q.z, q.w))
    if not math.isfinite(norm) or norm < 1e-8:
        return None
    return math.degrees(math.atan2(2*(q.w*q.z+q.x*q.y)/norm,
                                   1-2*(q.y*q.y+q.z*q.z)/norm))


def pose_data(pose):
    q = pose.orientation
    return {"position": vector(pose.position), "quaternion": [q.x, q.y, q.z, q.w],
            "yaw": yaw_degrees(q)}


def convert(key, msg):
    if key in ("pose", "vision", "aruco", "position_command"):
        return pose_data(msg.pose)
    if key == "odom":
        return dict(pose_data(msg.pose.pose), velocity=vector(msg.twist.twist.linear))
    if key == "velocity":
        return {"linear": vector(msg.twist.linear), "angular": vector(msg.twist.angular)}
    if key in ("command", "target"):
        mask = msg.type_mask
        return {"position": vector(msg.position), "velocity": vector(msg.velocity),
                "acceleration": vector(msg.acceleration_or_force), "yaw": math.degrees(msg.yaw),
                "yaw_rate": math.degrees(msg.yaw_rate), "frame": msg.coordinate_frame, "mask": mask,
                "position_active": [not bool(mask & (1 << i)) for i in range(3)],
                "velocity_active": [not bool(mask & (1 << (i+3))) for i in range(3)],
                "yaw_active": not bool(mask & 1024), "yaw_rate_active": not bool(mask & 2048)}
    if key == "state":
        return {"connected": msg.connected, "armed": msg.armed, "mode": msg.mode,
                "system_status": msg.system_status}
    if key == "extended":
        return {"landed_state": msg.landed_state, "vtol_state": msg.vtol_state}
    if key == "track":
        return {"values": list(msg.data)}
    if key == "battery":
        return {"voltage": msg.voltage, "current": msg.current, "percentage": msg.percentage}
    if key == "rc":
        return {"channels": list(msg.channels), "rssi": msg.rssi}
    return {"severity": msg.severity, "text": msg.text}


class Recorder:
    def __init__(self, directory, budget=1024*1024*1024):
        self.directory = Path(directory).expanduser()
        self.directory.mkdir(parents=True, exist_ok=True)
        self.budget = budget
        self.messages = queue.Queue(maxsize=8192)
        self.dropped = 0
        self.error = None
        self.filename = None
        self.written = 0
        self.closed = False
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def put(self, item):
        try:
            self.messages.put_nowait(item)
            return True
        except queue.Full:
            self.dropped += 1
            return False

    def segment(self, test, reason, metadata):
        name = datetime.now().strftime("%Y%m%d_%H%M%S_%f") + "_" + test + ".jsonl"
        item = {"kind": "segment", "time": time.time(), "file": name,
                "test": test, "reason": reason, "metadata": metadata, "schema": 1}
        if not self.put(item):
            raise RuntimeError("记录队列已满，未能创建记录段")
        return name

    def _run(self):
        handle = None
        total = sum(p.stat().st_size for p in self.directory.glob("*.jsonl"))
        last_sync = 0.0
        try:
            while True:
                try:
                    item = self.messages.get(timeout=0.5)
                except queue.Empty:
                    item = {}
                if item is None:
                    break
                try:
                    if item.get("kind") == "segment":
                        if handle:
                            handle.flush()
                            os.fsync(handle.fileno())
                            handle.close()
                        handle = (self.directory / item["file"]).open("x", encoding="utf-8")
                        self.filename = item["file"]
                        self.written = 0
                        self.error = None
                    if item and handle:
                        if total >= self.budget:
                            raise OSError("记录目录达到容量上限，请下载并清理旧记录")
                        if self.written == 0 or time.monotonic() - last_sync >= 1:
                            if shutil.disk_usage(self.directory).free < 200*1024*1024:
                                raise OSError("磁盘剩余不足 200 MB，已停止写入")
                        line = json.dumps(clean(item), ensure_ascii=False, allow_nan=False) + "\n"
                        count = len(line.encode("utf-8"))
                        handle.write(line)
                        total += count
                        self.written += count
                    if handle and time.monotonic() - last_sync >= 1:
                        handle.flush()
                        os.fsync(handle.fileno())
                        last_sync = time.monotonic()
                except (OSError, ValueError) as exc:
                    self.error = str(exc)
                    if handle:
                        handle.close()
                        handle = None
                    # Resume only when a new segment is explicitly queued.
        finally:
            if handle:
                handle.flush()
                os.fsync(handle.fileno())
                handle.close()
            self.closed = True

    def close(self):
        if self.closed:
            return
        self.messages.put(None)
        self.thread.join(timeout=10)

    def files(self):
        result = []
        for path in sorted(self.directory.glob("*.jsonl"), reverse=True)[:100]:
            info = path.stat()
            result.append({"name": path.name, "size": info.st_size, "mtime": info.st_mtime})
        return result

    def path(self, name):
        if Path(name).name != name or not name.endswith(".jsonl"):
            raise ValueError("无效的记录文件名")
        path = self.directory / name
        if path.is_symlink() or not path.is_file():
            raise ValueError("记录文件不存在")
        return path


class FlightMonitor:
    def __init__(self, directory, test="aruco", metadata=None):
        self.lock = threading.RLock()
        self.recorder = Recorder(directory)
        self.test = test
        self.metadata = metadata or {}
        self.latest = {}
        self.rates = collections.defaultdict(lambda: collections.deque(maxlen=120))
        self.history = collections.deque(maxlen=1800)
        self.events = collections.deque(maxlen=60)
        self.sequence = 0
        self.publishers = {}
        self.master_error = None
        self.started = time.monotonic()
        self.segment_started = self.started
        self.current_file = self.recorder.segment(test, "monitor_start", self.metadata)

    def new_segment(self, test, reason="manual"):
        if test not in TESTS:
            raise ValueError("未知测试类型")
        with self.lock:
            name = self.recorder.segment(test, reason, self.metadata)
            self.test, self.current_file = test, name
            self.segment_started = time.monotonic()
            # Keep telemetry continuity and the pre-arm trajectory on screen.
            self.event("记录段", TESTS[test] + " · " + reason)
            for key, item in self.latest.items():
                self.recorder.put({"kind": "context", "time": time.time(), "key": key,
                                   "topic": TOPICS[key], "sample": item})
            return name

    def event(self, title, text):
        event = {"kind": "event", "time": time.time(), "title": title, "text": text}
        self.events.append(event)
        self.recorder.put(event)

    def receive(self, key, data, stamp=None, frame="", caller=""):
        wall, mono = time.time(), time.monotonic()
        data = clean(data)
        with self.lock:
            previous = self.latest.get(key, {}).get("data", {})
            if key == "state" and data != previous:
                if data.get("armed") and not previous.get("armed"):
                    self.new_segment(self.test, "armed_detected")
                for field in ("armed", "mode", "connected"):
                    if data.get(field) != previous.get(field):
                        self.event(field, str(previous.get(field)) + " → " + str(data.get(field)))
            if key == "status_text":
                self.event("PX4", data.get("text", ""))
            self.latest[key] = {"data": data, "time": wall, "mono": mono,
                                "stamp": stamp, "frame": frame, "caller": caller}
            self.rates[key].append(mono)
            self.recorder.put({"kind": "telemetry", "time": wall, "key": key,
                               "topic": TOPICS[key], "stamp": stamp, "frame": frame,
                               "caller": caller, "data": data})

    def point(self):
        now = time.monotonic()
        def value(key, field, index=None):
            item = self.latest.get(key)
            if not item or now-item["mono"] > 1.5:
                return None
            result = item["data"].get(field)
            if index is not None:
                return result[index] if isinstance(result, list) and len(result) > index else None
            return result
        def age(key):
            item = self.latest.get(key)
            return now-item["mono"] if item else None
        p = dict.fromkeys(CSV_FIELDS)
        p.update(time=time.time(), mode=value("state", "mode"), armed=value("state", "armed"),
                 yaw=value("pose", "yaw"), marker_yaw=value("aruco", "yaw"),
                 cmd_frame=value("command", "frame"), cmd_mask=value("command", "mask"),
                 pose_age=age("pose"), marker_age=age("aruco"), cmd_age=age("command"))
        for i, axis in enumerate("xyz"):
            p[axis] = value("pose", "position", i)
            p["v"+axis] = value("velocity", "linear", i)
            p["marker_"+axis] = value("aruco", "position", i)
            if value("command", "velocity_active", i):
                p["cmd_v"+axis] = value("command", "velocity", i)
        if value("command", "yaw_rate_active"):
            p["cmd_yaw_rate"] = value("command", "yaw_rate")
        if p["marker_z"] is not None and p["marker_z"] > 0:
            if p["marker_x"] is not None and p["marker_y"] is not None:
                p["marker_error"] = math.hypot(p["marker_x"], p["marker_y"])
        else:
            for field in ("marker_x", "marker_y", "marker_yaw"):
                p[field] = None
        for i, field in enumerate(("track_state", "track_x", "track_y")):
            p[field] = value("track", "values", i)
        command = self.latest.get("command", {})
        position = self.latest.get("position_command", {})
        if (command.get("mono", 0) > position.get("mono", 0) and
                value("command", "frame") == 1 and
                all(value("command", "position_active", i) for i in range(3))):
            for i, axis in enumerate("xyz"):
                p["goal_"+axis] = value("command", "position", i)
        elif position.get("mono", 0) > command.get("mono", 0):
            for i, axis in enumerate("xyz"):
                p["goal_"+axis] = value("position_command", "position", i)
        if all(p[k] is not None for k in ("x", "y", "goal_x", "goal_y")):
            p["goal_error"] = math.hypot(p["x"]-p["goal_x"], p["y"]-p["goal_y"])
        return clean(p)

    def sample(self):
        with self.lock:
            if time.monotonic()-self.segment_started >= 1800 or self.recorder.written >= 64*1024*1024:
                self.new_segment(self.test, "rotation")
            self.sequence += 1
            p = dict(self.point(), seq=self.sequence)
            self.history.append(p)
            self.recorder.put({"kind": "sample", "time": p["time"], "data": p})

    def state(self, after=0):
        now = time.monotonic()
        with self.lock:
            topics = {}
            for key, topic in TOPICS.items():
                item = self.latest.get(key)
                times = self.rates[key]
                recent = [t for t in times if now-t < 5]
                rate = (len(recent)-1)/(recent[-1]-recent[0]) if len(recent)>1 else None
                topics[key] = {"topic": topic, "age": now-item["mono"] if item else None,
                               "rate": rate, "data": item["data"] if item else None,
                               "stamp": item["stamp"] if item else None,
                               "stamp_age": time.time()-item["stamp"] if item and item["stamp"] else None,
                               "frame": item["frame"] if item else "",
                               "caller": item["caller"] if item else "",
                               "publishers": self.publishers.get(topic, [])}
            point = self.point()
            warnings = []
            def fresh(key):
                age = topics[key]["age"]
                return age is not None and age < 1.5
            if not fresh("state"):
                warnings.append("飞控状态没有新数据，模式和解锁状态未知")
            elif not (topics["state"]["data"] or {}).get("connected"):
                warnings.append("MAVROS 报告飞控未连接")
            if not fresh("pose"):
                warnings.append("PX4 本地位姿没有新数据")
            if self.test == "aruco":
                if not fresh("aruco"):
                    warnings.append("ArUco 消息断流；当前不能判断是否对准")
                elif point["marker_error"] is None:
                    warnings.append("未识别有效降落标记（z≤0 或数值无效）")
                if point["cmd_yaw_rate"] is not None and abs(point["cmd_yaw_rate"]) > 1:
                    warnings.append("程序正在发送旋转指令：%.1f °/s" % point["cmd_yaw_rate"])
                if (point["cmd_vz"] is not None and point["cmd_vz"] < -0.01 and
                        point["marker_error"] is not None and point["marker_error"] > 0.10):
                    warnings.append("水平视觉偏差 >10 cm 时仍在下发下降速度")
            if self.test == "track" and (not fresh("track") or point["track_state"] == -1):
                warnings.append("跟踪结果断流或目标丢失")
            cmd = topics["command"]["data"] or {}
            if fresh("command") and len(topics["command"]["publishers"]) > 1:
                warnings.append("原始控制话题存在多个发布者：" + ", ".join(topics["command"]["publishers"]))
            if (fresh("command") and fresh("position_command") and
                    topics["command"]["publishers"] != topics["position_command"]["publishers"]):
                warnings.append("位置与原始设定点同时更新，请核对是否有多条控制链")
            if fresh("command") and cmd.get("frame") in (8, 9):
                warnings.append("速度指令使用机体系；曲线中的实际速度使用本地坐标系，不能逐轴直接比较")
            if self.recorder.error:
                warnings.append("记录故障：" + self.recorder.error)
            if self.recorder.dropped:
                warnings.append("记录队列累计丢弃 %d 条消息" % self.recorder.dropped)
            if self.master_error:
                warnings.append("ROS master 查询失败：" + self.master_error)
            points = [p for p in self.history if p["seq"] > after][-600:]
            return {"time": time.time(), "test": self.test, "tests": TESTS,
                    "sequence": self.sequence,
                    "point": point, "topics": topics, "points": points,
                    "events": list(self.events), "warnings": warnings,
                    "master_error": self.master_error,
                    "recording": {"file": self.current_file, "error": self.recorder.error,
                                  "dropped": self.recorder.dropped, "queued": self.recorder.messages.qsize(),
                                  "bytes": self.recorder.written, "directory": str(self.recorder.directory)}}


def make_handler(monitor):
    class Handler(BaseHTTPRequestHandler):
        def reply(self, status, body, mime="application/json; charset=utf-8", filename=None):
            if not isinstance(body, bytes):
                body = json.dumps(clean(body), ensure_ascii=False, allow_nan=False).encode()
            self.send_response(status)
            self.send_header("Content-Type", mime)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            if filename:
                self.send_header("Content-Disposition", 'attachment; filename="%s"' % filename)
            self.end_headers()
            self.wfile.write(body)

        def do_GET(self):
            parts = urlsplit(self.path)
            query = parse_qs(parts.query)
            try:
                if parts.path == "/":
                    self.reply(200, Path(__file__).with_name("flight_monitor.html").read_bytes(), "text/html; charset=utf-8")
                elif parts.path == "/api/state":
                    self.reply(200, monitor.state(int(query.get("after", [0])[0])))
                elif parts.path == "/api/files":
                    self.reply(200, {"files": monitor.recorder.files()})
                elif parts.path in ("/api/download", "/api/replay"):
                    path = monitor.recorder.path(query.get("name", [""])[0])
                    if parts.path == "/api/replay":
                        points = collections.deque(maxlen=18000)
                        events = collections.deque(maxlen=200)
                        meta = {}
                        with path.open(encoding="utf-8") as source:
                            for line in source:
                                try:
                                    item = json.loads(line)
                                except ValueError:
                                    continue  # A live file may have a partial final line.
                                if item.get("kind") == "sample":
                                    points.append(item["data"])
                                elif item.get("kind") == "event":
                                    events.append(item)
                                elif item.get("kind") == "segment":
                                    meta = item
                        self.reply(200, {"points": list(points), "events": list(events), "metadata": meta,
                                         "limit": 18000})
                    elif query.get("format", [""])[0] == "csv":
                        output = io.StringIO()
                        writer = csv.DictWriter(output, CSV_FIELDS, extrasaction="ignore")
                        writer.writeheader()
                        with path.open(encoding="utf-8") as source:
                            for line in source:
                                try:
                                    item = json.loads(line)
                                except ValueError:
                                    continue
                                if item.get("kind") == "sample":
                                    writer.writerow(item["data"])
                        self.reply(200, b"\xef\xbb\xbf"+output.getvalue().encode(), "text/csv; charset=utf-8", path.stem+".csv")
                    else:
                        # Bounded by segment size; snapshot excludes any later appends.
                        self.reply(200, path.read_bytes(), "application/x-ndjson", path.name)
                else:
                    self.reply(404, {"error": "未知路径"})
            except (ValueError, OSError) as exc:
                self.reply(400, {"error": str(exc)})

        def do_POST(self):
            # The only mutation creates a local log segment; never controls ROS.
            if self.path != "/api/segment":
                self.reply(404, {"error": "未知路径"})
                return
            try:
                size = int(self.headers.get("Content-Length", "0"))
                if not 0 < size <= 2048:
                    raise ValueError("无效的请求长度")
                request = json.loads(self.rfile.read(size))
                name = monitor.new_segment(request.get("test", monitor.test))
                self.reply(200, {"file": name})
            except (ValueError, RuntimeError, AttributeError) as exc:
                self.reply(400, {"error": str(exc)})

        def log_message(self, *_):
            pass
    return Handler


def source_metadata():
    root = Path.home()/"something1017"
    result = {"note": "sources at monitor startup; not a guarantee of executable identity", "files": {}}
    for rel in ["TECH_DEMO/src/land_test/src/Safina_land_test.cpp",
                "TECH_DEMO/src/land_test/config/takeoff_land.yaml",
                "TECH_DEMO/src/aruco_test/src/Safina_aruco_test.cpp",
                "TECH_DEMO/devel/lib/land_test/Safina_land_test"]:
        path = root/rel
        if path.is_file():
            data = path.read_bytes()
            result["files"][rel] = {"sha256": hashlib.sha256(data).hexdigest(),
                                     "mtime": path.stat().st_mtime}
            if path.suffix == ".yaml":
                result["files"][rel]["text"] = data.decode("utf-8", "replace")
    return result


def attach_ros(monitor):
    import rospy
    from geometry_msgs.msg import PoseStamped, TwistStamped
    from mavros_msgs.msg import State, PositionTarget, ExtendedState, StatusText, RCIn
    from nav_msgs.msg import Odometry
    from sensor_msgs.msg import BatteryState
    from std_msgs.msg import Float32MultiArray
    rospy.init_node("flight_data_monitor", anonymous=False, disable_signals=True)
    types = {"state": State, "pose": PoseStamped, "velocity": TwistStamped, "odom": Odometry,
             "vision": PoseStamped, "aruco": PoseStamped, "command": PositionTarget,
             "position_command": PoseStamped, "target": PositionTarget, "track": Float32MultiArray,
             "extended": ExtendedState, "battery": BatteryState, "rc": RCIn, "status_text": StatusText}
    def callback(key):
        def receive(msg):
            try:
                header = getattr(msg, "header", None)
                stamp = header.stamp.to_sec() if header else None
                monitor.receive(key, convert(key, msg), stamp, header.frame_id if header else "",
                                getattr(msg, "_connection_header", {}).get("callerid", ""))
            except Exception as exc:
                rospy.logwarn_throttle(5, "flight monitor message error: %s", exc)
        return receive
    subscriptions = [rospy.Subscriber(topic, types[key], callback(key), queue_size=20,
                                     tcp_nodelay=True) for key, topic in TOPICS.items()]
    def inspect_master():
        while not rospy.is_shutdown():
            try:
                code, message, state = rospy.get_master().getSystemState()
                if code != 1:
                    raise RuntimeError(message)
                with monitor.lock:
                    monitor.publishers = dict(state[0])
                    monitor.master_error = None
            except Exception as exc:
                monitor.master_error = str(exc)
            time.sleep(5)
    threading.Thread(target=inspect_master, daemon=True).start()
    return rospy, subscriptions


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8766)
    parser.add_argument("--log-dir", default="~/.ros/flight_monitor")
    parser.add_argument("--test", choices=TESTS, default="aruco")
    parser.add_argument("--offline", action="store_true", help="HTTP/recorder check without ROS or fabricated telemetry")
    args = parser.parse_args()
    monitor = FlightMonitor(args.log_dir, args.test, source_metadata())
    server = None
    ros = None
    subscriptions = []
    try:
        if not args.offline:
            ros, subscriptions = attach_ros(monitor)
        server = ThreadingHTTPServer((args.host, args.port), make_handler(monitor))
        threading.Thread(target=server.serve_forever, daemon=True).start()
        print("Flight monitor: http://%s:%d · %s" % (args.host, args.port, args.log_dir), flush=True)
        while ros is None or not ros.is_shutdown():
            started = time.monotonic()
            monitor.sample()
            time.sleep(max(0.0, 0.2-(time.monotonic()-started)))
    except KeyboardInterrupt:
        pass
    finally:
        for subscription in subscriptions:
            subscription.unregister()
        if ros is not None:
            ros.signal_shutdown("flight monitor stopped")
        if server:
            server.shutdown()
            server.server_close()
        monitor.recorder.close()


if __name__ == "__main__":
    main()
