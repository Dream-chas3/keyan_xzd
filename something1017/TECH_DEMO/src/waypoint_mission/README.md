# waypoint_mission

数据驱动的 OFFBOARD 航点任务：航点数量、每个航点是否拍照、云台角度都由
YAML 任务文件决定，改任务不用改 C++、不用重新编译。

## 安全启动门（重要）

节点启动后是 **IDLE**：不解锁、不切模式、不发布飞行设定点。

启动顺序：

1. 遥控器处于 **定点模式 POSCTL**，飞机未解锁。
2. 节点打印 `RC gate READY in POSCTL: switch to OFFBOARD slot 4 to start.`
3. 把 5 通道飞行模式开关从 Position 档（slot 6）拨到 **Offboard 档（slot 4）**。
4. 节点检测到“离开 Offboard → 再进入 Offboard”的新边沿后：
   - 先以 30 Hz 发当前位姿设定点 1.2 s（预发送）；
   - 请求 OFFBOARD；
   - 确认 OFFBOARD 后请求解锁；
   - 解锁后按 YAML 执行起飞和航点。

如果节点启动时开关已经在 Offboard 档，**不会启动**；必须先拨回定点，再拨入。
这和 `land_test` 的启动门一致（`RC_MAP_FLTMODE=5`、`COM_FLTMODE4=7`、
`COM_FLTMODE6=2`）。

任务过程中出现以下任一情况，节点立即停止发布设定点，并且**不会抢回模式**：

- 遥控器离开 Offboard 档、RC 信号丢失/超时；
- 飞控模式离开 OFFBOARD（人工接管）；
- `/Odometry` 或 `/mavros/local_position/pose` 超时（视觉定位停更）；
- 任务/单段超时。

停止设定点后由 PX4 的 `COM_OF_LOSS_T=1.0 s` 失联保护接管（当前参数会切
Position），飞手保持控制。节点不会自动重新请求 OFFBOARD；要重新执行任务需要
落地、上锁后重启节点，并重新做一次开关边沿。

## 任务文件

默认：`missions/abc_photo.yaml`。坐标是**本地 ENU**，和
`/mavros/local_position/pose` 同一坐标系；`z` 是绝对本地 z，不是对地高度。
地面时请确认 `local_position.z ≈ 0`。

每个航点字段：

| 字段 | 含义 |
|---|---|
| `x,y,z` | 本地 ENU 目标 |
| `yaw` | `auto`=按航段方向/下一段方向；`hold`=保持该段起始航向；数字=绝对 ENU 角度(deg) |
| `hold` | 到点后悬停秒数（拍照前建议 ≥1.5 s） |
| `photo` | 是否在点拍照 |
| `gimbal_pitch`, `gimbal_yaw` | 拍照前云台角度（度），`-90` 为垂直下视；不写则不动云台 |
| `speed` | 本段最大水平速度，省略用全局 `max_speed` |
| `tolerance` | 本点到点半径，省略用全局 |
| `timeout` | 本段超时，省略用全局 `leg_timeout` |

`return_to_start: true` 表示最后一个航点后回**起飞点**（不是第一个航点）。
如果 A 点就是起飞点，把 A 写成 `(0,0,takeoff_z)` 即可；或者删掉 A、只写 B/C，
任务结束回起飞点。

## 编译

在香橙派上：

```bash
cd ~/something1017/TECH_DEMO
catkin_make --only-pkg-with-deps waypoint_mission -j4
source devel/setup.bash
```

## 不启动电机的两种测试

### 1) dry_run：只解析/校验/打印任务

```bash
source ~/something1017/TECH_DEMO/devel/setup.bash
rosrun waypoint_mission waypoint_mission _dry_run:=true
```

不订阅、不发布、不解锁、不切模式。换任务文件：

```bash
rosrun waypoint_mission waypoint_mission _dry_run:=true \
  _mission_file:=$HOME/something1017/TECH_DEMO/src/waypoint_mission/missions/abc_photo.yaml
```

### 2) bench_mode：只验证遥控器启动门

```bash
rosrun waypoint_mission waypoint_mission _bench_mode:=true
```

节点仍然处于 IDLE，只打印 RC 槽位/READY；检测到 Offboard 边沿时只打印
`BENCH: ... No OFFBOARD request, no arm, no setpoints.`
**不发设定点、不解锁、不切模式。** 需要重新测试时先把开关拨回定点，再拨入。

## 正常执行

> ROS 参数会**留在 ROS master 上**，节点退出也不会自动清除。做过
> `_dry_run:=true` 或 `_bench_mode:=true` 之后，如果直接用 `rosrun` 跑任务，
> 请显式写 `_dry_run:=false _bench_mode:=false`；用本包的 launch 文件则已经显式
> 写 false。否则节点可能又只打印任务就退出。

