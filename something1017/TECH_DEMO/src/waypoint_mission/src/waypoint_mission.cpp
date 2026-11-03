// Data-driven OFFBOARD waypoint mission.
//
// Behaviour summary:
//  - Starts in IDLE. It does not arm, switch modes, or publish flight setpoints.
//  - Waits for the transmitter flight-mode switch to leave the OFFBOARD slot in
//    POSCTL, then to enter the OFFBOARD slot again (fresh edge). This is the same
//    gate used by land_test (channel 5, slot 4 on this aircraft).
//  - On the edge: pre-streams current-pose setpoints for 1.2 s, requests
//    OFFBOARD, arms, takes off, then visits the waypoints from the YAML file.
//  - Photo capture and gimbal commands run in worker threads. The 30 Hz setpoint
//    loop keeps publishing while they run, so PX4 never hits COM_OF_LOSS_T.
//  - If the transmitter leaves the OFFBOARD slot, RC is lost, mode leaves
//    OFFBOARD, or localisation stops updating, the mission stops publishing and
//    never re-requests a mode. The pilot keeps control.
//
// Coordinates: all x/y/z/yaw in the YAML are MAVROS local ENU (the same frame
// as /mavros/local_position/pose). mavros setpoint_raw converts ENU->NED.

#include <ros/ros.h>
#include <ros/package.h>
#include <geometry_msgs/PoseStamped.h>
#include <nav_msgs/Odometry.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/RCIn.h>
#include <mavros_msgs/PositionTarget.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <std_msgs/String.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "gcu_control.h"

namespace waypoint_mission {

// ---------------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------------
static constexpr double kPi = 3.14159265358979323846;

static double deg2rad(double d) { return d * kPi / 180.0; }
static double wrap_pi(double a) {
    while (a > kPi) a -= 2.0 * kPi;
    while (a < -kPi) a += 2.0 * kPi;
    return a;
}
static bool finite_xyz(double x, double y, double z) {
    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}
static std::string stamp_now() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&t));
    return std::string(buf);
}
static std::string sanitize_name(const std::string& in) {
    std::string out;
    for (char c : in) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')
            out.push_back(c);
        else
            out.push_back('_');
    }
    if (out.empty()) out = "wp";
    return out;
}
static bool ensure_dir(const std::string& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return std::filesystem::is_directory(path, ec);
}

// ---------------------------------------------------------------------------
// RC start gate (same PX4 six-position selector maths as land_test)
// ---------------------------------------------------------------------------
struct RcConfig {
    int channel = 5;
    int offboard_slot = 4;
    double minimum = 1000.0;
    double trim = 1500.0;
    double maximum = 2000.0;
    double deadzone = 10.0;
    bool reversed = false;
    double timeout = 0.5;
    double off_hold_seconds = 0.3;
    double on_hold_seconds = 0.2;
    std::string ready_mode = "POSCTL";

    bool valid() const {
        return channel >= 1 && channel <= 18 && offboard_slot >= 1 && offboard_slot <= 6 &&
               std::isfinite(minimum) && std::isfinite(trim) && std::isfinite(maximum) &&
               std::isfinite(deadzone) && deadzone >= 0 &&
               minimum < trim - deadzone && trim + deadzone < maximum &&
               std::isfinite(timeout) && timeout > 0 &&
               std::isfinite(off_hold_seconds) && off_hold_seconds > 0 &&
               std::isfinite(on_hold_seconds) && on_hold_seconds > 0;
    }
    int slot(unsigned pwm) const {
        if (!valid() || pwm < 500 || pwm > 2500) return 0;
        float normalized = 0;
        if (pwm < trim - deadzone)
            normalized = std::max(-1.0f, float((double(pwm) - (trim - deadzone)) / (trim - deadzone - minimum)));
        else if (pwm > trim + deadzone)
            normalized = std::min(1.0f, float((double(pwm) - (trim + deadzone)) / (maximum - trim - deadzone)));
        if (reversed) normalized = -normalized;
        const float half = 1.0f / 6.0f;
        const float index = ((normalized + 1.05f) * 6.0f + half) / 2.1f + half + 1.0f;
        return std::max(1, std::min(6, int(index)));
    }
};

enum class RcEvent { NONE, START, CANCEL };

class RcStartGate {
public:
    explicit RcStartGate(const RcConfig& config = RcConfig()) : config(config) {}
    RcConfig config;
    bool ready() const { return ready_; }
    RcEvent update(double now, bool valid, bool offboard_selected, bool active) {
        if (!valid || !std::isfinite(now) || now < last_time_) {
            ready_ = false;
            off_since_ = on_since_ = -1;
            last_time_ = now;
            return active ? RcEvent::CANCEL : RcEvent::NONE;
        }
        last_time_ = now;
        if (!offboard_selected) {
            on_since_ = -1;
            if (off_since_ < 0) off_since_ = now;
            if (active) {
                ready_ = false;
            } else if (now - off_since_ >= config.off_hold_seconds) {
                ready_ = true;
            }
            return active ? RcEvent::CANCEL : RcEvent::NONE;
        }
        off_since_ = -1;
        if (active) {
            ready_ = false;
            on_since_ = -1;
            return RcEvent::NONE;
        }
        if (!ready_) {
            on_since_ = -1;
            return RcEvent::NONE;
        }
        if (on_since_ < 0) on_since_ = now;
        if (now - on_since_ >= config.on_hold_seconds) {
            ready_ = false;
            on_since_ = -1;
            return RcEvent::START;
        }
        return RcEvent::NONE;
    }
private:
    bool ready_ = false;
    double off_since_ = -1;
    double on_since_ = -1;
    double last_time_ = -1;
};

// ---------------------------------------------------------------------------
// mission file
// ---------------------------------------------------------------------------
struct Waypoint {
    std::string name;
    // File coordinates as authored in YAML (display frame), and the converted
    // local ENU coordinates used by the controller.
    double file_x = 0, file_y = 0;
    double x = 0, y = 0, z = 0;
    enum class YawMode { AUTO, HOLD, ABSOLUTE } yaw_mode = YawMode::AUTO;
    double yaw_abs = 0;      // radians, ENU
    double hold = 0;         // seconds
    bool photo = false;
    bool has_gimbal = false;
    int gimbal_pitch = 0;
    int gimbal_yaw = 0;
    double speed = 0;        // <=0 means global
    double tolerance = 0;    // <=0 means global
    double timeout = 0;      // <=0 means global
};

struct MissionConfig {
    std::string name = "mission";
    // "local": file x = SLAM start nose-forward, y = left (legacy)
    // "x_right_y_forward" / "vision_world": fixed visual/SLAM frame:
    //   file +x = visual right, +y = visual forward. This is NOT body-relative;
    //   changing the body yaw does not rotate these axes.
    std::string frame = "local";
    bool x_right_y_forward = false;
    double takeoff_z = 1.0;          // absolute local ENU z
    double max_speed = 0.6;          // m/s horizontal
    double max_z_speed = 0.3;        // m/s vertical
    double max_yaw_rate = deg2rad(30);
    double arrival_tolerance = 0.15; // m horizontal
    double z_tolerance = 0.10;       // m
    double yaw_tolerance = 0.15;     // rad
    double stable_seconds = 0.6;
    double leg_timeout = 30.0;
    double mission_timeout = 300.0;
    double pose_timeout = 0.5;
    double state_timeout = 3.0;   // /mavros/state is ~1 Hz on this FCU; do not use 1.0 s
    double photo_timeout = 8.0;
    double gimbal_settle = 1.0;
    double gimbal_timeout = 8.0;
    double align_timeout = 15.0;
    double settle_timeout = 10.0;
    double prestream_seconds = 1.2;
    double geofence_radius = 20.0;
    double min_z = 0.2;
    double max_z = 5.0;
    double kp_xy = 0.8;
    double kp_z = 0.8;
    double kp_yaw = 1.0;
    bool return_to_start = true;
    bool auto_land = true;
    bool photo_required = false;
    bool gimbal_required = false;
    bool dry_run = false;
    bool bench_mode = false;
    std::string photo_dir;
    std::string log_dir;
    RcConfig rc;
    std::vector<Waypoint> waypoints;
};

template <typename T>
static void read_opt(const YAML::Node& n, const char* key, T& out) {
    if (n[key]) out = n[key].as<T>();
}

