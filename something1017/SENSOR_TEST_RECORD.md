# Orange Pi + PX4 传感器初测记录

## 2026-10-27 10:36 飞行未获得目标，增加诊断与桌面三维上位机（已部署）

- 10:36 飞行已完成起飞稳定，10:37:13.750 进入 ACQUIRE；8.03 s 无有效目标后原地交接 AUTO.LAND，10:37:23.250 切回 POSCTL 取消。561 条诊断的 target_valid 全 0、target_age 全 -1；没有进入朝码平移/精准下降。与 10:14 的起飞门限问题不同。rqt 显示 231 由使用者确认；历史数据缺少每帧选中 ID 和拒绝原因，不能确证最初故障点。
- 详细分析、原始日志、时序图和验证结果位于 `flight_review/20261027/recent_1036/`，参见 `分析说明.txt`。当前地面识别为 `5X5_INVERTED:231`、19.5 cm，约 11～12 Hz；110 帧只读时序检查通过所核对的时效/配对门限，随后实际 IDLE 控制器也显示 accepted 计数增长、published=0。没有把当前地面结果当作已通过实飞验证。
- 控制器增加自描述诊断、目标拒绝原因/计数/时间差、目标世界估计位置及当前反馈引导 Path；视觉增加字典/ID 列表、选中目标、处理耗时。预览缩至宽 960、最多 8 Hz，原分辨率识别/标定不变，OpenCV 默认 2 线程。原控制速度、对准门限、遥控触发和失效策略保持。
- 桌面入口 `arucu码识别与精准降落`；源码 `upper_station/`。三维飞机/已接受目标/引导/实际轨迹、模式与数据监视、本地记录、回放、后台分析包导出均可用。条件下降线为示意，没有新增障碍地图或避障规划。坐标为本地 ENU，Z 不是离地高度。
- 香橙派只读桥接 `aruco-telemetry-bridge.service`，8767 端口，已启用开机自启。无飞行服务/设定点输出，不订阅点云/图像；PC 5 Hz 拉取并绘制/记录。地面窗口约 28 KB/s、44 MiB RSS、7.7% 单核 CPU（约 8 核总容量的 1%）；不代表最差负载保证。
- 本机与 ARM 四组离线测试、私有 ROS 隔离测试、五项上位机单元测试、真实 LAN 记录与后台导出、桌面显示/三维/回放通过。安装备份 `~/.ros/backups/aruco_station_20261027_111821/original.tar.gz`，校验清单见分析目录。
- 最终审计发现 YAML 于 11:27:59 在部署后再次改为 `auto_takeoff: false`，仅该文件与部署清单不同；运行参数也为 false。保留该后续修改，没有覆盖。当前是手动起飞后接管，不能描述为切 OFFBOARD 自动起飞。
- 安装时无人机未解锁、无飞行控制节点。只启动只读桥接，做过一次短时相机读取并结束自建读取进程。没有启动降落节点、解锁、使能、切模式、发送飞行设定点或云台动作，也没有启动电机；随后出现的常规视觉/IDLE 降落节点由使用者自行启动。

## 2026-10-27 10:14 实飞反馈：TAKEOFF 卡住与大码尺寸修正（10:33 已写入，未启动）

- 已下载 `landing_20261027_101446_4660.csv`、ROS/视觉日志，分析材料位于 `flight_review/20261027/recent_1020/`。10:14:53 进入 TAKEOFF，约 10:15:03 已到目标 1.5 m 附近，10:15:47 遥控切出取消前始终未进入 ACQUIRE/TRANSLATE/DESCEND。246 个 TAKEOFF 诊断样本中 169 个目标有效，说明识别结果已被控制器接受，但任务没有进入跟踪阶段。
- 直接原因是上一版起飞完成复用了精细稳定门槛：起点 XY 8 cm、速度 0.06 m/s、偏航速率 0.03 rad/s 等连续保持 2 s。记录采样上同时通过最长约 1.24 s，横向摆动和偏航速率波动反复清零计时。程序主要在守起飞点，因此没有向码中心移动，也尚未启动到高稳定后的 8 s 找码计时。
- 起飞完成条件独立为 XY 15 cm、高度误差 8 cm、水平/垂直速度 0.10 m/s、偏航速率 0.10 rad/s，仍连续稳定 2 s。精确下降的 7 cm/5°等门槛不变。上升仅从 0.25 小幅提高至 0.28 m/s（+12%），总垂直限速同步；下降 0.08 m/s、加速度 0.30 m/s²和高度 1.5 m 保持。
- 用户确认 ID 231 实际有效边长 19.5 cm，原配置 25.2 cm 会造成约 29.2% 平移/深度尺度放大，已修正 `five_size_m: 0.195`。原日志深度约 2.68 m，按尺寸比例约变为 2.07 m，仍与 1.5 m 本地上升量有差异，待核验相机标定/缩放/安装姿态；没有凭比例修改内参，小码尺寸也未擅自改变。
- 增加起飞失败门槛/连续稳定时间诊断，以及这次真实记录的离线回归。旧门槛一直 TAKEOFF，新门槛约在触发后 14.2 s 进入 TRANSLATE 并产生朝向被接受标记的速度。回归基于 5 Hz 测量插值，不等同于修改后实机动力学。22 个场景、四组 C++ 测试、本地隔离 ROS 和 ARM 构建通过。
- 10:33:48 已备份并安装源码、参数及 ARM 可执行文件，安装前后均未解锁、POSCTL，安装时已无降落控制进程。备份：`~/.ros/backups/aruco_takeoff_gate_20261027_103346/original.tar.gz`。没有启动、停止或重启任何机载控制/视觉节点，没有使能/解锁/切模式/飞行设定点命令，没有启动电机。
- 新码尺寸和新控制逻辑需用户下次在地面重新启动第三个视觉脚本和第四个降落节点后读取。前两个雷达定位终端无需因本次更新而重启。

