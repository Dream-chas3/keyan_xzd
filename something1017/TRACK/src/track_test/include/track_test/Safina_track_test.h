#ifndef SAFINA_TRACK_TEST_H
#define SAFINA_TRACK_TEST_H

#include <ros/ros.h>

#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/ExtendedState.h>
#include <mavros_msgs/PositionTarget.h>
#include <mavros_msgs/RCIn.h>
#include <std_msgs/Float32MultiArray.h>

#include <cmath>
#include <string>

#include "offboard/offboard_control.h"

class Safina_Track
{
public:
    Safina_Track(ros::NodeHandle node_track);
    ~Safina_Track();

private:
    enum class Swa
    {
        Up = 0,
        Mid = 1,
        Down = 2
    };

    enum class State
    {
        WaitOffboard,  // PX4 is not in OFFBOARD: never act on SWA
        Hold,          // OFFBOARD: hold the position captured on entry
        Track,         // OFFBOARD + SWA mid: follow the primary target
        Land           // OFFBOARD: descend in place, then AUTO.LAND
    };

    ros::NodeHandle node_;
    ros::NodeHandle pnh_;

    ros::Subscriber sub_local_pos_;
    ros::Subscriber sub_velocity_;
    ros::Subscriber sub_rc_;
    ros::Subscriber sub_track_;
    ros::Subscriber sub_extended_state_;
    ros::Publisher pub_setraw_;
    ros::Timer timer_;

    // Must be declared after node_ (constructed with node_).
    OffboardController controller_;

    // ---- parameters (config/track.yaml) ----
    double control_rate_hz_ = 20.0;
    bool require_offboard_ = true;
    double target_timeout_s_ = 0.5;
    double target_loss_grace_s_ = 0.3;
    double pose_timeout_s_ = 0.5;
    double rc_timeout_s_ = 1.0;

    double tracking_vlxy_ = 0.8;
    double max_forward_speed_ = 0.3;
    double max_backward_speed_ = 0.15;
    double tracking_ryaw_ = 1.5;
    double max_yaw_rate_ = 1.2;
    // Refuse to start tracking below this height (prevents ground dragging).
    double min_tracking_height_m_ = 0.3;

    // Metric standoff from the yolo node (depth fusion, bbox fallback).
    bool use_metric_distance_ = true;
    double target_distance_m_ = 3.0;
    double distance_kp_ = 0.5;
    double distance_deadband_m_ = 0.2;

    double altitude_kp_ = 0.8;
    double altitude_kd_ = 0.2;
    double altitude_offset_m_ = 0.0;
    double max_climb_speed_ = 0.4;
    double max_descent_speed_ = 0.3;

    double max_xy_accel_ = 0.6;
    double max_z_accel_ = 0.5;
    double max_yaw_accel_ = 1.0;

    double landing_descent_speed_ = 0.2;
    double landing_switch_height_ = 0.1;
    double landing_timeout_s_ = 20.0;

    // Ground guard: /mavros/extended_state landed_state + minimum height.
    bool use_landed_state_ = true;
    double landed_state_timeout_s_ = 1.0;
    int landed_state_ = 0;
    bool landed_state_valid_ = false;
    ros::Time last_landed_state_time_;

    int swa_up_max_pwm_ = 1211;
    int swa_mid_max_pwm_ = 1757;

    // ---- runtime state ----
    State state_ = State::WaitOffboard;
    Swa swa_ = Swa::Down;
    bool rc_valid_ = false;
    ros::Time last_rc_time_;

    bool offboard_ = false;
    bool offboard_prev_ = false;
    bool armed_ = false;
    bool swa_left_down_ = false;

    geometry_msgs::PoseStamped current_pos_;
    bool pose_valid_ = false;
    ros::Time last_pose_time_;
    double current_vz_ = 0.0;
    bool velocity_valid_ = false;

    ros::Time last_track_msg_;
    int target_state_ = -1;
    double target_x_offset_ = 0.0;
    double target_y_offset_ = 0.0;
    int target_id_ = -1;
    double measured_distance_m_ = 0.0;
    int distance_source_ = 0;
    bool distance_valid_ = false;
    ros::Time last_target_visible_time_;
    bool target_was_valid_ = false;

    bool hold_valid_ = false;
    double hold_x_ = 0.0;
    double hold_y_ = 0.0;
    double hold_z_ = 0.0;

    bool lost_hold_valid_ = false;
    double lost_hold_x_ = 0.0;
    double lost_hold_y_ = 0.0;
    double lost_hold_z_ = 0.0;

    ros::Time land_start_time_;
    double land_x_ = 0.0;
    double land_y_ = 0.0;
    double land_start_z_ = 0.0;
    double land_target_z_ = 0.0;
    bool auto_land_requested_ = false;
    ros::Time last_auto_land_try_;

    double cmd_vx_ = 0.0;
    double cmd_vy_ = 0.0;
    double cmd_vz_ = 0.0;
    double cmd_yaw_rate_ = 0.0;

    void loadParams();
    void initializeSubscribers();
    void initializePublishers();

    void localPosCb(const geometry_msgs::PoseStamped::ConstPtr &msg);
    void velocityCb(const geometry_msgs::TwistStamped::ConstPtr &msg);
    void rcCb(const mavros_msgs::RCIn::ConstPtr &msg);
    void trackCb(const std_msgs::Float32MultiArray::ConstPtr &msg);
    void extendedStateCb(const mavros_msgs::ExtendedState::ConstPtr &msg);
    void controlTimer(const ros::TimerEvent &event);

    bool poseFresh() const;
    bool rcFresh() const;
    bool targetFresh() const;
    bool landedStateFresh() const;
    bool landedStateBlocksTracking() const;

    void enterOffboard();
    void leaveOffboard();
    void captureHold();
    void captureLostHold();
    void enterLanding();

    void publishHold();
    void publishLostHold();
    void publishTrackCommand(double dt);
    void publishLandCommand(double dt);
    void publishPosition(double x, double y, double z);
    void publishVelocity(double vx, double vy, double vz, double yaw_rate);
    void resetCmd();

    static double slew(double current, double target, double max_rate, double dt);
    static const char *swaName(Swa swa);
    static const char *stateName(State state);
};

#endif // SAFINA_TRACK_TEST_H