终端 1、2 按手册先起定位：

```bash
slam
roslaunch fast_lio mapping_mid360.launch
# 另一个终端
slam
roslaunch to_mavros vision2mavros.launch
```

终端 3（云台相机 + 拍照节点 + 任务节点）：

```bash
demo
roslaunch waypoint_mission waypoint_mission.launch
```

或者分开：

```bash
demo
roslaunch rtsp_test rtsp_test.launch          # /rtsp_cam/image_raw
rosrun waypoint_mission waypoint_photo
rosrun waypoint_mission waypoint_mission _dry_run:=false _bench_mode:=false
```

确认 `/Odometry` 在更新（节点会拒绝在里程计超时的情况下启动），
`/mavros/local_position/pose` 在原点附近、模式 POSCTL、未解锁，然后按上面的
启动门拨开关。

拍照点悬停期间主循环仍以 30 Hz 发位置保持设定点；拍照/云台都在独立线程里，
不会因为 3 秒的抓帧或 2 秒的云台应答而切出 OFFBOARD。

## 输出

- 照片：`~/.ros/waypoint_mission/photos/<mission>_<timestamp>/wpXX_<name>.jpg`
- 任务日志：`~/.ros/waypoint_mission/logs/<mission>_<timestamp>.csv`
- 拍照记录：`~/.ros/waypoint_mission/logs/<mission>_<timestamp>_photos.csv`
- 状态话题：`/waypoint_mission/status`
- 拍照触发/应答：`/waypoint_mission/photo_trigger`、`/waypoint_mission/photo_done`

拍照节点订阅 `rtsp_test` 的 `/rtsp_cam/image_raw`（1920×1080）。如果没启动
RTSP 节点，拍照会返回 `FAIL no_image_topic_or_no_frame`；`photo_required: false`
时任务继续，`true` 时中止。

## 第一次实飞建议

1. `dry_run` 通过；
2. `bench_mode` 下确认开关边沿、槽位、READY 日志；
3. 保持 `photo_required: false`、`gimbal_required: false`，先把默认 1 m 高、
   小三角飞顺；
4. 确认 `mpc`/遥控器接管、急停；场地清空；
5. 再逐项加拍照、云台角度、更多航点。

## 限制

- 定位使用 `/Odometry`（FAST-LIO）和 `/mavros/local_position/pose` 的新鲜度；
  桥本身没有输入超时，所以这里直接检查 `/Odometry` 是否还在更新。
- 任务只支持本地 ENU 航点，不支持经纬度/GPS 航点。
- `yaw` 是 ENU 角度；当前 mavros `setpoint_raw` 会把 ROS 端的 ENU 转成 NED 发给
  PX4（与本平台已验证的 land_test 一致）。
- 任务结束支持：回起飞点降落、在最后一个航点原地降落、或原地悬停等飞手。
  想在某个指定点降落，就把那个点写成最后一个航点，并设
  `return_to_start: false`、`auto_land: true`。飞行中途不能临时改降落点。
- 节点不处理电池/避障；飞手和场地仍是最终安全边界。
- 启动时要求 `/Odometry` 新鲜；只开 mavros 而没有 FAST-LIO 时，
  `try_start` 会拒绝并打印 `start rejected: /Odometry stale`。先把定位拉起来，
  再拨开关；如果开关已经拨过，需要拨回定点再拨入。

## 坐标 frame

任务文件可以写：

```yaml
frame: local                 # 文件 x=启动机头前向, y=左侧（默认）
# 或
frame: x_right_y_forward     # 文件 x=右, y=前
```

`x_right_y_forward` 时节点内部自动转换为本地系（x前/y左），控制逻辑不变；
`dry_run` 会同时打印文件坐标和 `[local ...]`。上位机按任务 `frame` 显示。
当前香橙派任务文件使用 `x_right_y_forward`。

## 2026-10-29 首飞问题（已修复）

- 现象：拨 Offboard 后自动起飞，约 0.3 m 停住。
- 日志：`MISSION ABORT: fc_state_stale`。
- 原因：`/mavros/state` 实测约 **1.02 Hz**（最大间隔 1.002 s），节点旧阈值是
  `1.0 s`，起飞后恰好跨过两次 state，被误判为飞控状态丢失。
- 修复：`state_timeout` 可配置，默认 **3.0 s**；不再用 1.0 s。
- 结论：不是飞机/定位失控；节点停止设定点后 PX4 按 `COM_OF_LOSS_T=1.0 s`
  切 Position，飞机停在约 0.3 m。