## 2026-10-27 10:09 配置回退导致遥控无响应（已修复磁盘配置，节点未重启）

- 用户启动日志为 `Trigger=service auto_takeoff=false`。只读核对发现实际 YAML 于 09:48:05 修改，变回旧服务触发内容，缺失 `start_trigger` 及全部 RC 配置；`takeOff_altitude` 为 1.5。包路径正确，逐项核验源码、可执行文件和其他部署文件均匹配，只有 YAML 与部署清单不同。无法确定具体覆盖操作。
- 仅恢复 YAML 的 `start_trigger: rc_offboard`、`auto_takeoff: true` 和已核对的 RC 开关参数，保留现有 1.5 m 高度及所有其他控制/视觉参数。本地 YAML 同步到相同内容，解析/差异/校验和检查通过。
- 原文件备份于板端 `~/.ros/backups/aruco_config_repair_20261027_100910/`，本地修复记录为 `flight_review/20261027/config_repair_report.json`。
- 当前 PID 4907 的控制节点未停止或重启；未写 ROS 参数，实时参数仍是其启动时的旧值。用户需在第四终端 Ctrl+C 后重新 launch，确认 `Trigger=rc_offboard auto_takeoff=true` 和 `RC gate READY in POSCTL`。保持定点可只检查等待状态，无需切 OFFBOARD。
- 本次没有启动控制节点，没有调用使能/解锁/切模式服务，没有发布飞行设定点；只读看到未解锁、POSCTL。没有启动电机。

## 2026-10-27 09:44 后续修正：四终端 + 遥控器 OFFBOARD 触发（当前版本）

- 按用户澄清，默认流程已从服务使能/手动起飞改为：先定点档，启动第四个 `Safina_land.launch` 等 READY，然后遥控器切入 OFFBOARD，程序自动升高、找码、对准和降落。无需第五个终端或 `rosservice ... true`。以下上一段的“默认手动起飞”属于被本版本替换的历史流程。
- 默认 `start_trigger: rc_offboard`、`auto_takeoff: true`。启动时开关已经在 OFFBOARD 不触发；必须先回定点等 READY，再拨入。人工接管/失败后一直停在 OFFBOARD 不重试；重新触发需要新的明确开关动作，默认自动起飞模式只允许地面触发。
- 只读核对 PX4：`RC_MAP_FLTMODE=5`、`COM_FLTMODE4=7`，RC5 校准 1000/1500/2000、死区 10、未反向。YAML 按实际值解码 RC 信号；未修改任何飞控参数。等待期间无设定点/模式/解锁请求，识别到明确开关动作后才预发送并请求模式。
- PC 隔离 ROS 模拟验证了 RC 触发自动升高、启动已在 OFFBOARD 时不动作、退出取消、抖动不触发、重新切入才能重启以及备用服务方式；香橙派 ARM 编译和三组纯 C++ 测试通过。保留原有对准/下降/丢码/速度加速度约束。
- 09:44:59 安装完成，前后均 `armed: false, mode: POSCTL`。本次未启动香橙派控制或识别节点，未调用使能/解锁/模式服务，未发送飞行设定点，电机未由助手启动。未做真机飞行验证。
- 备份：`~/.ros/backups/aruco_rc_trigger_20261027_094457/original.tar.gz`。说明书 `TECH_DEMO/ARUCO_LANDING_V2.md` 已同步更新；部署/验证记录在 `flight_review/20261027/rc_trigger_*`。

## 2026-10-27 ArUco 降落 v2（已部署，未启动控制节点/电机）

