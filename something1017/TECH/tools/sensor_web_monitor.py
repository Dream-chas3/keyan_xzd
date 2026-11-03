#!/usr/bin/env python3
"""ROS Noetic sensor dashboard with a fixed local-RViz request interface."""

import json
import math
import os
import signal
import subprocess
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import rospy
from geometry_msgs.msg import PoseStamped
from livox_ros_driver2.msg import CustomMsg
from mavros_msgs.msg import State
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Image, Imu, PointCloud2


PORT = 8765
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
LOCK = threading.Lock()
DATA = {}
PROCESS_LOCK = threading.Lock()
PROCESSES = {}
RVIZ_LOCK = threading.Lock()
_RVIZ_EPOCH = int(time.time() * 1000)
RVIZ_STATE = {
    "request_id": _RVIZ_EPOCH,
    "request_action": None,
    "request_time": None,
    "ack_id": _RVIZ_EPOCH,
    "waiting_for_cloud": False,
    "state": "offline",
    "message": "等待本地 RViz 助手",
    "last_seen": None,
}
RVIZ_PENDING_TIMEOUT = 120.0
COMMANDS = {
    "fast_lio": ["roslaunch", "fast_lio", "mapping_mid360.launch"],
    "usb_camera": ["roslaunch", "usb_cam", "usb_cam.launch"],
    "vision_bridge": ["roslaunch", "to_mavros", "vision2mavros.launch"],
    "radar_stack": ["bash", os.path.join(TOOLS_DIR, "start_radar_position_stack.sh")],
}

SENSORS = [
    ("MID360 点云", "/livox/lidar", CustomMsg),
    ("FAST-LIO 注册点云", "/cloud_registered", PointCloud2),
    ("MID360 IMU", "/livox/imu", Imu),
    ("FAST-LIO 里程计", "/Odometry", Odometry),
    ("Orbbec 彩色图", "/camera/color/image_raw", Image),
    ("Orbbec 深度图", "/camera/depth/image_raw", Image),
    ("USB 相机", "/usb_cam/image_raw", Image),
    ("飞控 IMU", "/mavros/imu/data_raw", Imu),
    ("PX4 / MAVROS 状态", "/mavros/state", State),
    ("外部视觉位姿", "/mavros/vision_pose/pose", PoseStamped),
]

CONTROL_NAMES = {
    "fast_lio": "FAST-LIO",
    "usb_camera": "USB 相机",
    "vision_bridge": "外部视觉位姿桥",
    "radar_stack": "雷达定位链",
}
CONTROL_TOPICS = {
    "fast_lio": "/Odometry",
    "usb_camera": "/usb_cam/image_raw",
    "vision_bridge": "/mavros/vision_pose/pose",
}
MIN_RATES = {
    "/livox/lidar": 5.0,
    "/cloud_registered": 5.0,
    "/livox/imu": 100.0,
    "/Odometry": 5.0,
    "/camera/color/image_raw": 3.0,
    "/camera/depth/image_raw": 3.0,
    "/usb_cam/image_raw": 3.0,
    "/mavros/imu/data_raw": 20.0,
    "/mavros/state": 0.2,
    "/mavros/vision_pose/pose": 10.0,
}


def summary(topic, msg):
    if topic == "/livox/lidar":
        valid_points = sum(
            1 for point in msg.points
            if all(math.isfinite(v) for v in (point.x, point.y, point.z))
            and (point.x != 0.0 or point.y != 0.0 or point.z != 0.0)
        )
        return {"points": len(msg.points), "valid_points": valid_points}
    if topic == "/cloud_registered":
        point_count = msg.width * msg.height
        return {"points": point_count, "point_step": msg.point_step, "frame": msg.header.frame_id}
    if topic in ("/livox/imu", "/mavros/imu/data_raw"):
        a = msg.linear_acceleration
        return {"accel": [round(a.x, 3), round(a.y, 3), round(a.z, 3)]}
    if topic == "/Odometry":
        p = msg.pose.pose.position
        return {"position": [round(p.x, 4), round(p.y, 4), round(p.z, 4)]}
    if isinstance(msg, Image):
        return {"width": msg.width, "height": msg.height, "encoding": msg.encoding}
    if isinstance(msg, State):
        return {"connected": msg.connected, "armed": msg.armed, "mode": msg.mode}
    if isinstance(msg, PoseStamped):
        p = msg.pose.position
        return {"position": [round(p.x, 4), round(p.y, 4), round(p.z, 4)]}
    return {}


