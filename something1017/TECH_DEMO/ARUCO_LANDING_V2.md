# ArUco 精准降落 v2.1：遥控器触发自主起降

以当前 `land_test`、`aruco_test` 为基线，实现《aruco码精准识别降落.docx》的前四项要求，并将第 5 项 ID/码尺寸配置纳入同一个 YAML。按后续确认的操作习惯，默认改为遥控器切入 OFFBOARD 触发自动起飞，无需第五个终端或 enable 命令。没有改动 PX4 参数、雷达定位、人物跟踪或上位机。没有进行新的真机飞行验证。

## 四项要求与实现

| 要求 | 实现 |
| --- | --- |
| 接近前不转向，接近后慢速调整 | 平移时偏航角速度目标为 0；水平误差 ≤10 cm、平移制动并稳定后进入转向。转向时不主动平移；偏差 >15 cm 时先制动转向再回到平移。默认转速 ≤0.06 rad/s，约 3.44°/s，可关闭码方向对齐。 |
| 对准稳定后下降；偏差或丢码时暂停 | 水平误差 ≤7 cm、角度误差 ≤5°且实际运动足够小，连续稳定 1 s 后允许下降。下降中误差 >12 cm、角度误差 >9°或观测失效时停止继续下降的目标，限加速度制动后悬停。恢复识别后重新对准和确认。 |
| 显式使能、人工接管后不抢回 | 节点启动为 IDLE，不发送设定点、解锁、模式或云台命令。默认遥控器从定点档切入 OFFBOARD 档触发自动起飞。人工切出 OFFBOARD 会结束本次任务；持续停在 OFFBOARD 不会自动恢复，必须回定点等 READY 后重新切入，才会尝试开始新任务。启动时开关已经在 OFFBOARD 不会触发，必须先拨出再拨入。取消、退出及析构均不会自动切 POSCTL 或解锁。 |
| YAML 速度/加速度约束和日志 | 水平向量速度、垂直速度、偏航速率以及对应加速度均限幅；固定 30 Hz 控制循环，不依赖图像帧率。输出阶段、误差、有效性、期望/实际下发速度等 CSV。 |

起飞完成采用单独的粗稳定门槛：离起飞点 ≤15 cm、高度误差 ≤8 cm、水平/垂直速度均 ≤0.10 m/s、偏航速率 ≤0.10 rad/s，连续满足 2 s。精准下降仍使用原有 7 cm、5°及其速度门槛。2026-10-27 的实飞日志显示旧的起飞门槛会把流程卡在 TAKEOFF；已用该记录做回归重放验证，不能把这项粗判定当作精准对准。上升限速仅从 0.25 小幅改为 0.28 m/s，垂直总限速同步为 0.28，下降速度仍为 0.08 m/s。

暂停下降不是瞬间刹停：默认下降 0.08 m/s、垂直制动加速度 0.30 m/s²，仅指令侧理论上约需 0.27 s 才减至 0，飞机响应还受实际动力学影响。近地例外使用最近且有效的视觉光轴深度判断，不能替代真实测距传感器。

## 默认流程：四个终端，遥控器触发自动起飞

以下是用户日后准备测试时的操作说明。本次安装不启动香橙派上的控制/识别节点，不发送解锁、模式或飞行指令。原来的服务使能版已被此默认流程取代。

1. 遥控器先放在已验证的**定点档**，不要一开始就停在 OFFBOARD。
2. 按原方法在终端 1 启动 FAST-LIO，终端 2 启动 `vision2mavros`。
3. 终端 3 保持 RTSP + ArUco 图像链。如果 RTSP 图像已在运行，可以用原识别命令；如果没有，可在同一个第三终端运行：

   ```bash
   ~/something1017/TECH/tools/start_aruco_vision.sh --no-view
   ```

   它会启动或复用 RTSP/ArUco，不需要额外终端。
4. 终端 4 执行原来的命令：

   ```bash
   demo
   roslaunch land_test Safina_land.launch
   ```

   程序此时不解锁、不起飞、不请求模式，也不发送设定点。它会检查新鲜遥控数据和已确认的 POSCTL；在定点档保持稳定后，终端显示 `RC gate READY in POSCTL`。
5. 准备执行任务时，**用遥控器从定点档切入 OFFBOARD 档**。只有这个新的开关动作才会启动任务：云台朝下并确认 → 预发送 1.2 s 零速度设定点 → 申请 OFFBOARD → 确认后自动申请解锁 → 升到当前配置 1.5 m → 悬停稳定 → 找码 → 分阶段平移/转向 → 对准后下降。