- 依据用户桌面《aruco码精准识别降落.docx》及四项修改要求，以当前源码为基线重构降落状态机。本次没有启动真机控制或识别节点，没有调用使能、解锁、切换模式或发布飞行设定点；用户明确要求不启动电机。
- 默认 `auto_takeoff: false`，节点启动只等待。手动定点起飞后明确调用 `/aruco_landing/enable` 才允许接管；人工切出 OFFBOARD 立即锁定取消，再拨回 OFFBOARD 不会自动恢复，必须重新使能。取消/析构不再擅自切换模式。
- 分离平移和转向：接近目标并制动后才转向，默认最大偏航速率 0.06 rad/s；对准连续稳定后才下降，偏差超限或丢码时按限加速度制动并保持。近地且最近对准的短暂丢码允许交给最终 AUTO.LAND。
- 可选自动起飞只在显式使能后动作；初始 8 s 无目标计时从起飞到高并稳定后开始，超时制动并请求原地 AUTO.LAND。AUTO.LAND 完成需结合飞控落地/上锁状态，不以 mode_sent 判定落地。
- YAML 集中配置速度/加速度、稳定门限、目标 ID 与实际码尺寸。修复视觉无关 ID 屏蔽/覆盖目标、丢码残留旧值、latched 旧目标等问题；图像无时间戳时明确使用检测接收时刻，尚未解决 RTSP 真实采集延迟或实物外参标定。
- 本地和香橙派 ARM 编译通过，20 项独立控制场景和速度/加速度检查通过；PC 独立 localhost ROS master 的模拟飞控/合成标记图像集成测试通过。没有将模拟数据发送到香橙派或真机飞控。
- 09:26 安装前后只读状态均为 `connected: true, armed: false, mode: AUTO.LOITER`。旧源码/可执行文件备份：`~/.ros/backups/aruco_v2_20261027_092646/original.tar.gz`。部署清单位于板端 `~/something1017/TECH_DEMO/ARUCO_V2_DEPLOYMENT.json`，本地副本及验证日志在 `flight_review/20261027/`。
- 使用说明：`TECH_DEMO/ARUCO_LANDING_V2.md`（本地与板端均有）。最终 AUTO.LAND 的下降速度仍由 PX4 参数负责，本次未修改任何飞控参数。原上位机保持原样。

## 2026-10-26 ArUco 飞行分析与独立遥测监视器

- 用户反馈 ArUco 降落时机身旋转、落点偏差较大；现有文本日志无法量化实际航向或落点误差。
- 找到最近 17:32:14 启动、17:32:36 请求 AUTO.LAND 的降落日志。当前二进制更新时间 17:40:21，历史行号/输出对应旧版，不将现有限速配置当作那次飞行的实际参数。日志 `Land successful!` 不是落地确认。
- 源码会根据图像码边角度主动发送旋转指令，下降没有水平误差门限，丢码后仍沿用旧目标。证据、待校验的相机标定/外参问题与修改方向见 `flight_review/20261026/ARUCO_REVIEW.md`。
- 新增并部署开机自启的独立 `flight-data-monitor.service`，浏览器访问 `http://192.168.1.122:8766/`。订阅真实飞行遥测，支持三类测试标签、轨迹/趋势、持续记录、CSV 导出和历史回看。板端数据目录 `~/.ros/flight_monitor/`，容量与时间戳限制见 `FLIGHT_MONITOR.md`。
- 本次仅启动遥测服务，未修改飞控程序，未运行三个控制 demo，未发送解锁/模式切换/飞行设定点。已验证真实本地位姿约 30 Hz、记录写盘及网页回看；ArUco 未运行时明确显示断流，不视为识别或飞行验证。

## 2026-10-26 ArUco 起降降速（已部署编译，未飞行验证）

- 用户反馈起降过快。本次仅修改 `TECH_DEMO/src/land_test`，没有运行新版控制节点，没有发解锁、模式切换或飞行设定点。
- 保留板端已有起飞高度 1.2 m。起飞由直接发送最终位置，改为根据位置误差生成有上限的速度指令：向上不超过 0.25 m/s，向下不超过 0.10 m/s；水平速度向量不超过 0.20 m/s。
- 进入目标位置的 5 cm 容差范围后，连续保持 2 秒再进入视觉引导下降。仍通过小速度修正位置；离开容差范围会重新计时。
- 视觉下降由原先 0.10–0.30 m/s 调为 0.05–0.10 m/s，取决于偏航误差；水平修正增益由 0.8 减为 0.4，并新增水平速度向量限幅；最大偏航速率由 0.314 减为 0.15 rad/s。
- 配置集中在 `land_test/config/takeoff_land.yaml`，含 `takeoff_speed`、`takeoff_hover_seconds`、`landing_vlz`、`max_horizontal_speed` 等参数。程序加载参数时拒绝非正值/非有限值，回退到默认值。
- 只读确认 PX4 上报版本为 1.16.0；`MPC_LAND_SPEED=0.7`、`MPC_LAND_CRWL=0.3`、`LNDMC_Z_VEL_MAX=0.25`。没有修改飞控参数。若程序因目标丢失切 `AUTO.LAND`，速度由 PX4 决定，不受本程序 0.10 m/s 限制。
- 本地语法检查、速度上限/不同回调频率下的理想速度跟踪模型/连续停稳等待测试通过；香橙派临时构建和原工作区 `Safina_land_test` 目标编译通过，板端独立逻辑测试通过。原工作区 `CATKIN_ENABLE_TESTING` 关闭，测试在临时构建目录中执行；这些检查不等同于真机飞行验证。
- 部署前曾发现用户启动的旧降落节点；等待该节点停止且飞控未解锁后才安装。原源码、配置和可执行文件已备份到板端 `~/.ros/backups/land_slow_20261026_173416/original_land_test.tar.gz`。
- 下次由用户启动第四条命令时才运行新版。此前识别有效性、遥控接管和失联处理等逻辑没有在本次降速中重构，仍需单独核验。

