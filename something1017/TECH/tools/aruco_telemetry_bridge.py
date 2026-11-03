#!/usr/bin/env python3
"""ArUco station telemetry bridge: subscribers + HTTP GET only, no flight API.

Images/point clouds are deliberately not subscribed. No message-by-message disk
recording onboard. The workstation records compact 5 Hz snapshots instead.
"""
import argparse
import collections
import csv
import io
import json
import math
from pathlib import Path
import resource
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlsplit, parse_qs

VERSION = 'aruco-station-1.0'
LEGACY_FIELDS = ('ros_time steady_time phase reason enabled mode armed x y z yaw vx vy vz yaw_rate '
    'target_valid target_age target_x target_y xy_error marker_yaw marker_depth desired_vx desired_vy '
    'desired_vz desired_yaw_rate cmd_vx cmd_vy cmd_vz cmd_yaw_rate published planned_descent '
    'request_offboard request_arm request_land search_elapsed rc_valid rc_slot rc_offboard_selected '
    'rc_edge_ready takeoff_xy_error takeoff_z_error takeoff_failed_gates takeoff_hold_seconds').split()
TOPICS = {
    'state': '/mavros/state', 'pose': '/mavros/local_position/pose',
    'velocity': '/mavros/local_position/velocity_local', 'command': '/mavros/setpoint_raw/local',
    'marker': '/aruco_poseresult', 'id': '/aruco_target_id', 'controller': '/aruco_landing/status',
    'header': '/aruco_landing/status_header', 'detector': '/aruco_detector/metrics',
    'path': '/aruco_landing/guidance_path', 'battery': '/mavros/battery',
    'extended': '/mavros/extended_state', 'text': '/mavros/statustext/recv',
}


def clean(obj):
    if isinstance(obj, float) and not math.isfinite(obj): return None
    if isinstance(obj, dict): return {k: clean(v) for k, v in obj.items()}
    if isinstance(obj, (list, tuple)): return [clean(v) for v in obj]
    return obj


def scalar(value):
    try:
        x = float(value)
        return x if math.isfinite(x) else None
    except (TypeError, ValueError): return value


def parse_status(header, row):
    fields = next(csv.reader([header])) if header else LEGACY_FIELDS
    values = next(csv.reader([row]))
    return {k: scalar(v) for k, v in zip(fields, values)}


def xyz(v): return [v.x, v.y, v.z]


def pose(msg):
    p = msg.pose; q = p.orientation
    return {'position': xyz(p.position), 'quaternion': [q.x, q.y, q.z, q.w],
            'frame': msg.header.frame_id, 'stamp': msg.header.stamp.to_sec()}


class Cache:
    def __init__(self):
        self.lock = threading.Lock(); self.latest = {}; self.events = collections.deque(maxlen=30)
        self.rates = {}; self.last_store = {}; self.header = ''; self.sequence = 0
        self.config = {}; self.started = time.monotonic(); self.sent_bytes = 0

    def put(self, key, data, now=None):
        now = time.monotonic() if now is None else now
        with self.lock:
            times = self.rates.setdefault(key, collections.deque(maxlen=500)); times.append(now)
            if key == 'text': self.events.append({'time': time.time(), **data})
            self.latest[key] = (now, clean(data)); self.sequence += 1

    def snapshot(self, now=None):
        now = time.monotonic() if now is None else now
        with self.lock:
            topics = {}
            for key, (at, value) in self.latest.items():
                samples = [t for t in self.rates[key] if now-3 <= t <= now]
                hz = (len(samples)-1)/(samples[-1]-samples[0]) if len(samples)>1 else 0
                topics[key] = {'age': max(0, now-at), 'hz': round(hz, 2), 'data': value}
            elapsed = max(1, now-self.started); usage = resource.getrusage(resource.RUSAGE_SELF)
            return {'version': VERSION, 'server_time': time.time(), 'sequence': self.sequence,
                'topics': topics, 'events': list(self.events), 'config': self.config,
                'bridge': {'uptime': elapsed, 'cpu_average_percent_one_core':
                    100*(usage.ru_utime+usage.ru_stime)/elapsed, 'rss_mib': usage.ru_maxrss/1024,
                    'http_kib_per_second_average': self.sent_bytes/1024/elapsed,
                    'images_subscribed': False, 'read_only': True}}

    def logs_allowed(self):
        s = self.snapshot()['topics'].get('state', {})
        return s.get('age', 999) < 2 and s.get('data', {}).get('connected') and not s['data']['armed']