**不需要手动先起飞，不需要 `rosservice call ... true`，不需要第五个终端。** 默认 `start_trigger: rc_offboard`、`auto_takeoff: true`。启动时开关已经在 OFFBOARD 会一直等待，需要先回定点再重新切入。

程序读取的是 `/mavros/rc/in` 的实际模式开关信号，而不是只等待 `/mavros/state` 变成 OFFBOARD：PX4 需要预先收到设定点才能接受 OFFBOARD，所以第一次拨动可能先被飞控拒绝；程序识别该拨动后完成预发送，再替这一次明确的遥控请求申请模式。等待开关期间完全不发送飞行设定点。

人工切出 OFFBOARD 会立即取消任务。若飞控自行退出 OFFBOARD、遥控数据过期或任务失败，也会锁定取消；开关一直停在 OFFBOARD 不会自动重试。准备重新开始时，先回定点、等 READY，再重新拨入 OFFBOARD。默认自动起飞方式要求地面触发；空中人工接管后不会从空中重新起飞，应先人工降落后再开始新任务。短暂开关抖动不能启动；切出动作则优先立即停止本任务。

当前只读核对的飞控配置：`RC_MAP_FLTMODE=5`、`COM_FLTMODE4=7`（OFFBOARD），`RC5_MIN=1000`、`TRIM=1500`、`MAX=2000`、`DZ=10`、`REV=1`。YAML 已按此配置填写，第 5 通道约 1514 对应 OFFBOARD 档，第 6 模式档为 POSCTL。未修改飞控参数；如果之后改了遥控器映射或校准，也需同步修改 YAML。

### 自动起飞和丢码处理

- 起飞目标是在触发任务时的地面本地 Z 上增加 `takeOff_altitude`，当前配置 1.5 m；不要求地面本地 Z 为 0。
- 到达并连续稳定 `hover_seconds`（默认 2 s）后才开始初始找码计时。
- 此后 8 s 内未确认有效目标，制动后请求当前位置 AUTO.LAND。
- 确认过目标之后，普通丢码会暂停下降并等待恢复/人工接管，不会继续使用初始 8 s 截止时间。
- 已经在下降且最近目标对准、光轴深度 ≤0.25 m，短暂丢码 ≤0.6 s 可以交给最终 AUTO.LAND；长期旧数据不能触发此例外。
- 发出 AUTO.LAND 请求后保持零速度交接流，直到飞控状态确认已进入 AUTO.LAND 才停止发送；之后等待落地并上锁。`mode_sent` 不作为落地完成证据。

**最终 AUTO.LAND 的速度和加速度由 PX4 负责，不受本节点的 0.08 m/s 限制。** 本次没有修改飞控参数。

### 备用服务触发方式（默认不使用）

只有主动把 YAML 改为 `start_trigger: service` 才使用 `/aruco_landing/enable` 服务；设 `auto_takeoff: false` 可选择人工起飞后交接。默认遥控触发方式会拒绝 `enable true`，防止绕过遥控器开关。服务 `false` 仍可取消；通常应优先用遥控器切回人工模式。

取消或退出节点会停止任务输出，不会擅自切换模式。如果飞机仍在 OFFBOARD，停止输出会触发飞控已有的失联处理，不能把 Ctrl+C 当作悬停命令。

## 参数和视觉配置

共同配置：`~/something1017/TECH_DEMO/src/land_test/config/takeoff_land.yaml`。`Safina_land.launch` 和 `Safina_aruco.launch` 都加载此文件，修改后应在任务停止时重启相关节点，运行中的任务不会动态改变参数。