## 2026-10-26 最新进展：定点反馈与 ArUco 图像链恢复

- 用户确认：雷达辅助的遥控器定点飞行已能稳定悬停。下文 10 月 21–23 日关于“尚未飞行/定位未通过”的描述是历史状态，不作为本次当前结论。
- 本次 SSH 实查：Orbbec 和 ArUco 节点已停止，RTSP 图像话题不存在。USB 网卡 `enx00e09a5b46fc` 有物理连接但没有 IPv4 地址，云台 `192.168.144.108` 的流量错误地走 Wi-Fi 默认路由。
- 已创建并激活 NetworkManager 配置 `gimbal-rtsp`：USB 网卡使用 `192.168.144.100/24`，自动连接、不设置默认路由。实测云台 ping 成功；雷达 `.107` 仍走 `enP3p49s0`，Wi-Fi 默认路由未改变。
- 单独恢复 Orbbec，使用 `TECH_DEMO/devel/setup.bash` 与配套 SDK 2.10.6；彩色图 1280×720、深度图 848×480 均有数据，彩色图约 10 Hz。没有重启整个 `Safina_sensor.service`。
- 启动 `rtsp_test/rtsp_test.launch` 与 `aruco_test/Safina_aruco.launch` 后，RTSP 图像 1920×1080、约 30 Hz，ArUco 结果约 11 Hz；现场实际识别到 ID 231，位姿消息持续输出。此次验证了图像和识别链路，未验证测距精度或自动降落。
- 用 `QT_XCB_GL_INTEGRATION=none LIBGL_ALWAYS_SOFTWARE=1 QT_OPENGL=software rqt_image_view /aruco_imgresult` 打开 NoMachine 桌面图像窗口，新窗口日志未再出现先前的 GL 错误。
- 新增并部署 `TECH/tools/start_aruco_vision.sh`。在香橙派终端执行 `~/something1017/TECH/tools/start_aruco_vision.sh`，会复用已有发布者或启动 RTSP/ArUco，再打开软件渲染查看窗口。保持终端运行；退出只停止该脚本自己启动的进程。`--no-view` 可只检查/启动图像链路。
- 本次恢复使用的进程日志在 `~/.ros/log/camera_recovery_20261026/`；新脚本日志在 `~/.ros/log/aruco_vision/`。脚本已通过 Bash 语法检查和板端复用已有节点的实测。
- 本次没有发送解锁、模式切换、飞行设定点，也没有启动 `land_test`、`track_test` 或 `offboard_ctl`。连接初始和最终读到的状态都是 `armed: False`、`mode: OFFBOARD`；该模式是连接前已有的状态。

记录更新：2026-10-21

## 环境与飞控

- 香橙派 5 Ultra，Ubuntu 20.04.6 LTS，aarch64，ROS Noetic。
- 板端工程路径：`/home/orangepi/something1017`；已确认 `TECH` 环境入口为 `~/something1017/TECH/setup.bash`，`TECH/devel/setup.bash` 不存在。`TECH_DEMO` 的 Orbbec 节点也曾从 `~/something1017/TECH_DEMO/devel` 运行；TRACK 本轮未核验。
- 工程分为 `TECH`（Livox、Orbbec/USB 驱动、FAST-LIO、TO_MAVROS）、`TECH_DEMO`（控制/测试 demo）和 `TRACK`（跟踪程序）。目录存在不等于其中所有功能都已真机验证；尤其不要把 `offboard_ctl` 当普通监视程序运行。
- `Safina_sensor.service` 为 enabled/active，启动 ROS master、MAVROS、Orbbec、USB 相机和延迟启动的 Livox 驱动。不要因单个话题异常就反复重启整套服务。
- 当前所见 MAVROS 状态：`connected: True`、`armed: False`、模式 `POSCTL`。这只证明 MAVROS 已连接及 PX4 报告当前模式，不证明 PX4 正在融合雷达定位。

## 传感器与定位