static bool load_mission(const std::string& path, MissionConfig& c, std::string& err) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        YAML::Node m = root["mission"] ? root["mission"] : root;
        if (!m || !m.IsMap()) {
            err = "mission root is not a map";
            return false;
        }
        read_opt(m, "name", c.name);
        std::string frame = "local";
        read_opt(m, "frame", frame);
        if (frame == "local" || frame.empty()) {
            c.x_right_y_forward = false;
            c.frame = "local";
        } else if (frame == "x_right_y_forward" || frame == "right_forward" ||
                   frame == "vision_world" || frame == "world") {
            c.x_right_y_forward = true;
            c.frame = (frame == "vision_world" || frame == "world")
                          ? "vision_world" : "x_right_y_forward";
        } else {
            err = "frame must be 'local', 'x_right_y_forward' or 'vision_world'";
            return false;
        }
        read_opt(m, "takeoff_z", c.takeoff_z);
        read_opt(m, "max_speed", c.max_speed);
        read_opt(m, "max_z_speed", c.max_z_speed);
        double yaw_rate_deg = c.max_yaw_rate * 180.0 / kPi;
        read_opt(m, "max_yaw_rate_deg", yaw_rate_deg);
        c.max_yaw_rate = deg2rad(yaw_rate_deg);
        read_opt(m, "arrival_tolerance", c.arrival_tolerance);
        read_opt(m, "z_tolerance", c.z_tolerance);
        read_opt(m, "yaw_tolerance", c.yaw_tolerance);
        read_opt(m, "stable_seconds", c.stable_seconds);
        read_opt(m, "leg_timeout", c.leg_timeout);
        read_opt(m, "mission_timeout", c.mission_timeout);
        read_opt(m, "pose_timeout", c.pose_timeout);
        read_opt(m, "state_timeout", c.state_timeout);
        read_opt(m, "photo_timeout", c.photo_timeout);
        read_opt(m, "gimbal_settle", c.gimbal_settle);
        read_opt(m, "gimbal_timeout", c.gimbal_timeout);
        read_opt(m, "align_timeout", c.align_timeout);
        read_opt(m, "settle_timeout", c.settle_timeout);
        read_opt(m, "prestream_seconds", c.prestream_seconds);
        read_opt(m, "geofence_radius", c.geofence_radius);
        read_opt(m, "min_z", c.min_z);
        read_opt(m, "max_z", c.max_z);
        read_opt(m, "kp_xy", c.kp_xy);
        read_opt(m, "kp_z", c.kp_z);
        read_opt(m, "kp_yaw", c.kp_yaw);
        read_opt(m, "return_to_start", c.return_to_start);
        read_opt(m, "auto_land", c.auto_land);
        read_opt(m, "photo_required", c.photo_required);
        read_opt(m, "gimbal_required", c.gimbal_required);

        if (m["rc"]) {
            YAML::Node r = m["rc"];
            read_opt(r, "channel", c.rc.channel);
            read_opt(r, "offboard_slot", c.rc.offboard_slot);
            read_opt(r, "min", c.rc.minimum);
            read_opt(r, "trim", c.rc.trim);
            read_opt(r, "max", c.rc.maximum);
            read_opt(r, "deadzone", c.rc.deadzone);
            read_opt(r, "reversed", c.rc.reversed);
            read_opt(r, "timeout", c.rc.timeout);
            read_opt(r, "off_hold_seconds", c.rc.off_hold_seconds);
            read_opt(r, "on_hold_seconds", c.rc.on_hold_seconds);
            read_opt(r, "ready_mode", c.rc.ready_mode);
        }

        YAML::Node wps = m["waypoints"];
        if (!wps || !wps.IsSequence() || wps.size() == 0) {
            err = "mission.waypoints must be a non-empty sequence";
            return false;
        }
        for (std::size_t i = 0; i < wps.size(); ++i) {
            YAML::Node w = wps[i];
            Waypoint p;
            p.name = "wp" + std::to_string(i);
            read_opt(w, "name", p.name);
            if (!w["x"] || !w["y"] || !w["z"]) {
                err = "waypoint " + std::to_string(i) + " needs x, y, z";
                return false;
            }
            p.file_x = w["x"].as<double>();
            p.file_y = w["y"].as<double>();
            p.z = w["z"].as<double>();
            if (c.x_right_y_forward) {
                // Fixed visual frame: file +x = visual right, +y = visual forward.
                // Controller-local +x = forward, +y = left; the transform is
                // constant and does not use the current body yaw.
                p.x = p.file_y;
                p.y = -p.file_x;
            } else {
                p.x = p.file_x;
                p.y = p.file_y;
            }
            read_opt(w, "hold", p.hold);
            read_opt(w, "photo", p.photo);
            read_opt(w, "speed", p.speed);
            read_opt(w, "tolerance", p.tolerance);
            read_opt(w, "timeout", p.timeout);

            if (w["gimbal_pitch"] || w["gimbal_yaw"]) {
                p.has_gimbal = true;
                read_opt(w, "gimbal_pitch", p.gimbal_pitch);
                read_opt(w, "gimbal_yaw", p.gimbal_yaw);
            }

            std::string yaw = "auto";
            if (w["yaw"]) {
                if (!w["yaw"].IsScalar()) {
                    err = "waypoint " + std::to_string(i) + " yaw must be auto/hold/degrees";
                    return false;
                }
                yaw = w["yaw"].as<std::string>();
            }
            if (yaw == "auto") {
                p.yaw_mode = Waypoint::YawMode::AUTO;
            } else if (yaw == "hold") {
                p.yaw_mode = Waypoint::YawMode::HOLD;
            } else {
                try {
                    p.yaw_mode = Waypoint::YawMode::ABSOLUTE;
                    p.yaw_abs = deg2rad(std::stod(yaw));
                } catch (...) {
                    err = "waypoint " + std::to_string(i) + " invalid yaw: " + yaw;
                    return false;
                }
            }
            c.waypoints.push_back(p);
        }

        // validation
        if (!c.rc.valid()) { err = "invalid rc configuration"; return false; }
        if (c.name.empty()) { err = "mission name is empty"; return false; }
        if (!(c.max_speed > 0.05 && c.max_speed <= 3.0)) { err = "max_speed must be in (0.05, 3.0]"; return false; }
        if (!(c.max_z_speed > 0.05 && c.max_z_speed <= 2.0)) { err = "max_z_speed must be in (0.05, 2.0]"; return false; }
        if (!(c.max_yaw_rate > 0.01 && c.max_yaw_rate <= 1.5)) { err = "max_yaw_rate_deg out of range"; return false; }
        if (!(c.arrival_tolerance > 0.02 && c.arrival_tolerance <= 1.0)) { err = "arrival_tolerance out of range"; return false; }
        if (!(c.z_tolerance > 0.02 && c.z_tolerance <= 0.5)) { err = "z_tolerance out of range"; return false; }
        if (!(c.yaw_tolerance > 0.01 && c.yaw_tolerance <= 0.6)) { err = "yaw_tolerance out of range"; return false; }
        if (!(c.stable_seconds >= 0.0 && c.stable_seconds <= 10.0)) { err = "stable_seconds out of range"; return false; }
        if (!(c.leg_timeout > 0 && c.mission_timeout > 0 && c.pose_timeout > 0)) { err = "timeouts must be positive"; return false; }
        if (!(c.state_timeout > 0.5 && c.state_timeout <= 10.0)) { err = "state_timeout out of range (0.5, 10]"; return false; }
        if (!(c.photo_timeout > 0 && c.gimbal_timeout > 0 && c.gimbal_settle >= 0)) { err = "photo/gimbal timeouts invalid"; return false; }
        if (!(c.align_timeout > 0 && c.settle_timeout > 0 && c.prestream_seconds > 0)) { err = "phase timeouts invalid"; return false; }
        if (!(c.min_z < c.max_z && c.takeoff_z >= c.min_z && c.takeoff_z <= c.max_z)) { err = "takeoff_z outside min_z/max_z"; return false; }
        if (!(c.geofence_radius > 0 && c.geofence_radius <= 100)) { err = "geofence_radius out of range"; return false; }
        if (!(c.kp_xy > 0 && c.kp_z > 0 && c.kp_yaw > 0)) { err = "gains must be positive"; return false; }
        for (std::size_t i = 0; i < c.waypoints.size(); ++i) {
            const Waypoint& p = c.waypoints[i];
            if (!finite_xyz(p.x, p.y, p.z)) { err = "waypoint " + std::to_string(i) + " has non-finite xyz"; return false; }
            if (p.z < c.min_z || p.z > c.max_z) { err = "waypoint " + std::to_string(i) + " z outside min_z/max_z"; return false; }
            if (std::hypot(p.x, p.y) > c.geofence_radius) { err = "waypoint " + std::to_string(i) + " outside geofence"; return false; }
            if (p.hold < 0 || p.hold > 120) { err = "waypoint " + std::to_string(i) + " hold out of range"; return false; }
            if (p.speed < 0 || p.speed > 3.0) { err = "waypoint " + std::to_string(i) + " speed out of range"; return false; }
            if (p.tolerance < 0 || p.tolerance > 1.0) { err = "waypoint " + std::to_string(i) + " tolerance out of range"; return false; }
            if (p.timeout < 0 || p.timeout > 600) { err = "waypoint " + std::to_string(i) + " timeout out of range"; return false; }
            if (p.has_gimbal) {
                if (p.gimbal_pitch < -90 || p.gimbal_pitch > 90) { err = "waypoint " + std::to_string(i) + " gimbal_pitch out of range"; return false; }
                if (p.gimbal_yaw < -170 || p.gimbal_yaw > 170) { err = "waypoint " + std::to_string(i) + " gimbal_yaw out of range"; return false; }
            }
        }
        return true;
    } catch (const std::exception& e) {
        err = std::string("YAML: ") + e.what();
        return false;
    }
}