def attach_ros(cache):
    import rospy
    from std_msgs.msg import String, Int32
    from geometry_msgs.msg import PoseStamped, TwistStamped
    from nav_msgs.msg import Path as RosPath
    from sensor_msgs.msg import BatteryState
    from mavros_msgs.msg import State, ExtendedState, PositionTarget, StatusText
    rospy.init_node('aruco_telemetry_bridge', disable_signals=True)
    def header(m): cache.header = m.data
    def status(m): cache.put('controller', parse_status(cache.header, m.data))
    def detector(m):
        try: cache.put('detector', json.loads(m.data))
        except (ValueError, TypeError): pass
    converters = {
        'state': (State, lambda m: {'mode': m.mode, 'armed': m.armed, 'connected': m.connected}),
        'pose': (PoseStamped, pose), 'marker': (PoseStamped, pose),
        'velocity': (TwistStamped, lambda m: {'linear': xyz(m.twist.linear), 'angular': xyz(m.twist.angular)}),
        'command': (PositionTarget, lambda m: {'velocity': xyz(m.velocity), 'yaw_rate': m.yaw_rate,
            'frame': m.coordinate_frame, 'mask': m.type_mask}),
        'id': (Int32, lambda m: m.data),
        'path': (RosPath, lambda m: {'frame': m.header.frame_id,
            'points': [xyz(p.pose.position) for p in m.poses[:20]]}),
        'battery': (BatteryState, lambda m: {'voltage': m.voltage, 'percentage': m.percentage}),
        'extended': (ExtendedState, lambda m: {'landed_state': m.landed_state}),
        'text': (StatusText, lambda m: {'severity': m.severity, 'text': m.text[:500]}),
    }
    subs = [rospy.Subscriber(TOPICS['header'], String, header, queue_size=1),
            rospy.Subscriber(TOPICS['controller'], String, status, queue_size=1),
            rospy.Subscriber(TOPICS['detector'], String, detector, queue_size=1)]
    for key, (typ, convert) in converters.items():
        # Conversion is tiny; ROS callback and HTTP snapshots share no network waits.
        subs.append(rospy.Subscriber(TOPICS[key], typ,
            lambda m, k=key, f=convert: cache.put(k, f(m)), queue_size=1, tcp_nodelay=True))
    def configuration(_=None):
        try:
            values = {key: rospy.get_param('/'+key, {}) for key in
                ('aruco_landing', 'aruco_marker', 'takeOff_altitude')}
            with cache.lock: cache.config = values
        except Exception: pass
    configuration(); timer = rospy.Timer(rospy.Duration(30), configuration)
    return subs, timer


def make_handler(cache, log_dir):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *_): pass
        def do_GET(self):
            url = urlsplit(self.path)
            if url.path == '/api/state':
                body = json.dumps(cache.snapshot(), ensure_ascii=False, allow_nan=False).encode()
                self.send_body(body, 'application/json; charset=utf-8'); return
            if url.path in ('/api/logs', '/api/log'):
                if not cache.logs_allowed():
                    self.send_error(409, 'Log download requires fresh, connected, DISARMED state'); return
                files = sorted(log_dir.glob('landing_*.csv'), key=lambda p: p.stat().st_mtime, reverse=True)[:30]
                if url.path == '/api/logs':
                    body = json.dumps([{'name': p.name, 'bytes': p.stat().st_size} for p in files]).encode()
                    self.send_body(body, 'application/json'); return
                name = parse_qs(url.query).get('name', [''])[0]
                chosen = next((p for p in files if p.name == name), None)
                if chosen is None or chosen.is_symlink() or chosen.stat().st_size > 32*1024*1024:
                    self.send_error(404); return
                # Bound this optional post-flight transfer to 32 MiB.
                self.send_body(chosen.read_bytes(), 'text/csv'); return
            self.send_error(404)
        def send_body(self, body, content_type):
            self.send_response(200); self.send_header('Content-Type', content_type)
            self.send_header('Content-Length', str(len(body))); self.send_header('Cache-Control', 'no-store')
            self.end_headers()
            try:
                self.wfile.write(body)
                with cache.lock: cache.sent_bytes += len(body)
            except (BrokenPipeError, ConnectionResetError): pass
    return Handler


def main():
    p = argparse.ArgumentParser();p.add_argument('--bind', default='0.0.0.0')
    p.add_argument('--port', type=int, default=8767); args=p.parse_args()
    cache=Cache(); ros_handles=attach_ros(cache)
    server=ThreadingHTTPServer((args.bind,args.port),make_handler(cache,Path.home()/'.ros/aruco_landing'))
    server.daemon_threads=True
    try: server.serve_forever(poll_interval=.3)
    except KeyboardInterrupt: pass
    finally: server.server_close()

if __name__ == '__main__': main()