- MID360 主机网卡 `enP3p49s0`：`192.168.1.50/24`；已安装配置雷达 IP：`192.168.1.107`。定向 ping 3/3 成功，邻居 MAC 为 `ec:72:f7:0d:a9:be`。
- 当时 Wi-Fi `wlan0` 地址为 `192.168.1.122/23`，默认网关 `192.168.1.1`。DHCP 地址可能改变，换手机热点后需重新查地址。
- Livox 驱动节点 `/livox_lidar_publisher2` 正常运行；`/livox/lidar` 类型 `livox_ros_driver2/CustomMsg`，多次实测约 10 Hz，样本含非零 XYZ 与反射率。雷达 `.107` 可从主机网口 ping 通。
- `/livox/imu` 曾实测约 200 Hz；网页最新截图显示约 147 Hz（此前也见约 176 Hz）。数据持续到达，但速率有变化，需在 FAST-LIO 和网页都运行时再次确认是否稳定。
- FAST-LIO `/laserMapping` 可启动，日志报告 `IMU Initial Done`，曾提示一帧 `No point, skip this scan!`；之后 `/Odometry` 稳定约 10 Hz。
- 静止时抽查 `/Odometry` 的三帧位置约为 x=0.019–0.026 m、y=0.004–0.005 m、z=0.024–0.029 m。样本时间短，不代表长时漂移或飞行精度。
- `/camera/color/image_raw` 与 `/camera/depth/image_raw` 曾实测约 10 Hz；网页看到彩色图 1280x720 `rgb8`、深度图 848x480 `16UC1`。服务日志曾记录 Gemini 335L 断开/重连，设备连接类型报告 USB2.1，后续需检查连接稳定性和实际画面。
- `/mavros/imu/data_raw` 网页显示约 49–50 Hz；`/mavros/state` 网页显示已连接、未解锁、POSCTL。
- `/usb_cam/image_raw` 本轮没有数据；日志曾有 `Error starting device`。USB 相机未通过，且由总服务管理，不要从网页重复启动另一个同名节点。

## 当前状态与下一步

- FAST-LIO 曾手动运行成功：`roslaunch fast_lio mapping_mid360.launch` 日志出现 `IMU Initial Done`，`/laserMapping` 出现，`/Odometry` 曾稳定约 10 Hz。它是前台进程，关闭启动终端后会停止。
- 较早截图中 FAST-LIO 与位姿桥都无数据；**更新：最新网页截图显示两者已在运行**。`/Odometry` 约 10 Hz，`/mavros/vision_pose/pose` 约 29.9 Hz。
- 最新截图的位置摘要异常：`/Odometry` 约 `(2563.785, 2200.318, -7416.230) m`，视觉位姿约 `(3187.198, 1817.118, -6724.703) m`。台架测试出现数千米位置且两路位置不一致，定位判为失败/未解释；不能因频率正常就认为位姿有效。
- 最新截图 MID360 IMU 约 60.3 Hz（此前测试约 200 Hz，网页曾显示 147/176 Hz），Orbbec 彩色图约 5.5 Hz、深度图约 9.9 Hz；速率波动明显，需查负载、时间同步和当前机载配置。
- `/mavros/vision_pose/pose` 有 29.9 Hz 仅代表桥在发布。桥接源码会在初始化后重发最近一次输入，而且没有输入超时保护；需要验证位置连续性及 FAST-LIO 输入仍在更新。
- 尚未记录 PX4 固件版本、EKF2 参数备份/外部视觉融合状态、遥控器接管实测结果；未验证定位长时漂移/运动坐标方向/尺度，未做拆桨 OFFBOARD 验证，也未进行任何飞行。
- 当前初测结论：MID360 点云、Orbbec 彩色/深度、飞控 IMU 和 MAVROS 连接有数据；USB 相机无数据。FAST-LIO/桥虽然有频率，但位姿数值异常且相互不一致，IMU/相机频率偏低且波动。定位链路没有通过验收，不能开始带桨定点飞行。

### 2026-10-21 最新截图后的判定