static void print_mission(const MissionConfig& c, const std::string& path) {
    std::cout << "mission file: " << path << "\n";
    std::cout << "mission: " << c.name << "\n";
    std::cout << "frame: " << c.frame
              << " (file x=" << (c.x_right_y_forward ? "right" : "SLAM start nose-forward")
              << ", y=" << (c.x_right_y_forward ? "forward" : "left")
              << ", z=up; controller uses local ENU)\n";
    std::cout << "takeoff_z: " << c.takeoff_z << " m (absolute local z, not AGL)\n";
    std::cout << "max_speed: " << c.max_speed << " m/s, max_z_speed: " << c.max_z_speed
              << " m/s, max_yaw_rate: " << (c.max_yaw_rate * 180.0 / kPi) << " deg/s\n";
    std::cout << "arrival_tolerance: " << c.arrival_tolerance << " m, z_tolerance: " << c.z_tolerance
              << " m, yaw_tolerance: " << (c.yaw_tolerance * 180.0 / kPi) << " deg\n";
    std::cout << "pose_timeout: " << c.pose_timeout << " s, state_timeout: " << c.state_timeout << " s\n";
    std::cout << "return_to_start: " << (c.return_to_start ? "true" : "false")
              << ", auto_land: " << (c.auto_land ? "true" : "false") << "\n";
    std::cout << "rc: channel " << c.rc.channel << ", offboard_slot " << c.rc.offboard_slot
              << ", ready_mode " << c.rc.ready_mode
              << " (PWM " << c.rc.minimum << "/" << c.rc.trim << "/" << c.rc.maximum << ")\n";
    std::cout << "waypoints (" << c.waypoints.size() << "):\n";
    for (std::size_t i = 0; i < c.waypoints.size(); ++i) {
        const Waypoint& p = c.waypoints[i];
        std::cout << "  " << i << " " << p.name << " (" << p.file_x << ", " << p.file_y
                  << ", " << p.z << ")";
        if (c.x_right_y_forward)
            std::cout << " [local " << p.x << ", " << p.y << "]";
        std::cout << " yaw=";
        if (p.yaw_mode == Waypoint::YawMode::AUTO) std::cout << "auto";
        else if (p.yaw_mode == Waypoint::YawMode::HOLD) std::cout << "hold";
        else std::cout << (p.yaw_abs * 180.0 / kPi) << "deg";
        std::cout << " hold=" << p.hold << "s";
        if (p.photo) {
            std::cout << " PHOTO";
            if (p.has_gimbal) std::cout << " gimbal(" << p.gimbal_pitch << "," << p.gimbal_yaw << ")";
        }
        if (p.speed > 0) std::cout << " speed=" << p.speed;
        if (p.tolerance > 0) std::cout << " tol=" << p.tolerance;
        std::cout << "\n";
    }
    std::cout << "photo dir: " << c.photo_dir << "\n";
    std::cout << "log dir:   " << c.log_dir << "\n";
}

// ---------------------------------------------------------------------------
// mission node
// ---------------------------------------------------------------------------
class MissionNode {
public:
    explicit MissionNode(const MissionConfig& cfg)
        : cfg_(cfg), gate_(cfg.rc), nh_(), pnh_("~") {
        gate_.config = cfg_.rc;
    }
    ~MissionNode() { stop_workers(); }

    void run() {
        state_sub_ = nh_.subscribe("mavros/state", 10, &MissionNode::state_cb, this);
        rc_sub_ = nh_.subscribe("/mavros/rc/in", 10, &MissionNode::rc_cb, this);
        pose_sub_ = nh_.subscribe("/mavros/local_position/pose", 10, &MissionNode::pose_cb, this);
        odom_sub_ = nh_.subscribe("/Odometry", 10, &MissionNode::odom_cb, this);
        photo_done_sub_ = nh_.subscribe("/waypoint_mission/photo_done", 10, &MissionNode::photo_done_cb, this);
        raw_pub_ = nh_.advertise<mavros_msgs::PositionTarget>("/mavros/setpoint_raw/local", 10);
        status_pub_ = nh_.advertise<std_msgs::String>("/waypoint_mission/status", 10);
        photo_trigger_pub_ = nh_.advertise<std_msgs::String>("/waypoint_mission/photo_trigger", 10);
        arming_client_ = nh_.serviceClient<mavros_msgs::CommandBool>("mavros/cmd/arming");
        set_mode_client_ = nh_.serviceClient<mavros_msgs::SetMode>("mavros/set_mode");

        start_workers();
        tick_timer_ = nh_.createTimer(ros::Duration(1.0 / 30.0), &MissionNode::tick, this);
        ROS_WARN("waypoint_mission ready. IDLE: no setpoints, no arm, no mode request.");
        ROS_WARN("RC gate: leave OFFBOARD slot in %s, then switch to OFFBOARD slot %d to start.",
                 cfg_.rc.ready_mode.c_str(), cfg_.rc.offboard_slot);
        if (cfg_.bench_mode) ROS_WARN("bench_mode=true: the gate will be logged but no OFFBOARD/arm/setpoint is sent.");
        ros::spin();
        stop_workers();
    }

private:
    // ---- phase ----
    enum class Phase {
        IDLE, PRESTREAM, WAIT_OFFBOARD, WAIT_ARMED, TAKEOFF, GOTO, ALIGN, SETTLE,
        HOLD, PHOTO_PREP, PHOTO_WAIT, RETURN, LAND_REQUEST, WAIT_LAND, WAIT_DISARM,
        HOLD_COMPLETE, ABORT, DONE
    };
    static const char* phase_name(Phase p) {
        switch (p) {
            case Phase::IDLE: return "IDLE";
            case Phase::PRESTREAM: return "PRESTREAM";
            case Phase::WAIT_OFFBOARD: return "WAIT_OFFBOARD";
            case Phase::WAIT_ARMED: return "WAIT_ARMED";
            case Phase::TAKEOFF: return "TAKEOFF";
            case Phase::GOTO: return "GOTO";
            case Phase::ALIGN: return "ALIGN";
            case Phase::SETTLE: return "SETTLE";
            case Phase::HOLD: return "HOLD";
            case Phase::PHOTO_PREP: return "PHOTO_PREP";
            case Phase::PHOTO_WAIT: return "PHOTO_WAIT";
            case Phase::RETURN: return "RETURN";
            case Phase::LAND_REQUEST: return "LAND_REQUEST";
            case Phase::WAIT_LAND: return "WAIT_LAND";
            case Phase::WAIT_DISARM: return "WAIT_DISARM";
            case Phase::HOLD_COMPLETE: return "HOLD_COMPLETE";
            case Phase::ABORT: return "ABORT";
            case Phase::DONE: return "DONE";
        }
        return "?";
    }

    // ---- ROS callbacks ----
    void state_cb(const mavros_msgs::State::ConstPtr& m) {
        state_ = *m;
        state_rx_ = ros::WallTime::now().toSec();
    }
    void rc_cb(const mavros_msgs::RCIn::ConstPtr& m) {
        const double now = ros::WallTime::now().toSec();
        rc_rx_ = now;
        rc_rssi_ = m->rssi;
        const int idx = cfg_.rc.channel - 1;
        if (idx >= 0 && static_cast<std::size_t>(idx) < m->channels.size()) {
            rc_pwm_ = m->channels[idx];
            rc_slot_ = cfg_.rc.slot(rc_pwm_);
        } else {
            rc_pwm_ = 0;
            rc_slot_ = 0;
        }
        rc_valid_ = rc_slot_ != 0 && rc_rssi_ != 0;
        rc_selected_ = rc_slot_ == cfg_.rc.offboard_slot;
    }
    void pose_cb(const geometry_msgs::PoseStamped::ConstPtr& m) {
        pose_ = *m;
        pose_rx_ = ros::WallTime::now().toSec();
        tf2::Quaternion q(m->pose.orientation.x, m->pose.orientation.y,
                          m->pose.orientation.z, m->pose.orientation.w);
        if (q.length2() > 1e-8) {
            double r, p;
            tf2::Matrix3x3(q).getRPY(r, p, yaw_);
            yaw_valid_ = std::isfinite(yaw_);
        } else {
            yaw_valid_ = false;
        }
        pose_valid_ = yaw_valid_ &&
                      finite_xyz(m->pose.position.x, m->pose.position.y, m->pose.position.z);
    }
    void odom_cb(const nav_msgs::Odometry::ConstPtr& /*m*/) {
        odom_rx_ = ros::WallTime::now().toSec();
    }
    void photo_done_cb(const std_msgs::String::ConstPtr& m) {
        std::lock_guard<std::mutex> lock(photo_mutex_);
        if (!photo_pending_) return;
        // The trigger payload is unique per waypoint. Ignore late acknowledgements
        // from a previous trigger (for example after a photo timeout).
        if (photo_base_.empty() || m->data.find(photo_base_) == std::string::npos) return;
        photo_result_ = m->data;
        photo_done_ = true;
    }