def message_health(topic, msg):
    if topic == "/livox/lidar":
        valid = summary(topic, msg)["valid_points"]
        return (valid > 0, "有效点 {} / {}".format(valid, len(msg.points)))
    if topic == "/cloud_registered":
        point_count = msg.width * msg.height
        valid = point_count > 0 and msg.point_step > 0 and msg.row_step >= msg.width * msg.point_step
        valid = valid and len(msg.data) >= msg.row_step * msg.height
        return (valid, "{} 点 · {}".format(point_count, msg.header.frame_id or "无坐标系") if valid else "空点云或消息长度无效")
    if topic in ("/livox/imu", "/mavros/imu/data_raw"):
        values = (
            msg.linear_acceleration.x, msg.linear_acceleration.y,
            msg.linear_acceleration.z, msg.angular_velocity.x,
            msg.angular_velocity.y, msg.angular_velocity.z,
        )
        valid = all(math.isfinite(value) for value in values)
        magnitude = math.sqrt(sum(value * value for value in values[:3])) if valid else 0.0
        valid = valid and 0.1 < magnitude < 40.0
        return (valid, "IMU 数值有效" if valid else "IMU 数值异常")
    if topic in ("/Odometry", "/mavros/vision_pose/pose"):
        pose = msg.pose.pose if isinstance(msg, Odometry) else msg.pose
        p, q = pose.position, pose.orientation
        values = (p.x, p.y, p.z, q.x, q.y, q.z, q.w)
        norm = math.sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w)
        valid = all(math.isfinite(value) for value in values)
        valid = valid and max(abs(p.x), abs(p.y), abs(p.z)) < 100.0 and 0.8 < norm < 1.2
        return (valid, "位姿数值/姿态有效" if valid else "位姿越界或四元数无效")
    if isinstance(msg, Image):
        valid = msg.width > 0 and msg.height > 0 and bool(msg.encoding)
        return (valid, "{}x{} {}".format(msg.width, msg.height, msg.encoding) if valid else "图像尺寸/编码无效")
    if isinstance(msg, State):
        return (msg.connected, "已连接" if msg.connected else "飞控未连接")
    return (True, "已收到消息")


def callback(topic):
    def receive(msg):
        now = time.time()
        with LOCK:
            item = DATA[topic]
            item["last"] = now
            item["count"] += 1
            item["times"].append(now)
            if len(item["times"]) > 64:
                item["times"].pop(0)
            item["summary"] = summary(topic, msg)
            item["quality_ok"], item["quality"] = message_health(topic, msg)
    return receive