- `aruco_landing/align_yaw: false`：完全关闭根据码方向转向，保留平移和下降。
- `max_xy_speed: 0.15`、`descent_speed: 0.08`，单位 m/s。
- `max_yaw_rate: 0.06`，单位 rad/s；`max_xy_accel: 0.25`、`max_z_accel: 0.30`，单位 m/s²；`max_yaw_accel: 0.12`，单位 rad/s²。
- `xy_kp`、`z_kp`、`yaw_kp` 可后续调节，本次未引入积分/微分或自动 PID 整定。
- `aruco_marker/original_id: 0`、`original_size_m: 0.035`；`five_id: 231`、`five_size_m: 0.195`。尺寸是实际有效码边长（米），不包括纸张白边。ID 231 的边长已按用户确认从原模型 25.2 cm 改为现场 19.5 cm；小码 ID 0 尺寸尚未重新测量。
- 可以分别关闭某一类码；两个码同时可见时优先保持当前有效码，防止每帧跳选。无关 ORIGINAL ID 不再屏蔽有效的 5X5 目标，找到 5X5 目标后也不会被后续无关 ID 覆盖。
- `original_yaw_offset_deg`、`five_yaw_offset_deg` 用于实际打印码之间的朝向偏置，默认均为 0，需要按实物核对。
- 当前固定内参对应 1920×1080；默认拒绝不匹配的图像。修改 `calibrated_width/height` 本身不会重新标定或缩放内参，镜头/分辨率改变需要匹配标定结果。
- `camera_offset_x/y/z` 为相机原点相对机体的前/左/上偏移，默认 0，尚未测量标定。当前保留已用过的下视相机轴方向约定，未完成完整云台/横滚/俯仰外参标定。

视觉位姿改为非 latched，每帧重新初始化，无目标时 z=-1、旧 XY/角度清空；复制图像时间戳。若原 RTSP 图像没有时间戳，则使用检测接收时刻并打印提示，这不能测量或消除 RTSP 采集/网络延迟。控制器拒绝过期/无效观测，并以最近图像时刻配对本地位姿。

## 诊断日志

默认文件：`~/.ros/aruco_landing/landing_日期_时间_PID.csv`，默认 5 Hz；ROS 话题 `/aruco_landing/status` 是同一列顺序的 CSV 行，`/aruco_target_id` 输出所选 ID（无目标 -1）。

```bash
rostopic echo /aruco_landing/status
rostopic echo /aruco_target_id
```

日志包含：阶段、原因、任务使能、飞控模式/解锁、机体位置/航向/速度、目标年龄与世界坐标、水平误差、码角度/深度、期望与下发速度/偏航率、是否计划下降、模式/解锁/降落请求和初始找码经过时间。文件首行记录主要限幅参数，第二行是列名；末尾追加 RC 有效性、模式档位、OFFBOARD 选择、开关触发准备状态，以及起飞 XY/高度误差、未通过门槛位掩码和连续稳定秒数。未通过位：1=XY位置、2=高度、4=水平速度、8=垂直速度、16=偏航速率，可组合相加。

关键阶段：IDLE 等待；PRESTREAM 预发送/接管；TAKEOFF 自动起飞；ACQUIRE 找码；TRANSLATE 平移；ROTATE 转向；BRAKE_TURN 转向制动；STABILIZE 对准确认；DESCEND 下降；LOST_HOLD 丢码保持；LAND_HANDOFF 最终降落交接；AUTO_LAND 飞控降落；COMPLETE 完成；CANCELLED 锁定取消。

## 验证范围

- 独立 C++ 状态机测试覆盖 22 个场景，检查所有下发速度和加速度约束。
- 在 PC 的独立 localhost ROS master 下，用模拟飞控服务和生成的标记图像验证：启动零控制动作、遥控开关触发自动升高、启动已在 OFFBOARD 档时不动作、开关抖动不触发、重新拨动后才允许重启、备用服务接管、30 Hz 输出、人工模式切换后不抢回、退出不切模式、ID 配置、无目标清空、无关 ID 共存和分辨率拒绝。没有连接真机飞控。
- 香橙派临时工作区进行 ARM 编译和纯 C++ 测试，再安装源码和板端构建的可执行文件。未在香橙派启动控制或识别节点，未执行 enable、解锁或模式切换。
- 增加实飞记录回放：旧门槛保持 TAKEOFF，新门槛在记录开始约 14.2 s 后进入 TRANSLATE，并产生朝向已接受标记的速度。输入为 5 Hz 记录插值，只验证状态机回归，不模拟修改后的飞机动力学。
- 这些检查验证程序行为和构建，不代表已完成实机坐标方向、外参、延迟、制动性能或降落精度验收。