    // ---- worker threads ----
    void start_workers() {
        svc_thread_ = std::thread(&MissionNode::service_worker, this);
        gim_thread_ = std::thread(&MissionNode::gimbal_worker, this);
    }
    void stop_workers() {
        if (workers_stopped_.exchange(true)) return;
        svc_stop_ = true;
        gim_stop_ = true;
        svc_cv_.notify_all();
        gim_cv_.notify_all();
        if (svc_thread_.joinable()) svc_thread_.join();
        if (gim_thread_.joinable()) gim_thread_.join();
    }

    enum class SvcKind { NONE, OFFBOARD, ARM, LAND };
    struct SvcReq { SvcKind kind = SvcKind::NONE; uint64_t gen = 0; };
    void queue_service(SvcKind kind) {
        {
            std::lock_guard<std::mutex> lock(svc_mutex_);
            svc_queue_.push_back(SvcReq{kind, mission_gen_});
        }
        svc_cv_.notify_one();
    }
    void service_worker() {
        while (!svc_stop_) {
            SvcReq req;
            {
                std::unique_lock<std::mutex> lock(svc_mutex_);
                svc_cv_.wait(lock, [this] { return svc_stop_ || !svc_queue_.empty(); });
                if (svc_stop_) break;
                req = svc_queue_.front();
                svc_queue_.pop_front();
            }
            bool ok = false;
            if (req.kind == SvcKind::OFFBOARD || req.kind == SvcKind::LAND) {
                mavros_msgs::SetMode s;
                s.request.custom_mode = (req.kind == SvcKind::OFFBOARD) ? "OFFBOARD" : "AUTO.LAND";
                ok = set_mode_client_.call(s) && s.response.mode_sent;
            } else if (req.kind == SvcKind::ARM) {
                mavros_msgs::CommandBool s;
                s.request.value = true;
                ok = arming_client_.call(s) && s.response.success;
            }
            {
                std::lock_guard<std::mutex> lock(svc_result_mutex_);
                svc_result_kind_ = req.kind;
                svc_result_ok_ = ok;
                svc_result_gen_ = req.gen;
                svc_result_new_ = true;
            }
            ROS_INFO("async request %d accepted=%d gen=%lu", int(req.kind), int(ok),
                     static_cast<unsigned long>(req.gen));
        }
    }

    struct GimReq { uint64_t gen = 0; uint64_t id = 0; int pitch = 0; int yaw = 0; };
    void queue_gimbal(int pitch, int yaw) {
        {
            std::lock_guard<std::mutex> lock(gim_mutex_);
            const uint64_t id = ++gimbal_req_id_;
            gim_queue_.push_back(GimReq{mission_gen_, id, pitch, yaw});
        }
        gim_cv_.notify_one();
    }
    void gimbal_worker() {
        while (!gim_stop_) {
            GimReq req;
            {
                std::unique_lock<std::mutex> lock(gim_mutex_);
                gim_cv_.wait(lock, [this] { return gim_stop_ || !gim_queue_.empty(); });
                if (gim_stop_) break;
                req = gim_queue_.front();
                gim_queue_.pop_front();
            }
            bool ok = false;
            gcu_handle_t* h = gcu_connect("192.168.144.108", 2332);
            if (h) {
                ok = gcu_set_angle(h, 0, req.pitch, req.yaw) == 0;
                gcu_disconnect(h);
            }
            {
                std::lock_guard<std::mutex> lock(gim_result_mutex_);
                gim_result_ok_ = ok;
                gim_result_gen_ = req.gen;
                gim_result_id_ = req.id;
                gim_result_new_ = true;
            }
            ROS_WARN("gimbal request pitch=%d yaw=%d acknowledged=%d", req.pitch, req.yaw, int(ok));
        }
    }

    // ---- state machine ----
    void tick(const ros::TimerEvent&) {
        const double now = ros::WallTime::now().toSec();
        const bool rc_fresh = (now - rc_rx_) <= cfg_.rc.timeout;
        const bool state_fresh = (now - state_rx_) <= cfg_.state_timeout;
        const bool pose_fresh = (now - pose_rx_) <= cfg_.pose_timeout;
        const bool odom_fresh = (now - odom_rx_) <= cfg_.pose_timeout;

        // Service/gimbal/photo results are consumed every tick.
        SvcKind svc_kind = SvcKind::NONE;
        bool svc_ok = false;
        uint64_t svc_gen = 0;
        bool svc_new = false;
        {
            std::lock_guard<std::mutex> lock(svc_result_mutex_);
            if (svc_result_new_) {
                svc_kind = svc_result_kind_;
                svc_ok = svc_result_ok_;
                svc_gen = svc_result_gen_;
                svc_result_new_ = false;
                svc_new = true;
            }
        }
        bool gim_new = false, gim_ok = false;
        uint64_t gim_gen = 0, gim_id = 0;
        {
            std::lock_guard<std::mutex> lock(gim_result_mutex_);
            if (gim_result_new_) {
                gim_new = true;
                gim_ok = gim_result_ok_;
                gim_gen = gim_result_gen_;
                gim_id = gim_result_id_;
                gim_result_new_ = false;
            }
        }
        bool photo_new = false;
        std::string photo_result;
        {
            std::lock_guard<std::mutex> lock(photo_mutex_);
            if (photo_done_ && photo_pending_) {
                photo_new = true;
                photo_result = photo_result_;
                photo_done_ = false;
            }
        }

        if (phase_ == Phase::IDLE) {
            const bool baseline = rc_selected_ || (state_fresh && state_.mode == cfg_.rc.ready_mode && !state_.armed);
            const bool was_ready = gate_.ready();
            const RcEvent ev = gate_.update(now, rc_fresh && rc_valid_ && baseline, rc_selected_, false);
            if (!was_ready && gate_.ready())
                ROS_WARN("RC gate READY in %s: switch to OFFBOARD slot %d to start.",
                         cfg_.rc.ready_mode.c_str(), cfg_.rc.offboard_slot);
            if (ev == RcEvent::START) try_start(now, rc_fresh, state_fresh, pose_fresh, odom_fresh);
            publish_status(now, rc_fresh, state_fresh, pose_fresh, odom_fresh);
            return;
        }

        if (!mission_active_) {
            publish_status(now, rc_fresh, state_fresh, pose_fresh, odom_fresh);
            return;
        }

        // Stop conditions that must never be fought by this node.
        if (!rc_fresh || !rc_valid_ || !rc_selected_) {
            abort_mission("rc_switch_out_or_timeout");
            return;
        }
        if (!state_fresh || !state_.connected) {
            abort_mission("fc_state_stale");
            return;
        }
        if (!pose_fresh || !pose_valid_ || !odom_fresh) {
            abort_mission("localization_stale");
            return;
        }
        if (now - mission_start_ > cfg_.mission_timeout) {
            abort_mission("mission_timeout");
            return;
        }

        // Mode monitoring. During PRESTREAM/WAIT_OFFBOARD/WAIT_ARMED the mode may
        // still be POSCTL. From TAKEOFF to RETURN it must stay OFFBOARD.
        const bool flying_phase = phase_ >= Phase::TAKEOFF && phase_ <= Phase::RETURN;
        if (flying_phase && (state_.mode != "OFFBOARD" || !state_.armed)) {
            abort_mission("mode_or_arm_lost");
            return;
        }
        if (phase_ == Phase::WAIT_ARMED && state_.mode != "OFFBOARD") {
            abort_mission("offboard_lost_before_arm");
            return;
        }
        if ((phase_ == Phase::WAIT_LAND || phase_ == Phase::WAIT_DISARM) &&
            state_.armed && state_.mode != "AUTO.LAND" && state_.mode != "OFFBOARD") {
            abort_mission("pilot_takeover_during_land");
            return;
        }

        // Consume async results.
        if (svc_new && svc_gen == mission_gen_) {
            last_svc_kind_ = svc_kind;
            last_svc_ok_ = svc_ok;
            last_svc_at_ = now;
            if (svc_kind == SvcKind::ARM && !svc_ok)
                ROS_WARN("arming request rejected; will retry until WAIT_ARMED timeout");
            if (svc_kind == SvcKind::LAND && !svc_ok)
                ROS_WARN("AUTO.LAND request rejected; will retry");
        }
        if (gim_new && gim_gen == mission_gen_ && gim_id == gimbal_req_id_) {
            gimbal_done_ = true;
            gimbal_ok_ = gim_ok;
            gimbal_done_at_ = now;
        }

        switch (phase_) {
            case Phase::PRESTREAM: phase_prestream(now); break;
            case Phase::WAIT_OFFBOARD: phase_wait_offboard(now); break;
            case Phase::WAIT_ARMED: phase_wait_armed(now); break;
            case Phase::TAKEOFF: phase_takeoff(now); break;
            case Phase::GOTO: phase_goto(now); break;
            case Phase::ALIGN: phase_align(now); break;
            case Phase::SETTLE: phase_settle(now); break;
            case Phase::HOLD: phase_hold(now); break;
            case Phase::PHOTO_PREP: phase_photo_prep(now); break;
            case Phase::PHOTO_WAIT: phase_photo_wait(now, photo_new, photo_result); break;
            case Phase::RETURN: phase_return(now); break;
            case Phase::LAND_REQUEST: phase_land_request(now); break;
            case Phase::WAIT_LAND: phase_wait_land(now); break;
            case Phase::WAIT_DISARM: phase_wait_disarm(now); break;
            case Phase::HOLD_COMPLETE:
                publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
                break;
            case Phase::ABORT:
            case Phase::DONE:
            case Phase::IDLE:
                break;
        }
        publish_status(now, rc_fresh, state_fresh, pose_fresh, odom_fresh);
        write_log(now);
    }

