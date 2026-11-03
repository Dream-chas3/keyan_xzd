# 香橙派 + PX4 无人机工程备份

本仓库备份香橙派 `/home/orangepi/something1017`，快照日期为 **2026-11-03**。全部工程文件来自板端，未混入电脑端工程副本。

## 主要功能

| 路径 | 内容 |
| --- | --- |
| `something1017/TECH/` | 传感器驱动、FAST-LIO 定位、位姿转发及辅助工具；板端为安装后的运行目录，没有 `src` |
| `something1017/TECH_DEMO/src/aruco_test/` | ArUco 码识别 |
| `something1017/TECH_DEMO/src/land_test/` | ArUco 引导精准降落 |
| `something1017/TECH_DEMO/src/waypoint_mission/` | YAML 航点规划、悬停与拍照 |
| `something1017/TECH_DEMO/src/rtsp_test/` | 云台相机 RTSP 图像采集 |
| `something1017/TRACK/` | YOLO 人物检测、目标跟踪及跟随控制 |

工程内的相机 SDK、模型权重、运行库及现有 `build`、`devel` 文件一并保留。`backup_manifest.json` 记录原始文件校验值、权限和符号链接，便于核对备份。

## 下载

部分大文件使用 **Git LFS**。请先安装 Git LFS，再下载完整工程：

```bash
git lfs install
git clone https://github.com/Dream-chas3/keyan_xzd.git
cd keyan_xzd
git lfs pull
```

## 恢复说明

- 板端环境：香橙派 5 Ultra / ARM64、Ubuntu 20.04、ROS Noetic。
- 工程包含绝对路径，恢复时应放到 `/home/orangepi/something1017`；已有工程请先另行保存。
- `TECH` 是运行目录，需完整保留其中的 `lib`、`share`、`include` 和环境脚本。
- 本仓库是工程备份，不是整机镜像；系统安装的 ROS、MPP/FFmpeg 等依赖、网络配置、systemd 服务、`~/.bashrc`、`~/.ros` 数据和飞控中的 PX4 固件/参数不在本次备份范围内。

常用参数文件：

- 精准降落：`something1017/TECH_DEMO/src/land_test/config/takeoff_land.yaml`
- 人物跟随：`something1017/TRACK/src/track_test/config/track.yaml`
- 人物检测：`something1017/TRACK/src/yoloUltra_test/config/yolo_track.yaml`
- 航点任务：`something1017/TECH_DEMO/src/waypoint_mission/missions/abc_photo.yaml`