- 用户询问是否可直接尝试起飞。答案：**不可以，当前不能进行带桨起飞或雷达定点测试**。请先拆除螺旋桨並固定机体，保持未解锁，不启动任何会自动解锁/起飞的 demo。
- 当前截图：MID360 点云约 10.0 Hz、约 19,872 点；Livox IMU 约 60.3 Hz；FAST-LIO `/Odometry` 约 10.0 Hz，但位置约 `(2563.785, 2200.318, -7416.230) m`；视觉位姿约 29.9 Hz，约 `(3187.198, 1817.118, -6724.703) m`。两路位姿都远离静止台架时先前观察到的原点附近厘米级位置，且彼此不一致。频率正常不等于定位可信，当前定位验收失败。
- 外部视觉桥源话题默认是 `/Odometry`，发布 30 Hz；源码会重发最近输入，没有输入超时保护。因此不能用 `/mavros/vision_pose/pose` 的频率作为上游仍正常的证据。若桥正在运行，在拆桨状态下可在其启动终端按 Ctrl+C 停桥；暂不启动控制程序。
- 工作区 `TECH/src/FAST_LIO/config/mid360.yaml` 中 `time_sync_en: false`、时间偏移为 0、外参平移为 `[-0.011, -0.02329, 0.04412]`。之前板端 launch 打印过 `time_sync_en=True`，说明需核对板端实际加载文件/参数；不能仅凭本地源码判断板端设置。FAST-LIO README 也说明软件时间同步只应在确实无法外部同步时启用。
- 下一步只做地面只读诊断，不重启整个传感器服务、不改参数：检查 `/livox/lidar`、`/livox/imu`、`/Odometry` 的频率与消息时间戳；静止连续取 3 个里程计位置观察是否仍增长/跳变；读取当前 FAST-LIO 参数和最近日志。判断坐标是否稳定前，不启动或继续运行视觉桥。
- 用户随后检查发现：`rosparam get /common/time_sync_en` 返回 `true`，`/common/time_offset_lidar_to_imu` 为 `0.0`；`/livox/imu` 稳定约 200 Hz。但 `/Odometry` 取 3 帧无输出，节点列表只有 `/livox_lidar_publisher2`，没有 `/laserMapping` 或 `vision2mavros`。因此目前 FAST-LIO 未运行，先前截图是另一时刻/另一运行状态；尚未复现异常位置。
- 工作区 `mapping_mid360.launch` 会加载工作区 `mid360.yaml`，其值为 `time_sync_en: false`，而板端全局参数仍显示 `true`。此差异可能是旧 FAST-LIO 参数残留或板端加载了不同副本。尚未确认原因；先在固定、拆桨状态启动 FAST-LIO 并观察完整启动日志，不要仅凭全局 rosparam 改值或开始飞行。
- 后续用户确认其在机载电脑执行过 FAST-LIO launch；当前 `/laserMapping` 存在，`/Odometry` 实测约 9.97 Hz。由此解释此前无 `/Odometry` 是 FAST-LIO 当时未启动。当前仍未重新抽查位姿数值/静止漂移，也未确认 PX4 EKF 融合、遥控接管及失效保护；“有 10 Hz 输出”本身不构成起飞许可。

### 恢复地面定位检查的板端命令

先确认传感器总服务仍在工作：

```bash
source /opt/ros/noetic/setup.bash
source ~/something1017/TECH/setup.bash
systemctl is-active Safina_sensor.service
rosnode list | grep -E 'livox|mavros'
rostopic hz /livox/lidar
rostopic hz /livox/imu
```

在独立终端启动 FAST-LIO 并保持该终端打开；关闭终端会停止前台节点：

```bash
source /opt/ros/noetic/setup.bash
source ~/something1017/TECH/setup.bash
roslaunch fast_lio mapping_mid360.launch
```

另一终端确认 `/Odometry` 连续更新：

```bash
source /opt/ros/noetic/setup.bash
source ~/something1017/TECH/setup.bash
rosnode list | grep -E 'livox|laserMapping'
rostopic hz /Odometry
```

只有 `/Odometry` 连续稳定后，拆桨地面检查才可在第三终端启动位姿桥：

```bash
source /opt/ros/noetic/setup.bash
source ~/something1017/TECH/setup.bash
roslaunch to_mavros vision2mavros.launch
```

保持 FAST-LIO 与桥接终端运行；检查 `/mavros/vision_pose/pose`、`/mavros/local_position/pose` 和 QGC EKF 状态。当前还没有经核对的 PX4 固件版本/参数快照，禁止直接照抄旧 EKF 参数或切入带桨定点测试。

## 异常与排查记录

- Livox 驱动曾因 `new node registered with same name` 退出，随后恢复运行；避免从多个终端重复启动同名驱动。
- `/livox/lidar` 曾出现自定义消息无法加载，是因为终端没有 source 正确工作空间；source `~/something1017/TECH/setup.bash` 后消息类型可解析。
- 尝试 `sudo systemctl restart Safina_sensor.service` 时交互密码认证失败；之后 `sudo -n true` 返回 0，当前不需要为恢复 FAST-LIO 重启总服务。

## 网页监视器

- 已添加 `TECH/tools/sensor_web_monitor.py` 和 `run_sensor_web_monitor.sh`。机载端用 Python 标准库 HTTP 服务订阅 ROS，开发电脑浏览器访问 `http://192.168.1.122:8765`。
- 当前香橙派运行的网页截图仍是只显示状态卡片的旧版；工作区新版代码增加了 FAST-LIO、USB 相机、外部视觉桥启停按钮，但尚未确认已部署到板端。更新代码后需在板端停止旧网页进程、上传新版 `sensor_web_monitor.py`、重启 `run_sensor_web_monitor.sh` 并浏览器强制刷新。
- 网页服务地址曾为 `http://192.168.1.122:8765`。服务绑定所有网卡；浏览器只需能路由到香橙派 IP 和 TCP 8765，不要求必须同一 Wi-Fi SSID。手机热点是否允许设备互访取决于热点客户端隔离、防火墙和新分配地址；换热点后用板端 `ip -brief addr` 查 `wlan0`，再访问 `http://<新wlan0地址>:8765`。若热点禁止客户端互访，需改用 USB 网络共享/路由器或其他可互通网络。
- HTTP 页面没有登录认证，只应在可信的本地网络使用，不要转发/映射到公网。新版三个启停项只允许固定 ROS launch 命令，不包含解锁、模式切换或位置控制；外部视觉桥需要 `/Odometry` 近期有数据才允许启动。
- 使用说明见 `WEB_SENSOR_MONITOR.md`。网页订阅图像话题会增加机载 CPU/内存带宽负载；首轮飞行不建议让监视器订阅全部相机图像，需先在地面验证开着网页时 CPU、温度、IMU/点云频率稳定。

