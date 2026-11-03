# TRACK 人物跟踪调试说明（2026-10-28 改版）

## 1. 参数文件位置

| 节点 | 参数文件 | 启动文件 |
|---|---|---|
| 跟踪控制 `Safina_track_test` | `~/something1017/TRACK/src/track_test/config/track.yaml` | `~/something1017/TRACK/src/track_test/launch/Safina_track.launch` |
| 识别 + ID 跟踪 `Safina_yoloUltra_test` | `~/something1017/TRACK/src/yoloUltra_test/config/yolo_track.yaml` | `~/something1017/TRACK/src/yoloUltra_test/launch/Safina_yoloUltra_test.launch` |

两个 launch 文件都用 `<rosparam>` 把 YAML 加载到节点私有命名空间，改完 YAML 后**重启对应节点**即可生效，不需要重新编译。

PC 副本路径把 `~/something1017/TRACK` 换成
`~/桌面/PX4_WorkSpace/keyan/something1017/TRACK` 即可。

## 2. 新的控制逻辑

跟踪节点**不会解锁，也不会主动切 OFFBOARD**。飞手动作为：

1. 在**定点模式**下手动起飞，飞到想要的位置和高度。
2. 把模式开关 SWG 拨到 **OFFBOARD** 档（飞控参数 `COM_FLTMODE4=7`）。
3. 节点检测到 OFFBOARD 后，立即记录当前位置和高度作为保持点。
4. 按 SWA 第 9 通道执行：

| SWA | 进入 OFFBOARD 时 | 飞行中 |
|---|---|---|
| 下档（≈1913） | 定点保持，不降落 | 如果离开过下档再回来 → 原地降落；否则继续保持 |
| 中档（≈1494） | 直接开始人物跟踪 | 继续跟踪主目标 |
| 上档（≈1075） | 保护性保持，绝不起飞 | 停止跟踪，原地保持 |

- **不在 OFFBOARD 时**：SWA 完全不执行。节点只以 20 Hz 发一个当前位置的保持设定点，让飞手能顺利切入 OFFBOARD；在定点/定高/自稳模式下这个设定点被 PX4 忽略，飞手完全控制。
- **地面保护**：低于 `min_tracking_height_m`（默认 0.3 m），或 `/mavros/extended_state` 报 `landed_state=ON_GROUND/LANDING` 时，即使 OFFBOARD + 中档也只保持，不开始跟踪；跟踪中如果变成地面/降落状态也会立即停止跟踪并保持。
- **上档保护**：不会飞回任何预设高度，也不会重新起飞。
- **原地降落**：位置设定点保持 X/Y，Z 以 `landing_descent_speed` 下降；低于 `landing_switch_height` 或超时后请求 `AUTO.LAND`。
- **目标丢失**：保持丢失瞬间的位置，等新目标出现后再继续。
- **主目标丢失超过 `track_loss_timeout_s`**：删除该 ID，等待新出现的人；默认只接受“新出现的人”（`reacquire_mode: new_track`），也可改成 `any_visible` 接受画面里已有的其他人。
- **控制频率**：20 Hz 定时器，与图像帧率解耦；图像断了也会持续发设定点。

## 3. 人物 ID 规则

- 只在 **PX4 OFFBOARD + SWA 中档** 时开始 ID 会话；会话开始后切到**上档保护不会重置 ID**，回到中档继续跟踪原来的 ID。切到下档或离开 OFFBOARD 才结束会话，下次中档重新从 ID 1 开始。
- **地面台架模式（不飞看 ID）**：给 yolo 节点加启动参数 `require_offboard:=false`，此时 **POSCTL + SWA 中档** 也会分配 ID：
  ```bash
  track
  roslaunch yoloUltra_test Safina_yoloUltra_test.launch require_offboard:=false
  ```
  这个模式只让 yolo 节点分配 ID，**不启动 `track_test`、不切 OFFBOARD、不解锁、不发布任何设定点**。飞行时不要带这个参数（默认 `require_offboard=true`）。
  台架模式下真正切到 **OFFBOARD** 时，yolo 会自动结束当前台架 ID 会话并按正常的 `OFFBOARD + SWA 中档` 流程从 ID 1 重新开始，所以**不影响真实飞行时的目标选择流程**。

### 主目标稳定性（防止路人抢目标）