    void try_start(double now, bool rc_fresh, bool state_fresh, bool pose_fresh, bool odom_fresh) {
        if (!state_fresh || !state_.connected) { ROS_WARN("start rejected: no FCU state"); return; }
        if (state_.armed) { ROS_ERROR("start rejected: vehicle is already armed; disarm and switch out/in"); return; }
        if (!rc_fresh) { ROS_WARN("start rejected: RC stale"); return; }
        if (!pose_fresh || !pose_valid_) { ROS_ERROR("start rejected: local pose stale/invalid"); return; }
        if (!odom_fresh) { ROS_ERROR("start rejected: /Odometry stale; start FAST-LIO and the bridge"); return; }

        if (cfg_.bench_mode) {
            ROS_WARN("BENCH: RC OFFBOARD edge observed. No OFFBOARD request, no arm, no setpoints.");
            return;
        }

        home_x_ = pose_.pose.position.x;
        home_y_ = pose_.pose.position.y;
        home_z_ = pose_.pose.position.z;
        home_yaw_ = yaw_;
        if (std::fabs(home_z_) > 0.3)
            ROS_WARN("local z at start is %.2f m; waypoint z/takeoff_z are absolute local z, not AGL.", home_z_);

        ++mission_gen_;
        mission_active_ = true;
        publishing_enabled_ = true;
        mission_start_ = now;
        phase_start_ = now;
        phase_ = Phase::PRESTREAM;
        last_svc_at_ = -1;
        last_svc_kind_ = SvcKind::NONE;
        last_svc_ok_ = false;
        gimbal_waiting_ = false;
        gimbal_done_ = false;
        gimbal_ok_ = false;
        photo_pending_ = false;
        photo_done_ = false;

        const std::string stamp = stamp_now();
        mission_dir_ = cfg_.photo_dir + "/" + sanitize_name(cfg_.name) + "_" + stamp;
        if (!cfg_.photo_dir.empty() && !ensure_dir(mission_dir_))
            ROS_WARN("cannot create photo dir %s; photos may fail", mission_dir_.c_str());
        if (!cfg_.log_dir.empty()) ensure_dir(cfg_.log_dir);
        open_log(stamp);
        log_event("mission start: home=(%.3f, %.3f, %.3f) yaw=%.1f deg", home_x_, home_y_, home_z_,
                  home_yaw_ * 180.0 / kPi);
        ROS_WARN("MISSION START gen=%lu home=(%.3f,%.3f,%.3f) waypoints=%zu",
                 static_cast<unsigned long>(mission_gen_), home_x_, home_y_, home_z_, cfg_.waypoints.size());
    }

    void abort_mission(const std::string& reason) {
        if (!mission_active_) return;
        mission_active_ = false;
        publishing_enabled_ = false;
        phase_ = Phase::ABORT;
        log_event("ABORT: %s", reason.c_str());
        ROS_ERROR("MISSION ABORT: %s. Setpoints stopped; no mode is re-requested. "
                  "PX4 will use its offboard-loss failsafe; pilot keeps control.", reason.c_str());
        close_log();
    }

    void finish_mission(const std::string& reason) {
        if (!mission_active_) return;
        mission_active_ = false;
        publishing_enabled_ = false;
        phase_ = Phase::DONE;
        log_event("DONE: %s", reason.c_str());
        ROS_WARN("MISSION DONE: %s", reason.c_str());
        close_log();
    }

    // ---- phase implementations ----
    void phase_prestream(double now) {
        publish_current_hold();
        if (now - phase_start_ >= cfg_.prestream_seconds) {
            queue_service(SvcKind::OFFBOARD);
            last_svc_at_ = now;
            phase_ = Phase::WAIT_OFFBOARD;
            phase_start_ = now;
            log_event("prestream done; requesting OFFBOARD");
        }
    }

    void phase_wait_offboard(double now) {
        publish_current_hold();
        if (state_.mode == "OFFBOARD") {
            queue_service(SvcKind::ARM);
            last_svc_at_ = now;
            phase_ = Phase::WAIT_ARMED;
            phase_start_ = now;
            log_event("OFFBOARD confirmed; requesting arm");
            return;
        }
        if (now - phase_start_ > 5.0) { abort_mission("offboard_timeout"); return; }
        // Retry the mode request once per second; the RC switch may already have
        // put PX4 in OFFBOARD, in which case this is harmless.
        if (now - last_svc_at_ > 1.0) {
            queue_service(SvcKind::OFFBOARD);
            last_svc_at_ = now;
        }
    }

    void phase_wait_armed(double now) {
        publish_current_hold();
        if (state_.armed) {
            index_ = 0;
            phase_ = Phase::TAKEOFF;
            phase_start_ = now;
            leg_start_ = now;
            target_x_ = home_x_;
            target_y_ = home_y_;
            target_z_ = cfg_.takeoff_z;
            final_yaw_ = home_yaw_;
            stable_since_ = -1;
            log_event("armed; takeoff to z=%.2f", target_z_);
            return;
        }
        if (now - phase_start_ > 5.0) { abort_mission("arm_timeout"); return; }
        if (now - last_svc_at_ > 1.0) {
            queue_service(SvcKind::ARM);
            last_svc_at_ = now;
        }
    }

    void phase_takeoff(double now) {
        double vx, vy, vz, yr;
        compute_velocity(target_x_, target_y_, target_z_, final_yaw_, cfg_.max_speed, &vx, &vy, &vz, &yr);
        publish_velocity(vx, vy, vz, yr);
        if (position_arrived(target_x_, target_y_, target_z_, cfg_.arrival_tolerance, cfg_.z_tolerance)) {
            if (stable_since_ < 0) stable_since_ = now;
            if (now - stable_since_ >= cfg_.stable_seconds) {
                log_event("takeoff complete");
                start_waypoint(0, now);
            }
        } else {
            stable_since_ = -1;
        }
        if (now - leg_start_ > cfg_.leg_timeout) abort_mission("takeoff_timeout");
    }

    void start_waypoint(std::size_t idx, double now) {
        index_ = idx;
        const Waypoint& p = cfg_.waypoints[idx];
        target_x_ = p.x;
        target_y_ = p.y;
        target_z_ = p.z;
        final_yaw_ = yaw_;
        stable_since_ = -1;
        photo_pending_ = false;
        photo_done_ = false;
        gimbal_done_ = false;
        gimbal_ok_ = false;
        phase_ = Phase::GOTO;
        phase_start_ = now;
        leg_start_ = now;
        log_event("GOTO wp%zu %s (%.2f,%.2f,%.2f)", idx, p.name.c_str(), p.x, p.y, p.z);
    }