## 雷达定点飞行前置流程（尚未完成）

按以下顺序逐项验收，任何一步失败都停止，不进入带桨测试：

1. 拆桨并固定机体。确认 `Safina_sensor.service`、MAVROS、`/livox/lidar`（约 10 Hz）和 `/livox/imu`（目标约 200 Hz，实测需稳定）持续运行。
2. 在独立终端启动 `roslaunch fast_lio mapping_mid360.launch` 并保持终端开启。确认 `/Odometry` 连续更新，做至少数分钟静止漂移记录，再在安全拆桨条件下轻微平移/旋转，核对坐标方向、尺度、回到起点误差及是否丢跟踪。
3. 仅在 `/Odometry` 健康时启动 `roslaunch to_mavros vision2mavros.launch`。检查 `/mavros/vision_pose/pose` 和 `/mavros/local_position/pose`。桥接源码前 100 帧初始化并按 30 Hz 重发最近位姿；它没有输入超时保护，所以 FAST-LIO 停止时必须立即停止桥，不能仅凭 vision_pose 频率认为定位有效。
4. 在 QGroundControl 记录并备份 PX4 固件版本及全部参数。按该固件版本核验 EKF2 外部视觉融合、水平/高度/偏航来源、GPS/气压计/磁力计选择及 failsafe；不要复制未核实的旧参数组合。拆桨下移动机体，确认 QGC 估计器状态和 MAVROS local_position 跟随正确。
5. 先完成独立的拆桨模式/控制链测试，检查控制程序失联保护、遥控器退出方式、Failsafe、起降/降落方案。当前 `offboard_ctl` 默认会请求 OFFBOARD、解锁并飞到 1 m 后降落，禁止把它当作首次定点测试程序。
6. 只有定位融合、拆桨控制链、遥控器接管、飞手与场地检查全部通过后，才由有经验飞手制定短时低高度带桨测试。当前阶段尚未达到该门槛。

## 对《科研飞行平台使用说明书0804》的核对（2026-10-21）

- 手册“激光雷达定位”两条命令分别启动 FAST-LIO (`mapping_mid360.launch`) 和转换桥 (`vision2mavros.launch`)；其作用是 `/livox/{lidar,imu}` → `/Odometry` → `/mavros/vision_pose/pose`。它们不解锁、不切换飞行模式、不发起飞设定点，因此执行后不能推断遥控起飞/定点已经就绪。
- 手册另一个“运行 offboard_ctl”章节执行 `rosrun offboard_ctl offboard_ctl`。当前工作区对应源码会请求 `OFFBOARD`、请求解锁，并先向 `mavros/setpoint_position/local` 发 0 m 目标，再按 demo=1 自动飞至 z=1 m、悬停约 5 秒并请求 `AUTO.LAND`。这不是遥控器定点模式测试程序，也不是只读测试；在确认安全前不要运行。
- 手册中的 `slam` alias 来自旧环境安装笔记，旧笔记指向 `TECH/devel/setup.bash`；当前实测正确环境入口为 `~/something1017/TECH/setup.bash`。应以当前已验证路径为准。
- 当前已知 FAST-LIO 约 10 Hz，拆桨静止两分钟位置变化约 4.5 mm，桥曾输出接近原点的位置。这些只验证短时 ROS 定位数据链。尚未验证 QGC/PX4 EKF 外部视觉融合、`/mavros/local_position/pose` 跟随、PX4 本地位置有效状态、遥控器模式切换/接管与失效保护；当前结论仍是不批准直接带桨起飞。

## 上位机网页连接与更新（2026-10-22）