- 主目标有单独的关联门限：**位置预测 + 中心距离 + IoU + 框高一致性**。路人从旁边走过、距离/大小明显不同时，不会把主目标“抢走”。
- 丢失容忍 `track_loss_timeout_s` 默认 **3.0 s**：短暂遮挡期间主目标保持、跟踪节点原地保持，不切换。
- 主目标丢失后，默认只接受在**最后位置附近**（`reacquire_radius_ratio`）且**框高相似**的新人；远处的路人不会被接替。
- **兜底重获**：主目标丢失超过 `track_loss_timeout_s`（3 s）后，再等 `reacquire_fallback_s`（1.5 s）仍没有“原位置/原大小”的人，就选择画面里**最接近原位置/大小**的人，避免“明明有人在却一直不跟”。设为 0 可关闭兜底。
- 想让主目标丢失后接受画面里任何可见的人，设 `reacquire_mode: any_visible`。
- 仍然没有 ReID；两人在**同一距离、同一位置**交叉时仍可能模糊，这是无外观特征的固有限制。
- 第一个识别到的人 = **ID 1**，作为主跟踪目标；后来的人依次 ID 2、3……
- 主目标（ID 1）持续可见时，即使有 ID 2 进入，也**始终跟踪 ID 1**。
- 主目标丢失：先保持位置；超过 `track_loss_timeout_s` 后等待新的人出现，再把新的人作为主目标。
- 画面中会显示每个人的 `ID:n`；主目标用黄色粗框，其他人用橙色细框。
- **限制**：没有外观 ReID。两个人交叉/严重遮挡时仍可能交换 ID；这是当前轻量跟踪器的固有限制。

## 4. 深度融合测距

yolo 节点现在会订阅 `/camera/depth/image_raw`（16UC1，848×480，约 10 Hz，单位 mm），
用 `/camera/depth_to_color` 的外参（已写入 YAML）把人框躯干区域映射到深度图，取中值。

- 深度有效：`distance_source=2`（深度）
- 深度无效：`distance_source=1`（框高兜底，`d = fy × person_height_m / 框高`）
- 都无效：`distance_source=0`
- `/Safina_track` 现在是 7 个元素：
  `[state, x_offset, y_offset, id, distance_m, distance_source, distance_valid]`
- `track_test` 用 `target_distance_m` 做米制前向控制；距离无效时自动回退到原来的像素法。
- 上位机显示“距离 / 距离来源”，CSV 记录 `distance_m / distance_source / distance_valid`，
  分析脚本会额外生成 `analysis_distance.png`。
- 人身高只在框高兜底时使用，默认 **1.7 m**，可在 `distance/person_height_m` 修改。
- 算力：深度图只在本机订阅，每帧约 `grid×grid`（默认 7×7=49）个采样点；
  不把深度图传给上位机，开销很小，主要耗时仍是 YOLO 推理。

> 深度图没有对齐到彩色图（`depth_registration=false`），所以使用外参映射。
> 外参在 `yolo_track.yaml` 的 `depth/extrinsic_rotation/translation`；换相机模块后需要重新读取。

## 5. 常用调参

### track.yaml

| 参数 | 默认 | 作用 |
|---|---|---|
| `control_rate_hz` | 20 | 控制/设定点发布频率 |
| `require_offboard` | true | 是否只在 OFFBOARD 执行 SWA；真机保持 true |
| `target_timeout_s` | 0.5 | 多久没收到 `/Safina_track` 视为丢目标 |
| `target_loss_grace_s` | 0.3 | 丢目标先减速的宽限时间 |
| `tracking_vlxy` | 0.8 | 前向速度比例 |
| `max_forward_speed` | 0.3 | 前向最大速度 m/s |
| `max_backward_speed` | 0.15 | 后退最大速度 m/s |
| `tracking_ryaw` | 1.5 | 偏航速率比例 |
| `max_yaw_rate` | 1.2 | 最大偏航速率 rad/s |
| `min_tracking_height_m` | 0.3 | 低于此高度不开始跟踪，防止地面拖行 |
| `use_landed_state` | true | 同时用 `/mavros/extended_state` 的 `landed_state` 做地面保护 |
| `landed_state_timeout_s` | 1.0 | extended_state 数据超时 |
| `altitude_kp/kd` | 0.8/0.2 | 切入 OFFBOARD 时的高度保持 |
| `altitude_offset_m` | 0 | 跟踪高度相对切入高度的偏移 |
| `use_metric_distance` | true | 使用 yolo 发布的融合距离做前向控制 |
| `target_distance_m` | 3.0 | 想保持的米制距离 |
| `distance_kp` | 0.5 | 距离误差到速度的增益 |
| `distance_deadband_m` | 0.2 | 距离死区 |
| `landing_descent_speed` | 0.2 | 原地降落速度 m/s |
| `landing_switch_height` | 0.1 | 低于此高度请求 AUTO.LAND |
| `swa/up_max_pwm` | 1211 | ≤该值判为上档 |
| `swa/mid_max_pwm` | 1757 | ≤该值判为中档，否则下档 |

