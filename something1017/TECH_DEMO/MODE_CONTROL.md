# 以 PX4 实际飞行模式触发任务

`offboard_ctl` 和 `land_test` 的默认触发均读取 `/mavros/state.mode`。
遥控器通道、开关方向、飞行模式档位只在 PX4/QGroundControl 中配置。
香橙派不再解释 PWM，不维护第二套 `rc_channel`、`rc_offboard_slot` 或遥控校准参数，
也不会为了启动任务主动申请 OFFBOARD。PX4 拒绝切换时，任务不会开始。
此触发依据实际模式变化，因此地面站切换到 OFFBOARD 同样会被识别。

等待期间，在数据新鲜、起飞/接管条件满足且未发现其他控制发布者时，程序以 30 Hz
发送保持设定点，为人工切入 OFFBOARD 准备数据流。这些设定点在 POSCTL 中不接管控制；
等待期间不申请解锁、不申请切换模式。启动时已在 OFFBOARD 不发送等待设定点，也不触发任务。
默认在 POSCTL 连续准备至少 1.2 秒后打印 `RC gate READY in POSCTL`，
进入 OFFBOARD 后再确认 0.2 秒。READY 只表示本程序的门控就绪，不替代 PX4 的解锁检查。

## offboard_ctl：地面起降演示

1. 飞机在地面、未解锁，遥控器选择 PX4 的定点模式 `POSCTL`。
2. 使用现有 `demo` 环境后，执行 `rosrun offboard_ctl offboard_ctl`。
3. 等待 `RC gate READY in POSCTL`，再人工切入 OFFBOARD。
4. 程序只在实际 OFFBOARD 中申请解锁，保持起点 X/Y 和航向，目标高度为起点 Z 加 1 米。
5. 到位且速度稳定后连续悬停 2 秒，请求 `AUTO.LAND`。PX4 确认 AUTO.LAND 后，程序停止设定点输出，
   等待落地并上锁；程序不会主动在空中上锁。

切出 OFFBOARD（程序预期的 AUTO.LAND 除外）、数据失效或任务超时会取消任务。
取消后不会重夺 OFFBOARD、重新解锁或自行再飞。再次执行需要先落地上锁、回 POSCTL 等 READY，再切入。
未获得解锁、飞行器未到目标或降落模式被拒绝时，均有超时退出，不无限重试。

## land_test：手动起飞后接管精准降落

`config/takeoff_land.yaml` 默认保留 `auto_takeoff: false`：

1. 遥控定点手动起飞。
2. 启动现有 ArUco 检测和 `roslaunch land_test Safina_land.launch`。
3. 在空中保持 POSCTL，等 READY，再按 PX4 配置切入 OFFBOARD。
4. 程序接管找码、对准和降落。切回定点立即取消本次任务。
5. 再次接管需要重新在 POSCTL 等 READY，然后切入；一直停留 OFFBOARD 不会重试。

若明确将 `auto_takeoff` 改为 `true`，流程才变为地面等待、切入后申请解锁并自动升高，
高度仍由 `takeOff_altitude` 控制。原有识别、运动限制和精准降落参数保持原值。
旧的 `start_trigger: rc_offboard` 仅作为 `px4_mode` 的兼容别名；旧通道/档位参数会告警并忽略。
显式的 `start_trigger: service` 仍保留原服务使能流程。

默认 CSV 保留原列顺序，原 RC 四列改为 `mode_state_fresh,legacy_rc_slot_unused,px4_offboard_selected,mode_edge_ready`，
其中废弃档位列恒为 0。等待预发送计入 `published` 列，任务仍为 IDLE/CANCELLED，未发生接管。

## 编译与验证

仓库 `build/`、`devel/` 中的旧二进制是历史备份，本次不更新。必须从更新后的源码编译再运行：

```bash
source /opt/ros/noetic/setup.bash
# 再 source 板端现有的传感器/基础工作区环境
cd <实际的 TECH_DEMO 或 mission_demo 工作区>
catkin_make --only-pkg-with-deps offboard_ctl land_test -DCMAKE_BUILD_TYPE=Release
source devel/setup.bash
```

两个控制程序不能同时运行，也不能与其他设定点控制器并行运行。
GitHub Actions 在独立 ROS Noetic 工作区编译，运行 C++ 回归测试和仅使用本机新端口 ROS master 的模拟飞控测试；
这些验证不连接香橙派或真实飞控，不代表真机飞行验证。

PX4 对 OFFBOARD 预发送与模式切换的要求见
[PX4 OFFBOARD 文档](https://docs.px4.io/main/en/flight_modes/offboard)。