    void phase_goto(double now) {
        const Waypoint& p = cfg_.waypoints[index_];
        double yaw_target = leg_yaw_target(index_);
        const double max_sp = (p.speed > 0 ? p.speed : cfg_.max_speed);
        double vx, vy, vz, yr;
        compute_velocity(target_x_, target_y_, target_z_, yaw_target, max_sp, &vx, &vy, &vz, &yr);
        publish_velocity(vx, vy, vz, yr);
        if (position_arrived(target_x_, target_y_, target_z_, tolerance_for(p), cfg_.z_tolerance)) {
            if (stable_since_ < 0) stable_since_ = now;
            if (now - stable_since_ >= 0.2) {
                stable_since_ = -1;
                phase_start_ = now;
                final_yaw_ = outgoing_yaw(index_);
                if (p.yaw_mode == Waypoint::YawMode::HOLD ||
                    std::fabs(wrap_pi(final_yaw_ - yaw_)) <= cfg_.yaw_tolerance) {
                    phase_ = Phase::SETTLE;
                    log_event("arrived wp%zu %s; settle", index_, p.name.c_str());
                } else {
                    phase_ = Phase::ALIGN;
                    log_event("arrived wp%zu %s; align yaw %.1f deg", index_, p.name.c_str(),
                              final_yaw_ * 180.0 / kPi);
                }
            }
        } else {
            stable_since_ = -1;
        }
        const double limit = (p.timeout > 0 ? p.timeout : cfg_.leg_timeout);
        if (now - leg_start_ > limit) abort_mission("waypoint_timeout");
    }

    void phase_align(double now) {
        double vx, vy, vz, yr;
        compute_velocity(target_x_, target_y_, target_z_, final_yaw_, cfg_.max_speed, &vx, &vy, &vz, &yr);
        publish_velocity(vx, vy, vz, yr);
        if (std::fabs(wrap_pi(final_yaw_ - yaw_)) <= cfg_.yaw_tolerance) {
            phase_ = Phase::SETTLE;
            phase_start_ = now;
            stable_since_ = -1;
            log_event("yaw aligned wp%zu", index_);
            return;
        }
        if (now - phase_start_ > cfg_.align_timeout) {
            ROS_WARN("yaw align timeout at wp%zu; accepting current heading (error %.1f deg)", index_,
                     wrap_pi(final_yaw_ - yaw_) * 180.0 / kPi);
            log_event("yaw align timeout; accept current heading");
            final_yaw_ = yaw_;   // stop commanding the unreachable heading
            phase_ = Phase::SETTLE;
            phase_start_ = now;
            stable_since_ = -1;
        }
    }

    void phase_settle(double now) {
        publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
        const Waypoint& p = cfg_.waypoints[index_];
        const bool xy = position_arrived(target_x_, target_y_, target_z_, tolerance_for(p), cfg_.z_tolerance);
        const bool yaw_ok = std::fabs(wrap_pi(final_yaw_ - yaw_)) <= cfg_.yaw_tolerance;
        if (xy && yaw_ok) {
            if (stable_since_ < 0) stable_since_ = now;
            if (now - stable_since_ >= cfg_.stable_seconds) {
                phase_ = Phase::HOLD;
                phase_start_ = now;
                hold_start_ = now;
                if (p.photo && p.has_gimbal) {
                    gimbal_waiting_ = true;
                    gimbal_done_ = false;
                    gimbal_ok_ = false;
                    gimbal_queue_at_ = now;
                    queue_gimbal(p.gimbal_pitch, p.gimbal_yaw);
                    log_event("gimbal queued pitch=%d yaw=%d", p.gimbal_pitch, p.gimbal_yaw);
                }
                log_event("stable at wp%zu %s; hold %.1fs%s", index_, p.name.c_str(), p.hold,
                          p.photo ? " + photo" : "");
            }
        } else {
            stable_since_ = -1;
        }
        if (now - phase_start_ > cfg_.settle_timeout) abort_mission("settle_timeout");
    }

    void phase_hold(double now) {
        publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
        const Waypoint& p = cfg_.waypoints[index_];
        if (now - hold_start_ >= p.hold) {
            if (p.photo) {
                phase_ = Phase::PHOTO_PREP;
                phase_start_ = now;
            } else {
                next_waypoint(now);
            }
        }
    }

    void phase_photo_prep(double now) {
        publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
        const Waypoint& p = cfg_.waypoints[index_];
        if (p.has_gimbal) {
            if (!gimbal_done_) {
                if (now - std::max(gimbal_queue_at_, phase_start_) > cfg_.gimbal_timeout) {
                    gimbal_done_ = true;
                    gimbal_ok_ = false;
                    gimbal_done_at_ = now;
                    log_event("gimbal timeout");
                } else {
                    return;
                }
            }
            if (!gimbal_ok_) {
                if (cfg_.gimbal_required) { abort_mission("gimbal_failed"); return; }
                ROS_WARN("gimbal failed at wp%zu; photo continues at current gimbal angle", index_);
            }
            if (now - gimbal_done_at_ < cfg_.gimbal_settle) return;
        }
        trigger_photo(now);
    }

    void phase_photo_wait(double now, bool photo_new, const std::string& photo_result) {
        publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
        if (photo_new) {
            photo_pending_ = false;
            const bool ok = photo_result.rfind("OK ", 0) == 0;
            log_event("photo %s: %s", ok ? "ok" : "failed", photo_result.c_str());
            write_photo_row(now, photo_result, ok);
            if (!ok && cfg_.photo_required) { abort_mission("photo_failed"); return; }
            next_waypoint(now);
            return;
        }
        if (now - photo_start_ > cfg_.photo_timeout) {
            photo_pending_ = false;
            log_event("photo timeout");
            write_photo_row(now, "FAIL timeout", false);
            ROS_WARN("photo timeout at wp%zu", index_);
            if (cfg_.photo_required) { abort_mission("photo_timeout"); return; }
            next_waypoint(now);
        }
    }

    void phase_return(double now) {
        double yaw_target = std::atan2(home_y_ - pose_.pose.position.y, home_x_ - pose_.pose.position.x);
        double vx, vy, vz, yr;
        compute_velocity(home_x_, home_y_, cfg_.takeoff_z, yaw_target, cfg_.max_speed, &vx, &vy, &vz, &yr);
        publish_velocity(vx, vy, vz, yr);
        if (position_arrived(home_x_, home_y_, cfg_.takeoff_z, cfg_.arrival_tolerance, cfg_.z_tolerance)) {
            if (stable_since_ < 0) stable_since_ = now;
            if (now - stable_since_ >= cfg_.stable_seconds) {
                log_event("home reached");
                target_x_ = home_x_;
                target_y_ = home_y_;
                target_z_ = cfg_.takeoff_z;
                if (cfg_.auto_land) {
                    phase_ = Phase::LAND_REQUEST;
                    phase_start_ = now;
                    land_retries_ = 0;
                } else {
                    phase_ = Phase::HOLD_COMPLETE;
                    log_event("auto_land=false; holding for pilot");
                }
            }
        } else {
            stable_since_ = -1;
        }
        if (now - leg_start_ > cfg_.leg_timeout) abort_mission("return_timeout");
    }

    void phase_land_request(double now) {
        // If return_to_start is false this is the last waypoint, not home.
        publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
        queue_service(SvcKind::LAND);
        last_svc_at_ = now;
        phase_ = Phase::WAIT_LAND;
        phase_start_ = now;
        log_event("requesting AUTO.LAND");
    }

    void phase_wait_land(double now) {
        if (state_.mode == "AUTO.LAND") {
            phase_ = Phase::WAIT_DISARM;
            phase_start_ = now;
            publishing_enabled_ = false;
            log_event("AUTO.LAND confirmed; waiting for disarm");
            return;
        }
        // Keep holding until PX4 accepts AUTO.LAND.
        publish_position_hold(target_x_, target_y_, target_z_, final_yaw_);
        if (now - last_svc_at_ > 1.5 && now - phase_start_ < 10.0) {
            queue_service(SvcKind::LAND);
            last_svc_at_ = now;
            ++land_retries_;
            if (land_retries_ >= 5) { abort_mission("land_rejected"); return; }
        }
        if (now - phase_start_ > 10.0) abort_mission("land_timeout");
    }

    void phase_wait_disarm(double now) {
        if (!state_.armed) {
            finish_mission("disarmed after landing");
            return;
        }
        if (now - phase_start_ > 120.0) abort_mission("disarm_timeout");
    }