HTML = r'''<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>PX4 传感器监视</title><style>
:root{font-family:system-ui,-apple-system,"Segoe UI",sans-serif;color:#17202b;background:#f3f5f7}*{box-sizing:border-box}body{margin:0}
header{background:#102a43;color:#fff;padding:18px 24px;display:flex;align-items:center;justify-content:space-between}h1{font-size:22px;margin:0}#connection{font-size:13px;color:#b9e6ff}
main{max-width:1400px;margin:22px auto;padding:0 16px}.table-wrap{overflow:auto;background:#fff;border:1px solid #d6dee6;border-radius:6px}table{width:100%;border-collapse:collapse;min-width:940px;font-size:13px}th{background:#e8edf1;color:#263746;text-align:left;font-weight:700}th,td{padding:9px 11px;border-bottom:1px solid #e3e8ed;vertical-align:middle}tbody tr:last-child td{border-bottom:0}.sensor-name{font-weight:650;white-space:nowrap}.topic{font:12px ui-monospace,monospace;color:#657386}.state{font-weight:700;white-space:nowrap}.ok{color:#137333}.wait{color:#a15c00}.rate{font-weight:700;white-space:nowrap}.detail{color:#4b5b6b;min-width:220px}.dot{width:8px;height:8px;border-radius:50%;display:inline-block;margin-right:6px;background:#9aa7b4}.dot.ok{background:#188038}.dot.wait{background:#d97706}
.controls{display:flex;gap:8px;margin-top:12px}.controls button{border:1px solid #b8c7d6;border-radius:5px;background:#fff;color:#19354d;padding:7px 10px;font:inherit;cursor:pointer}.controls button:hover:not(:disabled){background:#edf5fa}.controls button:disabled{opacity:.45;cursor:default}.action-note{font-size:12px;color:#607080;margin-top:8px;min-height:16px}
.toolbar{max-width:1400px;margin:16px auto 0;padding:0 16px;display:flex;align-items:center;gap:10px;flex-wrap:wrap}.toolbar button{border:1px solid #176b57;border-radius:5px;background:#176b57;color:white;padding:9px 14px;font:inherit;font-weight:650;cursor:pointer}.toolbar button.stop{background:white;border-color:#b44b42;color:#9c332c}.toolbar button.secondary{background:#245b9a;border-color:#245b9a}.toolbar button:disabled{opacity:.5;cursor:default}.toolbar-note{font-size:13px;color:#506070}.toolbar-status{margin-left:auto;font-size:13px;color:#334155}
@media(max-width:600px){header{padding:14px 16px}h1{font-size:19px}.toolbar{flex-wrap:wrap}.toolbar-status{margin-left:0}}
</style></head><body><header><h1>PX4 传感器与雷达定位工作台</h1><div id="connection">连接中...</div></header><section class="toolbar"><button id="radar-start" onclick="radarAction('start')">启动雷达定位链</button><button id="radar-stop" class="stop" onclick="radarAction('stop')" disabled>停止本台启动的定位链</button><span class="toolbar-note">只启动 FAST-LIO 和位姿转发；不解锁、不切模式、不起飞</span><span id="radar-status" class="toolbar-status"></span></section><section class="toolbar"><button id="rviz-start" class="secondary" onclick="rvizAction('start')" disabled>启动 FAST-LIO 并在本地打开 RViz</button><button id="rviz-stop" class="stop" onclick="rvizAction('stop')" disabled>关闭本地 RViz</button><span class="toolbar-note">FAST-LIO 在香橙派运行，点云通过 ROS 传到本地电脑渲染；不启动位姿桥</span><span id="rviz-status" class="toolbar-status">等待本地助手</span></section><main><div class="table-wrap"><table><thead><tr><th>传感器/数据流</th><th>话题</th><th>状态</th><th>Hz</th><th>采样龄期</th><th>数据摘要/有效性</th><th>操作</th></tr></thead><tbody id="grid"></tbody></table></div></main>
<script>
const grid=document.querySelector('#grid'), conn=document.querySelector('#connection'),radarStart=document.querySelector('#radar-start'),radarStop=document.querySelector('#radar-stop'),radarStatus=document.querySelector('#radar-status'),rvizStart=document.querySelector('#rviz-start'),rvizStop=document.querySelector('#rviz-stop'),rvizStatus=document.querySelector('#rviz-status');let controls={};
function esc(v){return String(v).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
function detail(x){let s=x.summary||{};if(s.points!==undefined)return `${s.points} 点`;if(s.position)return `位置 (${s.position.map(v=>Number(v).toFixed(3)).join(', ')}) m`;if(s.accel)return `加速度 (${s.accel.join(', ')})`;if(s.width)return `${s.width} x ${s.height} · ${esc(s.encoding)}`;if(s.mode!==undefined)return `${s.connected?'已连接':'未连接'} · ${s.armed?'已解锁':'未解锁'} · ${esc(s.mode||'模式未知')}`;return x.last? '收到消息':'--'}
const controlForTopic={'/Odometry':'fast_lio','/usb_cam/image_raw':'usb_camera','/mavros/vision_pose/pose':'vision_bridge'};
async function act(command,action){let note=command==='radar_stack'?radarStatus:document.querySelector(`#note-${command}`);if(note)note.textContent=action==='start'?'正在启动...':'正在停止...';try{let r=await fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command,action})}),d=await r.json();if(note)note.textContent=d.message||d.error||'操作完成';}catch(e){if(note)note.textContent='操作失败：'+e}poll()}
function radarAction(action){if(action==='start'&&!confirm('确认机体已拆桨并固定、PX4 未解锁？此操作只启动雷达定位与位姿转发，不会起飞。'))return;act('radar_stack',action)}
async function rvizAction(action){if(action==='start'&&!confirm('确认启动/复用 FAST-LIO 并在本地打开点云 RViz？不会启动 PX4 位姿桥、不会解锁或改变飞行模式。'))return;rvizStatus.textContent=action==='start'?'正在请求 FAST-LIO 与本地 RViz...':'正在请求关闭 RViz...';try{let r=await fetch('/api/rviz',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action})}),d=await r.json();rvizStatus.textContent=d.message||d.error||'请求已提交'}catch(e){rvizStatus.textContent='操作失败：'+e}poll()}
function renderRviz(d){rvizStart.disabled=!d.can_start;rvizStop.disabled=!d.can_stop;rvizStatus.textContent=d.message||'等待状态'}
function render(d){conn.textContent=`机载桥接在线 · ${new Date().toLocaleTimeString()}`;let radar=controls.radar_stack||{};radarStart.disabled=!radar.can_start;radarStop.disabled=!radar.can_stop;radarStatus.textContent=radar.running?(radar.external?'定位链已由其他终端运行':'定位链由本工作台运行'):(radar.reason||'');grid.innerHTML=d.sensors.map(x=>{let live=x.age!==null&&x.age<3, ok=live&&x.quality_ok, status=!live?'无新数据':(ok?'正常':'数据异常'), cls=ok?'ok':'wait', rate=live&&x.rate!==null?x.rate.toFixed(1):'--', age=live?x.age.toFixed(2)+' s':'--', key=controlForTopic[x.topic], ctl=key&&controls[key];let actions='';if(key){let label=ctl&&ctl.external?'终端运行':(ctl&&ctl.running?'运行中':'');let canStart=ctl&&ctl.can_start, canStop=ctl&&ctl.can_stop;actions=`<div class="controls"><button ${canStart?'':'disabled'} onclick="act('${key}','start')">启动</button><button ${canStop?'':'disabled'} onclick="act('${key}','stop')">停止</button><span class="action-note">${esc(label)}</span></div><div id="note-${key}" class="action-note">${ctl&&ctl.reason?esc(ctl.reason):''}</div>`}return `<tr><td class="sensor-name"><span class="dot ${cls}"></span>${esc(x.name)}</td><td class="topic">${esc(x.topic)}</td><td class="state ${cls}">${status}</td><td class="rate">${rate}</td><td>${age}</td><td class="detail">${detail(x)}${live&&x.quality?` · ${esc(x.quality)}`:''}</td><td>${actions||'--'}</td></tr>`}).join('')}
async function poll(){try{let [sr,cr,rr]=await Promise.all([fetch('/api/status',{cache:'no-store'}),fetch('/api/control',{cache:'no-store'}),fetch('/api/rviz',{cache:'no-store'})]);if(!sr.ok||!cr.ok||!rr.ok)throw Error('HTTP error');let [sd,cd,rd]=await Promise.all([sr.json(),cr.json(),rr.json()]);controls=cd.controls;render(sd);renderRviz(rd.rviz)}catch(e){conn.textContent='桥接离线 · '+e;rvizStart.disabled=true;rvizStop.disabled=true;rvizStatus.textContent='无法读取本地 RViz 状态';grid.innerHTML='<tr><td colspan="7">无法连接机载电脑监视服务</td></tr>'}setTimeout(poll,1000)}poll();
</script></body></html>'''


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/" or self.path == "/index.html":
            body = HTML.encode("utf-8")
            content_type = "text/html; charset=utf-8"
        elif self.path == "/api/status":
            now = time.time()
            with LOCK:
                sensors = []
                for name, topic, _ in SENSORS:
                    item = DATA[topic]
                    times = item["times"]
                    rate = ((len(times) - 1) / (times[-1] - times[0])) if len(times) > 1 and times[-1] > times[0] else None
                    rate_ok = rate is None or rate >= MIN_RATES.get(topic, 0.0)
                    quality = item.get("quality", "等待数据")
                    if rate is not None and not rate_ok:
                        quality += "；频率低于 {:.0f} Hz 参考值".format(MIN_RATES[topic])
                    sensors.append({"name": name, "topic": topic, "last": item["last"], "age": None if item["last"] is None else now - item["last"], "rate": rate, "summary": item["summary"], "quality_ok": item.get("quality_ok", False) and rate_ok, "quality": quality})
            body = json.dumps({"sensors": sensors}, ensure_ascii=False).encode("utf-8")
            content_type = "application/json; charset=utf-8"
        elif self.path == "/api/control":
            try:
                body = json.dumps({"controls": controls_status()}, ensure_ascii=False).encode("utf-8")
            except Exception as exc:
                rospy.logerr("control status failed: %s", exc)
                body = json.dumps({"controls": {}, "error": str(exc)}, ensure_ascii=False).encode("utf-8")
            content_type = "application/json; charset=utf-8"
        elif self.path == "/api/rviz":
            try:
                body = json.dumps({"rviz": rviz_status()}, ensure_ascii=False).encode("utf-8")
            except Exception as exc:
                rospy.logerr("RViz status failed: %s", exc)
                body = json.dumps({"rviz": {"state": "error", "message": str(exc)}}, ensure_ascii=False).encode("utf-8")
            content_type = "application/json; charset=utf-8"
        elif self.path == "/api/local_rviz":
            body = json.dumps(local_rviz_command(), ensure_ascii=False).encode("utf-8")
            content_type = "application/json; charset=utf-8"
        else:
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if self.path not in ("/api/control", "/api/rviz", "/api/local_rviz"):
            self.send_error(404)
            return
        try:
            size = int(self.headers.get("Content-Length", "0"))
            request = json.loads(self.rfile.read(size) or b"{}")
            if self.path == "/api/control":
                result = control_process(request.get("action"), request.get("command"))
            elif self.path == "/api/rviz":
                result = request_local_rviz(request.get("action"))
            else:
                result = report_local_rviz(request)
            body = json.dumps(result, ensure_ascii=False).encode("utf-8")
            self.send_response(200 if result.get("ok") else 400)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        except Exception as exc:
            self.send_error(400, str(exc))

    def log_message(self, *_):
        pass