## 2026-10-28 板端验证记录（未启动电机/未动云台）

- `catkin_make` 通过，`waypoint_mission`、`waypoint_photo` 两个可执行文件生成。
- `dry_run` 通过：3 航点默认任务和本机额外测试的 6 航点/3 拍照点任务都能正确解析。
- `bench_mode` 通过：真实 `/mavros/rc/in` 下状态为
  `phase=IDLE armed=0 mode=POSCTL rc_slot=6 rc_pwm=1913 gate_ready=1`，
  `/mavros/setpoint_raw/local` 没有新消息（未发设定点、未解锁）。
- 拍照链路通过：`rtsp_test` 的 `/rtsp_cam/image_raw` 约 30 Hz，
  手动触发后保存 1920x1080 JPEG（约 0.8 MB），应答 `OK /tmp/wm_photo_test.jpg`。
- 验证时 FAST-LIO/to_mavros 未运行，因此状态中 `odom_fresh=0`；这是预期的
  保护行为（节点不会在 `/Odometry` 缺失时启动任务）。


## 常见修改

### 1. 全程不主动转动机身

把每个航点的 `yaw` 都写成 `hold`：

```yaml
waypoints:
  - {name: A, x: 0.0, y: 0.0, z: 1.0, yaw: hold, hold: 0.0}
  - {name: B, x: 1.5, y: 0.0, z: 1.0, yaw: hold, hold: 2.0, photo: true, gimbal_pitch: -60}
  - {name: C, x: 1.5, y: -1.5, z: 1.0, yaw: hold, hold: 1.0}
```

`hold` 的含义是保持该航段起始航向，不做主动转向；飞机靠速度控制在水平面
平移（可以侧飞/后退）。如果受到扰动，偏航控制器仍会做小幅修正来维持航向，
但不会像 `auto` 那样在航点处转头。想改变拍照方向请用 `gimbal_yaw`，不要靠
机身转。

### 2. 不回起点，在最后一个航点原地降落

```yaml
return_to_start: false
auto_land: true
```

最后一个航点悬停/拍照结束后，节点继续保持该点的位置设定点并请求
`AUTO.LAND`，飞机从最后一个航点的正上方垂直下降；不会飞回起点。
最后一个航点请选在地面空旷、可垂直降落的位置。

- 不想自动降落、想自己接管：`auto_land: false`，飞机会在最后一点悬停。
- 想回起点降落：`return_to_start: true`、`auto_land: true`（默认）。

### 3. 增加/减少航点

直接编辑 `missions/abc_photo.yaml` 的 `waypoints:` 列表：

- **增加**：在列表里追加一条 `- {name: D, x: ..., y: ..., z: ..., yaw: ...}`；
- **减少**：删掉或注释掉对应的整条 `- {...}`。

不需要改代码、不需要 `catkin_make`。改完重启任务节点（Ctrl+C 后重新
`rosrun`/`roslaunch`）即可，节点只在启动时读 YAML。

新增点要注意：

- `z` 必须在 `min_z`/`max_z` 之间（默认 0.2~3.0 m）；
- 水平距离不能超过 `geofence_radius`（默认 5 m）；
- 想在新点拍照，就在那条里写 `photo: true` 和 `gimbal_pitch/gimbal_yaw`；
- 文件名里的 `wpXX` 按列表顺序编号，增删点后编号会变化，属正常。

改完建议先 `dry_run` 看一遍解析结果，再起飞：

```bash
rosrun waypoint_mission waypoint_mission _dry_run:=true
rosrun waypoint_mission waypoint_mission _dry_run:=false _bench_mode:=false
```

### 4. 三个需求一起改的完整例子

```yaml
mission:
  name: no_yaw_land_here
  takeoff_z: 1.0
  max_speed: 0.5
  max_z_speed: 0.25
  max_yaw_rate_deg: 25
  return_to_start: false       # 不回起点
  auto_land: true              # 在最后一个航点原地降落
  waypoints:
    - {name: A, x: 0.0, y: 0.0, z: 1.0, yaw: hold, hold: 0.0}
    - {name: B, x: 1.5, y: 0.0, z: 1.0, yaw: hold, hold: 2.0,
       photo: true, gimbal_pitch: -60, gimbal_yaw: 0}
    - {name: C, x: 1.5, y: -1.5, z: 1.0, yaw: hold, hold: 1.0}
    - {name: D, x: 0.5, y: -1.5, z: 1.0, yaw: hold, hold: 0.0}
```

任务结束后会在 D 点上方请求 AUTO.LAND。