### yolo_track.yaml

| 参数 | 默认 | 作用 |
|---|---|---|
| `detection/object_threshold` | 0.2 | NMS 前候选框置信度 |
| `detection/nms_threshold` | 0.25 | NMS IoU |
| `detection/draw_confidence` | 0.5 | 画原始框的最低置信度 |
| `tracker/new_track_confidence` | 0.5 | 新的人分配 ID 的置信度门槛 |
| `tracker/match_confidence` | 0.2 | 已有 ID 保持的门槛 |
| `tracker/assoc_iou_threshold` | 0.25 | 帧间 IoU 关联阈值 |
| `tracker/assoc_center_ratio` | 0.10 | 帧间中心距离关联阈值（相对图像对角线） |
| `tracker/assoc_size_tol` | 0.60 | 帧间框高比例容差 |
| `tracker/track_loss_timeout_s` | 3.0 | ID 消失多久后删除（短暂遮挡容忍） |
| `tracker/primary_iou_min` | 0.15 | 主目标允许的最小 IoU |
| `tracker/primary_center_ratio` | 0.10 | 主目标预测中心允许距离（相对对角线） |
| `tracker/primary_size_tol` | 0.35 | 主目标框高一致性容差 |
| `tracker/primary_predict_s` | 0.20 | 主目标速度预测时间 |
| `tracker/max_predict_speed_ratio` | 0.50 | 预测速度上限（对角线/秒） |
| `tracker/reacquire_radius_ratio` | 0.25 | 丢失后重获的允许位置半径（相对对角线） |
| `tracker/reacquire_size_min_ratio` | 0.50 | 重获的框高下限比例 |
| `tracker/reacquire_size_max_ratio` | 2.00 | 重获的框高上限比例 |
| `tracker/reacquire_fallback_s` | 1.50 | 原位置/大小都没有时，多久后接受最接近的可见者；0=关闭 |
| `tracker/reacquire_mode` | new_track | 主目标丢失后的重获策略（new_track=只接受原位置附近的新出现者；any_visible=接受任何可见者） |
| `offset/standoff_deadband_px` | 70 | 径向像素死区，前向速度为 0 |
| `offset/standoff_full_scale_px` | 150 | 径向像素满量程，前向系数为 1.0 |
| `depth/enabled` | true | 深度融合开关 |
| `depth/grid` | 7 | 躯干区域采样网格（7×7） |
| `depth/min_valid_ratio` | 0.30 | 深度有效点比例阈值 |
| `distance/person_height_m` | 1.70 | 框高兜底用的人身高 |
| `distance/filter_frames` | 5 | 距离中值平滑帧数 |

两个 YAML 里的 `swa/up_max_pwm`、`swa/mid_max_pwm` **必须保持一致**。

## 5. 运行顺序

```bash
# 终端1 定位
slam && roslaunch fast_lio mapping_mid360.launch
# 终端2 定位送飞控
slam && roslaunch to_mavros vision2mavros.launch
# 终端3 识别（可先用 rqt_image_view 看 /yoloUltra_imgresult）
track && roslaunch yoloUltra_test Safina_yoloUltra_test.launch
# 终端4 跟踪控制
track && roslaunch track_test Safina_track.launch
```

雷达、相机、mavros 由 `Safina_sensor.service` 开机自启，不要重复启动。

## 6. 安全行为

- 节点从不解锁、从不请求 OFFBOARD。
- 节点退出时：只有**已解锁且处于 OFFBOARD** 才自动 `AUTO.LAND`；在定点模式下退出不会抢控制权。
- `/Safina_track` 断流、RC 断流、位姿断流时，跟踪节点会保持/刹车，不会继续追。
- PX4 侧：`COM_OF_LOSS_T=1.0s`、`COM_OBL_RC_ACT=0`（Position），OFFBOARD 失联 1 秒自动回定点。
- `COM_RC_OVERRIDE=1` 只对 AUTO 模式生效，**OFFBOARD 下摇杆不会接管**；要接管请拨 SWG。

## 7. 已验证 / 未验证

- 已验证：两个包在香橙派上编译通过；track 节点在 PC 上用独立假 ROS master（端口 11312）跑完 9 个状态机场景全部通过；多目标 ID 跟踪器用合成框做了单元测试。
- 未验证：真机飞行、真实相机下的 ID 稳定性、实际保持距离。首次飞行请低高度、拆桨先看状态，再逐步放开。