def main():
    rospy.init_node("sensor_web_monitor", anonymous=True, disable_signals=True)
    for _, topic, msg_type in SENSORS:
        DATA[topic] = {"last": None, "count": 0, "times": [], "summary": {}, "quality_ok": False, "quality": "等待数据"}
        rospy.Subscriber(topic, msg_type, callback(topic), queue_size=1, tcp_nodelay=True)
    server = ThreadingHTTPServer(("0.0.0.0", PORT), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    print("Sensor dashboard: http://0.0.0.0:{}".format(PORT), flush=True)
    print("Local RViz control API ready at /api/rviz", flush=True)
    try:
        while not rospy.is_shutdown():
            time.sleep(1)
    finally:
        server.shutdown()
        stop_managed_processes()


def controls_status():
    with PROCESS_LOCK:
        for key, process in list(PROCESSES.items()):
            if process.poll() is not None:
                del PROCESSES[key]
        topics = {topic for topic, _ in rospy.get_published_topics("/")}
        with LOCK:
            odom_recent = DATA["/Odometry"]["last"] is not None and time.time() - DATA["/Odometry"]["last"] < 3.0
        result = {}
        for key in COMMANDS:
            managed = key in PROCESSES and PROCESSES[key].poll() is None
            if key == "radar_stack":
                external = not managed and {"/Odometry", "/mavros/vision_pose/pose"}.issubset(topics)
            else:
                external = not managed and CONTROL_TOPICS[key] in topics
            reason = ""
            if key == "radar_stack" and not managed and not external:
                reason = "启动前需传感器服务正常、MAVROS 已连接且未解锁"
            elif key == "vision_bridge" and not odom_recent:
                reason = "先启动 FAST-LIO，确认 /Odometry 有持续数据"
            elif external:
                reason = "对应话题已由其他终端发布，避免重复启动"
            result[key] = {
                "name": CONTROL_NAMES[key],
                "running": managed or external,
                "external": external,
                "can_start": not managed and not external and (key != "vision_bridge" or odom_recent),
                "can_stop": managed,
                "reason": reason,
            }
        return result


def rviz_status():
    topics = {topic for topic, _ in rospy.get_published_topics("/")}
    now = time.time()
    with LOCK:
        odom_last = DATA["/Odometry"]["last"]
        cloud_last = DATA["/cloud_registered"]["last"]
        cloud_ok = DATA["/cloud_registered"].get("quality_ok", False)
        lidar_last = DATA["/livox/lidar"]["last"]
        imu_last = DATA["/livox/imu"]["last"]
        lidar_ok = DATA["/livox/lidar"].get("quality_ok", False)
        imu_ok = DATA["/livox/imu"].get("quality_ok", False)
        mavros_state = DATA["/mavros/state"]
        odom_recent = odom_last is not None and now - odom_last < 3.0
        cloud_recent = cloud_last is not None and now - cloud_last < 3.0 and cloud_ok
        lidar_recent = lidar_last is not None and now - lidar_last < 3.0 and lidar_ok
        imu_recent = imu_last is not None and now - imu_last < 3.0 and imu_ok
        mavros_recent_disarmed = (
            mavros_state["last"] is not None
            and now - mavros_state["last"] < 3.0
            and mavros_state.get("summary", {}).get("connected") is True
            and mavros_state.get("summary", {}).get("armed") is False
        )
    with RVIZ_LOCK:
        state = dict(RVIZ_STATE)
    agent_online = state["last_seen"] is not None and now - state["last_seen"] < 8.0
    cloud_published = "/cloud_registered" in topics
    fastlio_published = "/Odometry" in topics or cloud_published
    ready = cloud_published and odom_recent and cloud_recent
    sensors_ready = lidar_recent and imu_recent
    active = agent_online and state["state"] in ("starting", "running", "stopping")
    state.update({
        "agent_online": agent_online,
        "agent_age": None if not agent_online else now - state["last_seen"],
        "cloud_published": cloud_published,
        "cloud_recent": cloud_recent,
        "odom_recent": odom_recent,
        "sensors_ready": sensors_ready,
        "mavros_recent_disarmed": mavros_recent_disarmed,
        "running": active,
        "can_start": agent_online and (ready or (sensors_ready and (fastlio_published or mavros_recent_disarmed))) and state["state"] in ("idle", "error"),
        "can_stop": agent_online and state["state"] in ("starting", "running", "stopping"),
    })
    if not agent_online:
        state["message"] = "本地助手离线；RViz 运行状态未知"
    elif not ready and not active and state["state"] != "error":
        if not sensors_ready:
            state["message"] = "先确认 /livox/lidar 与 /livox/imu 正常"
        elif not fastlio_published and not mavros_recent_disarmed:
            state["message"] = "启动 FAST-LIO 前需要 MAVROS 在线且 PX4 未解锁"
        else:
            state["message"] = "按按钮可启动或复用 FAST-LIO"
    return state


def request_local_rviz(action):
    if action not in ("start", "stop"):
        return {"ok": False, "error": "无效的 RViz 操作"}
    now = time.time()
    topics = {topic for topic, _ in rospy.get_published_topics("/")}
    try:
        code, _message, system_state = rospy.get_master().getSystemState()
        ros_nodes = {node for section in system_state for _topic, nodes in section for node in nodes} if code == 1 else set()
    except Exception:
        ros_nodes = set()
    with LOCK:
        odom_last = DATA["/Odometry"]["last"]
        cloud_item = DATA["/cloud_registered"]
        cloud_last = cloud_item["last"]
        cloud_ok = cloud_item.get("quality_ok", False)
        mavros_item = DATA["/mavros/state"]
        lidar_item = DATA["/livox/lidar"]
        imu_last = DATA["/livox/imu"]["last"]
        lidar_recent = lidar_item["last"] is not None and now - lidar_item["last"] < 3.0 and lidar_item.get("quality_ok", False)
        imu_recent = imu_last is not None and now - imu_last < 3.0
        odom_recent = odom_last is not None and now - odom_last < 3.0
        cloud_recent = cloud_last is not None and now - cloud_last < 3.0 and cloud_ok
    should_start_fastlio = False
    with RVIZ_LOCK:
        state = RVIZ_STATE
        agent_online = state["last_seen"] is not None and now - state["last_seen"] < 8.0
        if not agent_online:
            return {"ok": False, "error": "本地 RViz 助手未在线；请先在本地电脑启动助手"}
        if action == "start":
            if state["state"] in ("starting", "running", "stopping"):
                return {"ok": False, "error": "本地 RViz 已经启动或正在处理请求"}
            if not lidar_recent or not imu_recent:
                return {"ok": False, "error": "MID360 点云或 IMU 未就绪；先检查传感器服务和 Livox 数据"}
            should_start_fastlio = not odom_recent and "/Odometry" not in topics and "/laserMapping" not in ros_nodes
        elif state["state"] not in ("starting", "running", "stopping"):
            return {"ok": True, "message": "本地 RViz 当前未运行"}
        state["request_id"] += 1
        state["request_action"] = action
        state["request_time"] = now
        state["waiting_for_cloud"] = action == "start" and not ("/cloud_registered" in topics and odom_recent and cloud_recent)
        state["state"] = "starting" if action == "start" else "stopping"
        state["message"] = ("FAST-LIO 已有点云，正在本地启动 RViz..." if not state["waiting_for_cloud"] else "正在启动/等待 FAST-LIO 点云...") if action == "start" else "正在关闭本地 RViz..."
        request_id = state["request_id"]
    if action == "start" and should_start_fastlio:
        mavros_last = mavros_item.get("last")
        mavros_summary = mavros_item.get("summary", {})
        connected = mavros_summary.get("connected")
        armed = mavros_summary.get("armed")
        if mavros_last is None or now - mavros_last > 3.0 or connected is not True or armed is not False:
            with RVIZ_LOCK:
                if RVIZ_STATE["request_id"] == request_id:
                    RVIZ_STATE["state"] = "error"
                    RVIZ_STATE["message"] = "仅在 MAVROS 状态新鲜且 PX4 未解锁时允许网页启动 FAST-LIO"
                    RVIZ_STATE["waiting_for_cloud"] = False
                    RVIZ_STATE["ack_id"] = request_id
            return {"ok": False, "error": "仅在 MAVROS 状态新鲜且 PX4 未解锁时允许网页启动 FAST-LIO"}
        result = control_process("start", "fast_lio")
        if not result.get("ok"):
            latest_topics = {topic for topic, _ in rospy.get_published_topics("/")}
            if "/Odometry" not in latest_topics:
                with RVIZ_LOCK:
                    if RVIZ_STATE["request_id"] == request_id:
                        RVIZ_STATE["state"] = "error"
                        RVIZ_STATE["message"] = "FAST-LIO 启动失败：{}".format(result.get("error", "未知错误"))
                        RVIZ_STATE["waiting_for_cloud"] = False
                        RVIZ_STATE["ack_id"] = request_id
                return {"ok": False, "error": "FAST-LIO 启动失败：{}".format(result.get("error", "未知错误"))}
    return {"ok": True, "message": state["message"], "request_id": request_id}


def local_rviz_command():
    now = time.time()
    topics = {topic for topic, _ in rospy.get_published_topics("/")}
    with LOCK:
        odom_last = DATA["/Odometry"]["last"]
        cloud_item = DATA["/cloud_registered"]
        cloud_last = cloud_item["last"]
        cloud_ok = cloud_item.get("quality_ok", False)
        odom_recent = odom_last is not None and now - odom_last < 3.0
        cloud_recent = cloud_last is not None and now - cloud_last < 3.0 and cloud_ok
    with RVIZ_LOCK:
        state = RVIZ_STATE
        pending = state["request_id"] > state["ack_id"]
        if pending and state["request_time"] is not None and now - state["request_time"] > RVIZ_PENDING_TIMEOUT:
            state["ack_id"] = state["request_id"]
            state["state"] = "error"
            state["message"] = "本地助手未及时处理请求，请检查助手进程后重试"
            state["waiting_for_cloud"] = False
            pending = False
        if pending and state["waiting_for_cloud"] and "/cloud_registered" in topics and odom_recent and cloud_recent:
            state["waiting_for_cloud"] = False
            state["message"] = "FAST-LIO 点云就绪，正在本地打开 RViz..."
        command_ready = pending and not state["waiting_for_cloud"]
        return {
            "request_id": state["request_id"],
            "pending": pending,
            "command": {"id": state["request_id"], "action": state["request_action"]} if command_ready else None,
        }


def report_local_rviz(request):
    allowed_states = {"idle", "starting", "running", "stopping", "error"}
    state_name = request.get("state")
    if state_name not in allowed_states:
        return {"ok": False, "error": "无效的助手状态"}
    try:
        ack_id = int(request.get("ack_id", 0))
    except (TypeError, ValueError):
        return {"ok": False, "error": "无效的请求编号"}
    message = str(request.get("message", ""))[:240]
    with RVIZ_LOCK:
        RVIZ_STATE["last_seen"] = time.time()
        current_id = RVIZ_STATE["request_id"]
        # A heartbeat must not overwrite a newer browser request before the
        # helper has polled and acknowledged it.
        if ack_id >= current_id:
            RVIZ_STATE["state"] = state_name
            RVIZ_STATE["message"] = message or state_name
            RVIZ_STATE["ack_id"] = max(RVIZ_STATE["ack_id"], min(ack_id, current_id))
    return {"ok": True}


def control_process(action, command):
    if command not in COMMANDS or action not in ("start", "stop"):
        return {"ok": False, "error": "invalid command or action"}
    with PROCESS_LOCK:
        process = PROCESSES.get(command)
        if action == "start":
            if process is not None and process.poll() is None:
                return {"ok": True, "message": "already running"}
            if command == "radar_stack":
                topics = {topic for topic, _ in rospy.get_published_topics("/")}
                if {"/Odometry", "/mavros/vision_pose/pose"}.issubset(topics):
                    return {"ok": False, "error": "radar localization stack already has an external owner"}
            elif CONTROL_TOPICS[command] in {topic for topic, _ in rospy.get_published_topics("/")}:
                return {"ok": False, "error": "topic already has a publisher; refusing duplicate launch"}
            if command == "vision_bridge":
                with LOCK:
                    odom_recent = DATA["/Odometry"]["last"] is not None and time.time() - DATA["/Odometry"]["last"] < 3.0
                if not odom_recent:
                    return {"ok": False, "error": "FAST-LIO /Odometry is not active"}
            env = os.environ.copy()
            log_dir = os.path.expanduser("~/.ros/log/sensor_web_monitor")
            os.makedirs(log_dir, exist_ok=True)
            log_file = open(os.path.join(log_dir, command + ".log"), "ab", buffering=0)
            process = subprocess.Popen(
                COMMANDS[command],
                env=env,
                start_new_session=True,
                stdout=log_file,
                stderr=subprocess.STDOUT,
            )
            log_file.close()
            PROCESSES[command] = process
            return {"ok": True, "message": "started", "pid": process.pid}
        if process is None or process.poll() is not None:
            PROCESSES.pop(command, None)
            return {"ok": True, "message": "already stopped"}
        os.killpg(process.pid, signal.SIGINT)
        return {"ok": True, "message": "stopping"}


def stop_managed_processes():
    with PROCESS_LOCK:
        for process in list(PROCESSES.values()):
            if process.poll() is None:
                try:
                    os.killpg(process.pid, signal.SIGINT)
                except ProcessLookupError:
                    pass
        for key, process in list(PROCESSES.items()):
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                try:
                    os.killpg(process.pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
                try:
                    process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    try:
                        os.killpg(process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
                    process.wait()
            del PROCESSES[key]


if __name__ == "__main__":
    main()