遥控档位换算参考：[PX4 RCUpdate v1.16](https://github.com/PX4/PX4-Autopilot/blob/v1.16.0/src/modules/rc_update/rc_update.cpp)、[PX4 COM_FLTMODE 参数](https://docs.px4.io/v1.16/en/advanced_config/parameter_reference#COM_FLTMODE1)。

参考：[PX4 Offboard](https://docs.px4.io/v1.16/en/flight_modes/offboard)、[OpenCV ArUco](https://docs.opencv.org/4.13.0/d5/dae/tutorial_aruco_detection.html)。


## 2026-10-27 只读三维上位机与目标诊断

桌面入口为 `arucu码识别与精准降落`，源码在 `../upper_station/`。香橙派桥接服务 `aruco-telemetry-bridge.service` 提供 8767 端口遥测；只订阅小型消息，不发送飞行控制，不传图像点云。本机记录、回放及导出说明见 `../upper_station/使用说明.txt`。

- `/aruco_detector/metrics`：JSON，含字典/全部可见 ID/选中 ID/检测原因/处理耗时/源时间戳可用性。
- `/aruco_landing/status_header`：现有 status CSV 的自描述表头，latched。旧列顺序保持；新增 marker_reason、源年龄、配对时差、跳变、接收/接受计数、目标估计 Z、反馈引导点及消息年龄。
- `/aruco_landing/accepted_target`、`active_goal`、`guidance_path`：本地位置坐标下的观测及当前反馈引导段；无有效引导时发布空 Path。目标必须结合 status 的有效性和消息新鲜度使用。
- 当前引导来自既有控制器的起飞/悬停/朝码闭环目标；没有增加新的全局路径规划器。下降 Z 端点为 1 秒速度前视示意，不能解释为相机地形测量或避障规划。
- 检测及位姿估计仍为原始标定分辨率；rqt 预览最大 960 像素、最多 8 Hz，画面增加 SELECTED TARGET / NO CONFIGURED TARGET 标签。OpenCV 使用 2 个工作线程，可在 YAML 配置。

本次没有更改控制速度、精度门限、失效保护、遥控触发或自动起飞设置。新诊断会在下次由用户重新启动视觉/降落节点时生效。


## v2.4 识别与短时丢码

降落节点启动可由 `gimbal_down_on_startup: true` 仅请求云台下视；飞行任务仍等待遥控触发。用户已授权这一启动时云台动作。
`SHORT_LOSS` 继承最近世界目标，0.6秒/0.05m/s/3cm触发制动预算；丢码不计划下降、不继续转向。重新观测确认后恢复，DESCEND恢复须重新稳定。3cm不代表真实机体可瞬间停止。
用户要求本轮以验证流程为先：下降准入10cm/10°/0.5秒，仍保持先平移后转向及丢码停降。
识别采用ROI、周期全图、局部对比度回退、原像素角点细化和两帧大小码切换确认。没有使用旧角点伪装新识别。
原始飞行分析和部署备份见 `../flight_review/20261027/recent_1426/分析与更新说明.txt`。

## v2.5 水平阻尼、视觉延迟补偿与终端下降（2026-10-27 17:33）

依据 2026-10-27 17:04 的上位机记录 `recordings/20261027_170401_230394/`：飞机在 z=1.58 m 开始下降，以 -0.08 m/s 降到 0.77 m；随后水平误差在 0.03~0.34 m 间以约 7 s 周期来回，峰峰值约 0.44 m，`target_x/y` 与飞机速度相关（估算视觉有效延迟 x≈0.22 s、y≈0.39 s），27 s 内未能重新满足下降门限，遥控切出取消。检测本身正常（飞行窗 153/154 帧有目标）。

本次改动：
- 水平环：`xy_kp 0.8→0.4`，新增实测速度阻尼 `xy_kd=0.35`（v = kp*e − kd*v_measured），`max_xy_speed 0.25→0.15`，`target_alpha 0.4→0.7`，`align_exit_error 0.16→0.22`，`stable_xy_speed 0.08→0.10`，`descent_speed 0.08→0.06`；`z_kd=0` 保留。
- 视觉延迟：新增 `image_latency_seconds: 0.30`。图像仍用接收时刻打戳，但控制器选配的飞机位姿改为“接收时刻 − 0.30 s”，用于消除 v×延迟造成的目标世界坐标偏移。设为 0 可关闭；位姿历史约 3 s，超出时自动回退。
- 相机偏置：`camera_offset_x: 0.10`（云台相机在机头，距机体中心约 10 cm）。控制器把相机原点前向偏置加到“相机→码”向量上，最终把**机体中心**驱动到码上方，而不是相机正下方。`camera_offset_y/z` 仍为 0；若相机不在中线或低于机体中心需补测。
- 终端下降：`terminal_descent_enabled: true`。DESCEND 中有两种在**仍然看得到码**时提交的方式：① 本地高度 z ≤ `terminal_height: 0.80` 且光轴深度 ≤ `terminal_loss_depth: 1.20`；② 光轴深度 ≤ `terminal_marker_depth: 0.85`。任一方式都要求水平误差 ≤0.10 m 连续 0.2 s。进入后冻结最后目标、以 0.06 m/s 原地下降，此后丢码不再暂停下降；边界为下降 1.0 m / 20 s，或偏离冻结目标 0.25 m（刹停悬停），到边界交 `LAND_HANDOFF`→AUTO.LAND，飞控落地检测也会提前接管。遥控切出仍然立即取消。
- 低空丢码回退：`terminal_loss_depth: 1.20`、`terminal_loss_error: 0.15`。若在 DESCEND 中丢码，且最后一次有效深度 ≤1.20 m、最后水平误差 ≤0.15 m、失效时间 ≤0.6 s，直接进入 `TERMINAL_DESCEND` 冻结最后目标继续下降，不再悬停。高于该深度丢码仍按 SHORT_LOSS/LOST_HOLD 处理。
- 低空原地保持：`terminal_hold_height`、`terminal_hold_error`。进入 `TERMINAL_DESCEND` 后，当本地高度低于门限且到冻结目标的水平偏差在门限内，进入低速位置保持（`terminal_hold_kp`、`terminal_hold_speed`），不再追新观测。注意不是零速度：零速度在 OFFBOARD 下不是位置保持，会被地面效应推走。判据用冻结目标（0.8 m 时的观测），不用低空新观测，因为后者在画面边缘容易有偏差。调大 `terminal_hold_error` 会更早停止水平修正但可能锁住偏差；调小更精确但停得更晚。

验证：板端 4 组 C++ 测试全过；私有 localhost ROS 隔离测试通过（模拟飞控，无真机解锁/模式请求）。安装时飞控未解锁、主 master 上未启动降落/识别节点，未发任何设定点。
注意：观测到 `marker_depth ≈ 1.42 × 本地 z`，说明当前 `five_size_m` 或相机内参与实际相差约 42%；`final_marker_distance=0.25` 基本不可达，终端下降成为实际生效的末端逻辑。建议用尺子复核 5X5 码有效边长，或按实测重新标定深度。

## 2026-10-28 10:15 板端用户参数调整

板端 `takeoff_land.yaml` 由用户调整为：`xy_kd=0.20`、`descent_speed=0.12`、`max_xy_speed=0.20`、`terminal_center_error=0.08`、`terminal_descent_speed=0.08`、`camera_offset_x=0.15`。`terminal_hold_height=0.35`、`terminal_hold_error=0.10` 已补回 YAML；之前缺失时代码使用同名默认值，行为相同但不可在 YAML 调整。`camera_offset_x=0.15` 与现场“约 10 cm”的说法不一致，待复核。

## 2026-10-28 10:29 低空保持漂移与修复

10:29 架次在 z=0.45 m 进入 `terminal_hold_in_place`（水平指令为 0），但 OFFBOARD 的零速度不是位置保持；从 0.45 降到 0.05 m 期间实际横漂约 23 cm，随后 `terminal_drift_guard_hold` 触发，`LOST_HOLD` 在 z≈0.04 悬停 23 s。10:22 架次未触发保持（漂移一直大于 0.10 m），落地但末端有水平摆动。

修复：保持阶段不再输出零速度，改为朝冻结目标的小增益位置保持：`terminal_hold_kp=0.8`、`terminal_hold_speed=0.08 m/s`，仍用 `xy_kd` 阻尼。这样既不追新观测，也不会被地面效应慢慢推走。隔离测试夹具将 `camera_offset_x/y/z` 归零，因为它的相机图像是静态的、不随飞机移动，非零偏置会造成恒定不可达误差。

## 2026-10-28 10:39 低空漂移与保持权限

10:39 架次 `terminal_hold_in_place` 已正常触发（0.45 m，drift 6 cm），但 0.19~0.13 m 期间地面效应把飞机横向推到约 12 cm/s；当时 `terminal_hold_speed=0.10` 已饱和（日志 cmd_vy=0.10），横向漂到 15 cm 后才被拉回，最终正常落地，未再触发 `LOST_HOLD`。

处置：`terminal_hold_kp` 0.8→1.2、`terminal_hold_speed` 0.10→0.15（`max_xy_speed` 同为 0.15）。若仍漂，下一步在 0.25 m 以下把盲降速度从 0.08 降到约 0.04 m/s，给保持环更多时间。

## 2026-10-28 11:08 航向漂移与低空交接

11:08 架次：`align_yaw=false` 下没有航向保持，yaw 在约 24 s 内从 -28° 漂到 -35°（约 0.3°/s），`marker_yaw` 约 -31.7°。低空 0.15 m 时实际垂直速度只有 -0.01~-0.05 m/s（指令 -0.08），飞机在 0.15 m 附近停留数秒，水平保持继续修正并在末端从 +4 cm 过冲到 -3 cm，看起来像“先保持再边降边退”；最后是 `terminal_timeout=15 s` 触发的交接。

改动：
- 新增柔和航向修正：`soft_yaw_enabled: true`、`soft_yaw_kp=0.3`、`soft_yaw_max_rate=0.06 rad/s`（约 3.4°/s）、`soft_yaw_deadband=0.05 rad`（约 2.9°）。只用方形码的最近边误差（fold 到 ±45°），只在 DESCEND/TERMINAL 生效，**不参与任何下降门限**，也不会因为角度大而暂停下降；允许与小幅水平修正同时进行。`align_yaw` 的严格 ROTATE 逻辑仍然关闭。
- 新增 `terminal_land_height: 0.15`：`TERMINAL_DESCEND` 中本地 Z ≤0.15 m 直接转 `LAND_HANDOFF → AUTO.LAND`，不再等 15 s 超时，避免地面附近继续水平修正造成后退/抖动。
- 隔离测试夹具将 `soft_yaw_enabled` 设为 false，以保留原有“航向关闭不转向”的测试语义；柔和航向由纯 C++ 测试覆盖（42 项）。

## 2026-10-28 11:35 落地“猛掉”与 AUTO.LAND 速度

11:08 架次交接后出现约 15 cm 的“猛掉”。日志显示 LAND_HANDOFF 发生在本地 Z=0.00（15 s 超时），随后 AUTO.LAND；读飞控参数 `MPC_LAND_SPEED=0.70 m/s`，是节点终端下降 0.08 m/s 的近 9 倍，15 cm 只需约 0.2 s，所以看起来像掉下去。这不是节点停电机：AUTO.LAND 会保持水平位置并下降，落地后才上锁。

处置：
- 通过 MAVROS 将飞控 `MPC_LAND_SPEED` 从 0.70 改为 **0.12 m/s**（回读确认）。该参数持久化在飞控里；恢复用同样的 param set 写回 0.7。
- 节点 `terminal_land_height` 从 0.15 改为 **0.05**（本地 Z），减少快速段长度。
- 观察：交接时本地 Z=0.00 而实际约 0.15 m，说明本地 Z 与实际离地高度约有 0.15 m 零点偏差；后续应按实测再校准 `terminal_land_height`，而不是直接按本地 Z 的数值理解。

## 2026-10-28 11:45 航向对齐固定为上边缘

用户实飞发现：手动顺时针转一点后切 OFFBOARD，柔和航向修正把机头对到了码的**右边缘**，而不是上边缘。原因是 `soft_yaw` 原先把角度 fold 到最近的 90°，只保证与四条边中最近的一条平行。

新增 `soft_yaw_to_top_edge: true`：目标角度改为“机头与 corner0→1（上）边缘平行”，即原始角 ±90°。右/左边缘属于垂直的那一对，会差 90°，因此会继续朝上/下这一对转。若当前停在右边缘，0.06 rad/s 下转 90° 约需 26 秒；想快可把 `soft_yaw_max_rate` 提到 0.09。该修正仍只在 DESCEND/TERMINAL 生效，不参与下降门限。

## 2026-10-28 11:55 跟踪环加入 I 项

降落节点的水平跟踪原来是 P（位置误差）+ D（实测速度阻尼），没有 I；包内的 `SimpleVelocityPID` 有 kp/ki/kd，但降落节点没有使用它。本次在 `desiredHorizontal()` 中加入可选 I：

- `xy_ki`（默认 0.05）、`xy_integral_limit`（默认 0.05 m/s）；`z_ki`（0.0）、`z_integral_limit`（0.05 m/s）。
- 积分对象是位置误差；I 的贡献按向量范数限幅（抗饱和），并在每次阶段切换、丢码、任务开始/取消时清零。
- 只作用于主水平/高度保持；`TERMINAL_DESCEND` 的低空冻结目标保持仍有自己的 P 律，不叠加 I。
- 建议：`xy_ki` 先 0.03~0.05，`xy_integral_limit` 0.03~0.05；若出现慢周期摆动就把 ki 减半；若存在稳定静差可加到 0.08~0.10。I 项只能消稳态偏差，不能解决低空地面效应造成的动态漂移。`z_ki` 建议保持 0，除非高度有稳定偏差。