- 桌面图标 `~/桌面/雷达定点监视台.desktop` 当前只是打开固定网址 `http://192.168.1.122:8765`。NoMachine 能连接 `.122`，但截图浏览器连接失败说明网页 TCP 服务当时不可达；桌面图标不会自动启动香橙派上的网页服务。先在香橙派执行 `~/something1017/TECH/tools/run_sensor_web_monitor.sh`，保持终端运行后再刷新网页；长期使用可安装 `TECH/tools/sensor-web-monitor.service` systemd 单元。
- 工作区新版网页监视器加入消息内容健康提示（点云非零有效点、IMU 数值、位姿范围与四元数、图像尺寸/编码、话题频率参考值），并增加一个“启动雷达定位链”按钮，调用 `start_radar_position_stack.sh`。启动前网页二次确认拆桨、固定和未解锁；守护脚本也检查传感器服务、MAVROS连接/未解锁及Livox话题。此按钮只启动 FAST-LIO 与位姿桥，不解锁、不切模式、不起飞；健康提示也不代表定位精度或飞行安全验收。
- 本次只修改本地工作区网页代码与部署说明，尚未将新版复制到香橙派，因此当前桌面浏览器中看到的仍可能是旧版/连接失败。更新文件列表及部署步骤见 `WEB_SENSOR_MONITOR.md`。
- 用户随后在香橙派启用 `sensor-web-monitor.service`，但状态为 `activating (auto-restart)`、`ExecStart` 退出 `status=1/FAILURE`。日志已确定根因：`/opt/ros/noetic/etc/catkin/profile.d/1.ros_distro.sh: ROS_DISTRO: unbound variable`，由启动脚本在 source ROS 前启用 `set -u` 引起。工作区两个启动脚本现已移除 `nounset`，板端也需移除这两个脚本首行中的 `u`，停止重启循环后重启服务并确认 8765 监听。
- 修正后网页端口已可访问，但页面控制状态请求引发 `AttributeError: module 'rospy' has no attribute 'get_node_names'`，浏览器 JSON 解析失败并导致表格不渲染。新版工作区 `sensor_web_monitor.py` 已改用发布话题集合识别外部定位链，且控制接口异常返回 JSON；需将此文件部署到板端、重启服务并用 `/api/control` 验证。
- 用户在重新连接 NoMachine 后发现网页服务 `inactive`、IP 仍为 `192.168.1.122`、8765 无监听；手动 `systemctl start` 后服务显示 `active (running)` 且 `0.0.0.0:8765` 正在监听。NoMachine 断开不应停止 systemd 系统服务。工作区服务策略已加强为 `Restart=always`、3 秒重启，需部署新版 unit 并观察持续运行；当前状态应先刷新浏览器验证。

### 本地电脑 RViz 点云按钮（2026-10-22，工作区实现待部署）

- 网页工作区新增 `/cloud_registered` FAST-LIO 注册点云行（`sensor_msgs/PointCloud2`）及本地 RViz 启动/关闭按钮。配置文件为 `TECH/tools/radar_pointcloud.rviz`，固定坐标系 `camera_init`；它订阅 FAST-LIO 点云，而不是 Livox 原始自定义消息 `/livox/lidar`。
- 本地工作区新增 `TECH/tools/local_rviz_agent.py` 与 `run_local_rviz_agent.sh`。本地电脑启动助手后，网页按钮通过机载网页 API 下发固定的 start/stop 请求，助手在本地运行 RViz，不接受任意 shell 命令，也不向飞控发送控制指令。按钮可在 MID360 点云/IMU 有效时启动或复用板端 FAST-LIO，只有 MAVROS 状态新鲜且 PX4 未解锁时才会启动新的 FAST-LIO；等待 `/cloud_registered` 和 `/Odometry` 后再打开 RViz，不会启动 PX4 位姿桥。
- ROS1 跨机数据连接要求本地电脑能访问 ROS master TCP 11311、ROS 节点的动态 XML-RPC/TCPROS 端口，且板端公布的节点 IP 可从本地电脑路由。工作区传感器服务启动脚本已改为优先发布 `wlan0` IPv4 (`ROS_IP`) 并清除 `ROS_HOSTNAME`，避免只公布无法在本地解析的主机名。
- 本次变更尚未部署至香橙派，板端 `Safina_sensor.service` 尚未按新脚本重启；本地助手到 ROS master 的连接、点云渲染及网络负载尚未实测。部署步骤在 `WEB_SENSOR_MONITOR.md`。更换热点/IP 后必须用新的网页/ROS master 地址启动助手，并确认热点没有客户端隔离。
- RViz 持续接收点云会增加网络、机载处理和本地渲染负载；应先做地面静置验证。是否飞行时常开 RViz 尚未测试，不据此推断雷达定位或飞行安全已验收。
- `start_radar_position_stack.sh` 曾在 FAST-LIO/位姿桥启动成功后，因为 PX4 `pos_horiz_rel/abs_status_flag` 未有效而退出；其 `EXIT` 清理逻辑会停止本次由脚本启动的节点，所以表现为“脚本没有启动成功”，而手动分别执行两个 launch 又能恢复话题。脚本现已改为：EKF 水平位置无效时只打印警告并保持 ROS 数据链，运行中继续监视节点和 `/Odometry`；这仍不代表允许解锁或起飞。需把更新后的脚本复制到香橙派后再使用。
- 2026-10-23 新截图中网页所有传感器同时变为“无新数据”，但仍显示上次摘要；疑似网页 ROS subscriber/master 连接中断或传感器链整体停止，尚未凭截图定因。排查应同时查网页 service 状态、ROS master、FAST-LIO/vision2mavros 节点和实时话题频率。