    void next_waypoint(double now) {
        ++index_;
        if (index_ < cfg_.waypoints.size()) {
            start_waypoint(index_, now);
        } else if (cfg_.return_to_start) {
            phase_ = Phase::RETURN;
            phase_start_ = now;
            leg_start_ = now;
            stable_since_ = -1;
            final_yaw_ = std::atan2(home_y_ - pose_.pose.position.y,
                                    home_x_ - pose_.pose.position.x);
            log_event("mission waypoints done; return to start");
        } else if (cfg_.auto_land) {
            phase_ = Phase::LAND_REQUEST;
            phase_start_ = now;
            land_retries_ = 0;
        } else {
            phase_ = Phase::HOLD_COMPLETE;
            log_event("mission done; holding for pilot (auto_land=false)");
        }
    }

    // ---- control helpers ----
    double tolerance_for(const Waypoint& p) const {
        return p.tolerance > 0 ? p.tolerance : cfg_.arrival_tolerance;
    }

    double leg_yaw_target(std::size_t idx) const {
        const Waypoint& p = cfg_.waypoints[idx];
        if (p.yaw_mode == Waypoint::YawMode::ABSOLUTE) return p.yaw_abs;
        if (p.yaw_mode == Waypoint::YawMode::HOLD) return final_yaw_;
        // AUTO: face along the leg. For the first leg, start from home.
        const double px = (idx == 0) ? home_x_ : cfg_.waypoints[idx - 1].x;
        const double py = (idx == 0) ? home_y_ : cfg_.waypoints[idx - 1].y;
        const double dx = p.x - px;
        const double dy = p.y - py;
        if (std::hypot(dx, dy) < 0.05) return yaw_;
        return std::atan2(dy, dx);
    }

    double outgoing_yaw(std::size_t idx) const {
        const Waypoint& p = cfg_.waypoints[idx];
        if (p.yaw_mode == Waypoint::YawMode::ABSOLUTE) return p.yaw_abs;
        if (p.yaw_mode == Waypoint::YawMode::HOLD) return yaw_;
        double nx, ny;
        if (idx + 1 < cfg_.waypoints.size()) {
            nx = cfg_.waypoints[idx + 1].x;
            ny = cfg_.waypoints[idx + 1].y;
        } else {
            nx = home_x_;
            ny = home_y_;
        }
        const double dx = nx - p.x;
        const double dy = ny - p.y;
        if (std::hypot(dx, dy) < 0.05) return yaw_;
        return std::atan2(dy, dx);
    }

    bool position_arrived(double tx, double ty, double tz, double tol, double ztol) const {
        const double dx = tx - pose_.pose.position.x;
        const double dy = ty - pose_.pose.position.y;
        const double dz = tz - pose_.pose.position.z;
        return std::hypot(dx, dy) <= tol && std::fabs(dz) <= ztol;
    }

    void compute_velocity(double tx, double ty, double tz, double yaw_target, double max_sp,
                          double* vx, double* vy, double* vz, double* yaw_rate) const {
        const double dx = tx - pose_.pose.position.x;
        const double dy = ty - pose_.pose.position.y;
        const double dz = tz - pose_.pose.position.z;
        const double dxy = std::hypot(dx, dy);
        double sp = std::min(max_sp, cfg_.kp_xy * dxy);
        sp = std::max(0.0, sp);
        if (dxy > 1e-6) {
            *vx = sp * dx / dxy;
            *vy = sp * dy / dxy;
        } else {
            *vx = 0;
            *vy = 0;
        }
        *vz = std::max(-cfg_.max_z_speed, std::min(cfg_.max_z_speed, cfg_.kp_z * dz));
        double yr = cfg_.kp_yaw * wrap_pi(yaw_target - yaw_);
        yr = std::max(-cfg_.max_yaw_rate, std::min(cfg_.max_yaw_rate, yr));
        *yaw_rate = yr;
    }

    void publish_velocity(double vx, double vy, double vz, double yaw_rate) {
        if (!publishing_enabled_ || !publish_ready_) return;
        mavros_msgs::PositionTarget cmd;
        cmd.header.stamp = ros::Time::now();
        cmd.header.frame_id = "map";
        cmd.coordinate_frame = mavros_msgs::PositionTarget::FRAME_LOCAL_NED;
        cmd.type_mask = mavros_msgs::PositionTarget::IGNORE_PX |
                        mavros_msgs::PositionTarget::IGNORE_PY |
                        mavros_msgs::PositionTarget::IGNORE_PZ |
                        mavros_msgs::PositionTarget::IGNORE_AFX |
                        mavros_msgs::PositionTarget::IGNORE_AFY |
                        mavros_msgs::PositionTarget::IGNORE_AFZ |
                        mavros_msgs::PositionTarget::FORCE |
                        mavros_msgs::PositionTarget::IGNORE_YAW;
        cmd.velocity.x = vx;
        cmd.velocity.y = vy;
        cmd.velocity.z = vz;
        cmd.yaw_rate = yaw_rate;
        raw_pub_.publish(cmd);
        last_cmd_velocity_ = true;
        last_vel_x_ = vx;
        last_vel_y_ = vy;
        last_vel_z_ = vz;
    }

    void publish_position_hold(double x, double y, double z, double yaw) {
        if (!publishing_enabled_ || !publish_ready_) return;
        mavros_msgs::PositionTarget cmd;
        cmd.header.stamp = ros::Time::now();
        cmd.header.frame_id = "map";
        cmd.coordinate_frame = mavros_msgs::PositionTarget::FRAME_LOCAL_NED;
        cmd.type_mask = mavros_msgs::PositionTarget::IGNORE_VX |
                        mavros_msgs::PositionTarget::IGNORE_VY |
                        mavros_msgs::PositionTarget::IGNORE_VZ |
                        mavros_msgs::PositionTarget::IGNORE_AFX |
                        mavros_msgs::PositionTarget::IGNORE_AFY |
                        mavros_msgs::PositionTarget::IGNORE_AFZ |
                        mavros_msgs::PositionTarget::FORCE |
                        mavros_msgs::PositionTarget::IGNORE_YAW_RATE;
        cmd.position.x = x;
        cmd.position.y = y;
        cmd.position.z = z;
        cmd.yaw = yaw;
        raw_pub_.publish(cmd);
        last_cmd_velocity_ = false;
        last_hold_x_ = x;
        last_hold_y_ = y;
        last_hold_z_ = z;
    }

    void publish_current_hold() {
        if (!pose_valid_) return;
        publish_position_hold(pose_.pose.position.x, pose_.pose.position.y,
                              pose_.pose.position.z, yaw_);
    }

    void trigger_photo(double now) {
        if (!publish_ready_) return;
        const Waypoint& p = cfg_.waypoints[index_];
        photo_base_ = mission_dir_ + "/wp" + (index_ < 10 ? "0" : "") + std::to_string(index_) +
                      "_" + sanitize_name(p.name);
        {
            std::lock_guard<std::mutex> lock(photo_mutex_);
            photo_pending_ = true;
            photo_done_ = false;
            photo_result_.clear();
        }
        std_msgs::String msg;
        msg.data = photo_base_;
        photo_trigger_pub_.publish(msg);
        photo_start_ = now;
        phase_ = Phase::PHOTO_WAIT;
        log_event("photo trigger %s", photo_base_.c_str());
    }

    // ---- status / logging ----
    void publish_status(double now, bool rc_fresh, bool state_fresh, bool pose_fresh, bool odom_fresh) {
        if (!status_pub_) return;
        if (now - last_status_at_ < 0.2) return;
        last_status_at_ = now;
        std::ostringstream os;
        os << "phase=" << phase_name(phase_);
        os << " mission=" << sanitize_name(cfg_.name);
        if (mission_active_) os << " wp=" << index_ << "/" << cfg_.waypoints.size();
        os << " armed=" << (state_.armed ? 1 : 0) << " mode=" << state_.mode
           << " connected=" << (state_.connected ? 1 : 0)
           << " rc_slot=" << rc_slot_ << " rc_pwm=" << rc_pwm_
           << " rssi=" << rc_rssi_
           << " gate_ready=" << (gate_.ready() ? 1 : 0)
           << " rc_fresh=" << (rc_fresh ? 1 : 0)
           << " pose_fresh=" << (pose_fresh ? 1 : 0)
           << " odom_fresh=" << (odom_fresh ? 1 : 0);
        auto clamp_age = [](double v) { return std::max(0.0, std::min(999.0, v)); };
        const double dx = target_x_ - pose_.pose.position.x;
        const double dy = target_y_ - pose_.pose.position.y;
        const double dz = target_z_ - pose_.pose.position.z;
        os << std::fixed << std::setprecision(2)
           << " state_age=" << clamp_age(now - state_rx_)
           << " pose_age=" << clamp_age(now - pose_rx_)
           << " odom_age=" << clamp_age(now - odom_rx_)
           << " target=" << std::setprecision(3) << target_x_ << "," << target_y_ << "," << target_z_
           << " dist=" << std::setprecision(2) << std::sqrt(dx * dx + dy * dy + dz * dz)
           << " yaw=" << yaw_ << " yaw_err=" << wrap_pi(final_yaw_ - yaw_)
           << " photo=" << (photo_pending_ ? "pending" : "none")
           << " last_event=" << (last_event_.empty() ? "none" : last_event_);
        std_msgs::String msg;
        msg.data = os.str();
        status_pub_.publish(msg);
    }

    void open_log(const std::string& stamp) {
        if (cfg_.log_dir.empty()) return;
        log_path_ = cfg_.log_dir + "/" + sanitize_name(cfg_.name) + "_" + stamp + ".csv";
        log_.open(log_path_, std::ios::out | std::ios::trunc);
        if (!log_) { ROS_WARN("cannot open log %s", log_path_.c_str()); return; }
        log_ << "ros_time,mission_time,phase,wp,x,y,z,tx,ty,tz,vx,vy,vz,yaw,yaw_err,"
                "mode,armed,rc_slot,photo_state\n";
        log_.flush();
        photos_path_ = cfg_.log_dir + "/" + sanitize_name(cfg_.name) + "_" + stamp + "_photos.csv";
        photos_.open(photos_path_, std::ios::out | std::ios::trunc);
        if (photos_) {
            photos_ << "ros_time,wp,name,ok,path_or_reason,x,y,z,yaw\n";
            photos_.flush();
        }
    }
    void close_log() {
        if (log_.is_open()) { log_.flush(); log_.close(); }
        if (photos_.is_open()) { photos_.flush(); photos_.close(); }
    }
    void log_event(const char* fmt, ...) {
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        ROS_INFO("[%s] %s", phase_name(phase_), buf);
        last_event_ = buf;
        std::replace(last_event_.begin(), last_event_.end(), ' ', '_');
        if (last_event_.size() > 160) last_event_.resize(160);
    }
    void write_log(double now) {
        if (!log_.is_open()) return;
        if (now - last_log_at_ < 0.1) return;
        last_log_at_ = now;
        const double dx = target_x_ - pose_.pose.position.x;
        const double dy = target_y_ - pose_.pose.position.y;
        const double dz = target_z_ - pose_.pose.position.z;
        log_ << std::fixed << std::setprecision(3)
             << ros::Time::now().toSec() << ',' << (now - mission_start_) << ','
             << phase_name(phase_) << ',' << index_ << ','
             << pose_.pose.position.x << ',' << pose_.pose.position.y << ',' << pose_.pose.position.z << ','
             << target_x_ << ',' << target_y_ << ',' << target_z_ << ','
             << (last_cmd_velocity_ ? last_vel_x_ : 0.0) << ',' << (last_cmd_velocity_ ? last_vel_y_ : 0.0)
             << ',' << (last_cmd_velocity_ ? last_vel_z_ : 0.0) << ','
             << yaw_ << ',' << wrap_pi(final_yaw_ - yaw_) << ','
             << state_.mode << ',' << (state_.armed ? 1 : 0) << ',' << rc_slot_ << ','
             << (photo_pending_ ? "pending" : "none") << '\n';
        log_.flush();
    }
    void write_photo_row(double now, const std::string& result, bool ok) {
        if (!photos_.is_open()) return;
        photos_ << std::fixed << std::setprecision(3)
                << ros::Time::now().toSec() << ',' << index_ << ','
                << cfg_.waypoints[index_].name << ',' << (ok ? 1 : 0) << ','
                << result << ',' << pose_.pose.position.x << ',' << pose_.pose.position.y << ','
                << pose_.pose.position.z << ',' << yaw_ << '\n';
        photos_.flush();
    }

    // ---- data ----
    MissionConfig cfg_;
    RcStartGate gate_;
    ros::NodeHandle nh_;
    ros::NodeHandle pnh_;
    ros::Subscriber state_sub_, rc_sub_, pose_sub_, odom_sub_, photo_done_sub_;
    ros::Publisher raw_pub_, status_pub_, photo_trigger_pub_;
    ros::ServiceClient arming_client_, set_mode_client_;
    ros::Timer tick_timer_;

    mavros_msgs::State state_;
    geometry_msgs::PoseStamped pose_;
    double state_rx_ = -1e9, rc_rx_ = -1e9, pose_rx_ = -1e9, odom_rx_ = -1e9;
    double yaw_ = 0;
    bool yaw_valid_ = false, pose_valid_ = false;
    unsigned rc_pwm_ = 0;
    int rc_slot_ = 0, rc_rssi_ = 0;
    bool rc_valid_ = false, rc_selected_ = false;

    bool publish_ready_ = true;
    bool publishing_enabled_ = false;
    bool last_cmd_velocity_ = false;
    double last_vel_x_ = 0, last_vel_y_ = 0, last_vel_z_ = 0;
    double last_hold_x_ = 0, last_hold_y_ = 0, last_hold_z_ = 0;

    bool mission_active_ = false;
    uint64_t mission_gen_ = 0;
    Phase phase_ = Phase::IDLE;
    double mission_start_ = 0, phase_start_ = 0, leg_start_ = 0;
    double hold_start_ = 0, stable_since_ = -1;
    std::size_t index_ = 0;
    double home_x_ = 0, home_y_ = 0, home_z_ = 0, home_yaw_ = 0;
    double target_x_ = 0, target_y_ = 0, target_z_ = 0, final_yaw_ = 0;
    double last_status_at_ = -1e9, last_log_at_ = -1e9;
    std::string last_event_ = "none";

    SvcKind last_svc_kind_ = SvcKind::NONE;
    bool last_svc_ok_ = false;
    double last_svc_at_ = -1e9;
    int land_retries_ = 0;

    // service worker
    std::mutex svc_mutex_, svc_result_mutex_;
    std::condition_variable svc_cv_;
    std::deque<SvcReq> svc_queue_;
    std::thread svc_thread_;
    std::atomic<bool> svc_stop_{false};
    std::atomic<bool> workers_stopped_{false};
    SvcKind svc_result_kind_ = SvcKind::NONE;
    bool svc_result_ok_ = false;
    uint64_t svc_result_gen_ = 0;
    bool svc_result_new_ = false;

    // gimbal worker
    std::mutex gim_mutex_, gim_result_mutex_;
    std::condition_variable gim_cv_;
    std::deque<GimReq> gim_queue_;
    std::thread gim_thread_;
    std::atomic<bool> gim_stop_{false};
    bool gim_result_new_ = false, gim_result_ok_ = false;
    uint64_t gim_result_gen_ = 0, gim_result_id_ = 0;
    uint64_t gimbal_req_id_ = 0;
    bool gimbal_waiting_ = false, gimbal_done_ = false, gimbal_ok_ = false;
    double gimbal_queue_at_ = 0, gimbal_done_at_ = 0;

    // photo
    std::mutex photo_mutex_;
    bool photo_pending_ = false, photo_done_ = false;
    std::string photo_result_, photo_base_;
    double photo_start_ = 0;

    // files
    std::ofstream log_, photos_;
    std::string log_path_, photos_path_, mission_dir_;
};

}  // namespace waypoint_mission

int main(int argc, char** argv) {
    ros::init(argc, argv, "waypoint_mission");
    ros::NodeHandle pnh("~");

    std::string mission_file, err;
    const std::string pkg = ros::package::getPath("waypoint_mission");
    pnh.param<std::string>("mission_file", mission_file, pkg + "/missions/abc_photo.yaml");
    bool dry = false, bench = false;
    pnh.param("dry_run", dry, false);
    pnh.param("bench_mode", bench, false);

    waypoint_mission::MissionConfig cfg;
    cfg.photo_dir = std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") +
                    "/.ros/waypoint_mission/photos";
    cfg.log_dir = std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") +
                  "/.ros/waypoint_mission/logs";
    if (!waypoint_mission::load_mission(mission_file, cfg, err)) {
        ROS_FATAL_STREAM("mission file error: " << mission_file << " : " << err);
        std::cerr << "mission file error: " << mission_file << " : " << err << "\n";
        return 2;
    }
    pnh.param<std::string>("photo_dir", cfg.photo_dir, cfg.photo_dir);
    pnh.param<std::string>("log_dir", cfg.log_dir, cfg.log_dir);
    cfg.dry_run = dry;
    cfg.bench_mode = bench;

    if (cfg.dry_run) {
        waypoint_mission::print_mission(cfg, mission_file);
        return 0;
    }

    waypoint_mission::MissionNode node(cfg);
    node.run();
    return 0;
}
