#include "track_test/Safina_track_test.h"

#include <algorithm>
#include <cmath>

Safina_Track::Safina_Track(ros::NodeHandle node_track)
    : node_(node_track),
      pnh_("~"),
      controller_(node_)
{
    loadParams();
    initializeSubscribers();
    initializePublishers();

    timer_ = node_.createTimer(ros::Duration(1.0 / control_rate_hz_),
                               &Safina_Track::controlTimer, this);

    ROS_INFO("track_test ready: control_rate=%.1f Hz require_offboard=%d",
             control_rate_hz_, require_offboard_ ? 1 : 0);
    ROS_INFO("The node never arms and never requests OFFBOARD; the pilot does that.");
    ROS_INFO("SWA mapping: up<=%d pwm -> protection, <=%d -> follow, above -> land",
             swa_up_max_pwm_, swa_mid_max_pwm_);
}

Safina_Track::~Safina_Track() {}

void Safina_Track::loadParams()
{
    pnh_.param<double>("control_rate_hz", control_rate_hz_, 20.0);
    pnh_.param<bool>("require_offboard", require_offboard_, true);
    pnh_.param<double>("target_timeout_s", target_timeout_s_, 0.5);
    pnh_.param<double>("target_loss_grace_s", target_loss_grace_s_, 0.3);
    pnh_.param<double>("pose_timeout_s", pose_timeout_s_, 0.5);
    pnh_.param<double>("rc_timeout_s", rc_timeout_s_, 1.0);

    pnh_.param<double>("tracking_vlxy", tracking_vlxy_, 0.8);
    pnh_.param<double>("max_forward_speed", max_forward_speed_, 0.3);
    pnh_.param<double>("max_backward_speed", max_backward_speed_, 0.15);
    pnh_.param<double>("tracking_ryaw", tracking_ryaw_, 1.5);
    pnh_.param<double>("max_yaw_rate", max_yaw_rate_, 1.2);
    pnh_.param<double>("min_tracking_height_m", min_tracking_height_m_, 0.3);

    pnh_.param<bool>("use_metric_distance", use_metric_distance_, true);
    pnh_.param<double>("target_distance_m", target_distance_m_, 3.0);
    pnh_.param<double>("distance_kp", distance_kp_, 0.5);
    pnh_.param<double>("distance_deadband_m", distance_deadband_m_, 0.2);

    pnh_.param<double>("altitude_kp", altitude_kp_, 0.8);
    pnh_.param<double>("altitude_kd", altitude_kd_, 0.2);
    pnh_.param<double>("altitude_offset_m", altitude_offset_m_, 0.0);
    pnh_.param<double>("max_climb_speed", max_climb_speed_, 0.4);
    pnh_.param<double>("max_descent_speed", max_descent_speed_, 0.3);

    pnh_.param<double>("max_xy_accel", max_xy_accel_, 0.6);
    pnh_.param<double>("max_z_accel", max_z_accel_, 0.5);
    pnh_.param<double>("max_yaw_accel", max_yaw_accel_, 1.0);

    pnh_.param<double>("landing_descent_speed", landing_descent_speed_, 0.2);
    pnh_.param<double>("landing_switch_height", landing_switch_height_, 0.1);
    pnh_.param<double>("landing_timeout_s", landing_timeout_s_, 20.0);

    pnh_.param<bool>("use_landed_state", use_landed_state_, true);
    pnh_.param<double>("landed_state_timeout_s", landed_state_timeout_s_, 1.0);

    pnh_.param<int>("swa/up_max_pwm", swa_up_max_pwm_, 1211);
    pnh_.param<int>("swa/mid_max_pwm", swa_mid_max_pwm_, 1757);

    // ---- validation ----
    if (!std::isfinite(control_rate_hz_) || control_rate_hz_ < 2.0)
        control_rate_hz_ = 20.0;
    if (target_timeout_s_ <= 0.0)
        target_timeout_s_ = 0.5;
    if (target_loss_grace_s_ < 0.0)
        target_loss_grace_s_ = 0.0;
    if (pose_timeout_s_ <= 0.0)
        pose_timeout_s_ = 0.5;
    if (rc_timeout_s_ <= 0.0)
        rc_timeout_s_ = 1.0;
    if (max_forward_speed_ < 0.0)
        max_forward_speed_ = 0.0;
    if (max_backward_speed_ < 0.0)
        max_backward_speed_ = 0.0;
    if (max_yaw_rate_ < 0.0)
        max_yaw_rate_ = 0.0;
    if (target_distance_m_ < 0.5)
        target_distance_m_ = 0.5;
    if (distance_kp_ <= 0.0)
        distance_kp_ = 0.5;
    if (distance_deadband_m_ < 0.0)
        distance_deadband_m_ = 0.0;
    if (max_climb_speed_ < 0.0)
        max_climb_speed_ = 0.0;
    if (max_descent_speed_ < 0.0)
        max_descent_speed_ = 0.0;
    if (landing_descent_speed_ <= 0.0)
        landing_descent_speed_ = 0.2;
    if (landing_switch_height_ < 0.0)
        landing_switch_height_ = 0.1;
    if (landing_timeout_s_ <= 0.0)
        landing_timeout_s_ = 20.0;
    if (landed_state_timeout_s_ <= 0.0)
        landed_state_timeout_s_ = 1.0;
    if (swa_up_max_pwm_ >= swa_mid_max_pwm_)
    {
        ROS_WARN("SWA thresholds invalid (up_max=%d mid_max=%d), using 1211/1757",
                 swa_up_max_pwm_, swa_mid_max_pwm_);
        swa_up_max_pwm_ = 1211;
        swa_mid_max_pwm_ = 1757;
    }
}

void Safina_Track::initializeSubscribers()
{
    sub_local_pos_ = node_.subscribe("/mavros/local_position/pose", 10,
                                     &Safina_Track::localPosCb, this);
    sub_velocity_ = node_.subscribe("/mavros/local_position/velocity_local", 10,
                                    &Safina_Track::velocityCb, this);
    sub_rc_ = node_.subscribe("/mavros/rc/in", 10, &Safina_Track::rcCb, this);
    sub_track_ = node_.subscribe("/Safina_track", 1, &Safina_Track::trackCb, this);
    sub_extended_state_ = node_.subscribe("/mavros/extended_state", 10,
                                          &Safina_Track::extendedStateCb, this);
}

void Safina_Track::initializePublishers()
{
    pub_setraw_ = node_.advertise<mavros_msgs::PositionTarget>("/mavros/setpoint_raw/local", 10);
}

void Safina_Track::localPosCb(const geometry_msgs::PoseStamped::ConstPtr &msg)
{
    current_pos_ = *msg;
    pose_valid_ = true;
    last_pose_time_ = ros::Time::now();
}

void Safina_Track::velocityCb(const geometry_msgs::TwistStamped::ConstPtr &msg)
{
    current_vz_ = msg->twist.linear.z;  // ENU: positive up
    velocity_valid_ = true;
}

void Safina_Track::rcCb(const mavros_msgs::RCIn::ConstPtr &msg)
{
    if (msg->channels.size() <= 8)
        return;

    const int pwm = msg->channels[8];
    if (pwm <= swa_up_max_pwm_)
        swa_ = Swa::Up;
    else if (pwm <= swa_mid_max_pwm_)
        swa_ = Swa::Mid;
    else
        swa_ = Swa::Down;

    rc_valid_ = true;
    last_rc_time_ = ros::Time::now();
}

void Safina_Track::trackCb(const std_msgs::Float32MultiArray::ConstPtr &msg)
{
    last_track_msg_ = ros::Time::now();
    if (msg->data.size() >= 3)
    {
        target_state_ = static_cast<int>(msg->data[0]);
        target_x_offset_ = msg->data[1];
        target_y_offset_ = msg->data[2];
    }
    if (msg->data.size() >= 4)
        target_id_ = static_cast<int>(msg->data[3]);
    if (msg->data.size() >= 7)
    {
        measured_distance_m_ = msg->data[4];
        distance_source_ = static_cast<int>(msg->data[5]);
        distance_valid_ = msg->data[6] > 0.5f;
    }
    else
    {
        measured_distance_m_ = 0.0;
        distance_source_ = 0;
        distance_valid_ = false;
    }
}

void Safina_Track::extendedStateCb(const mavros_msgs::ExtendedState::ConstPtr &msg)
{
    landed_state_ = msg->landed_state;
    landed_state_valid_ = true;
    last_landed_state_time_ = ros::Time::now();
}

bool Safina_Track::poseFresh() const
{
    return pose_valid_ && (ros::Time::now() - last_pose_time_).toSec() <= pose_timeout_s_;
}

bool Safina_Track::rcFresh() const
{
    return rc_valid_ && (ros::Time::now() - last_rc_time_).toSec() <= rc_timeout_s_;
}

bool Safina_Track::targetFresh() const
{
    return !last_track_msg_.isZero() &&
           (ros::Time::now() - last_track_msg_).toSec() <= target_timeout_s_ &&
           target_state_ == 0;
}

bool Safina_Track::landedStateFresh() const
{
    return landed_state_valid_ &&
           (ros::Time::now() - last_landed_state_time_).toSec() <= landed_state_timeout_s_;
}

bool Safina_Track::landedStateBlocksTracking() const
{
    if (!use_landed_state_ || !landedStateFresh())
        return false;
    return landed_state_ == mavros_msgs::ExtendedState::LANDED_STATE_ON_GROUND ||
           landed_state_ == mavros_msgs::ExtendedState::LANDED_STATE_LANDING;
}

void Safina_Track::controlTimer(const ros::TimerEvent &event)
{
    double dt = (event.current_real - event.last_real).toSec();
    if (!std::isfinite(dt) || dt <= 0.0 || dt > 0.5)
        dt = 1.0 / control_rate_hz_;

    offboard_ = controller_.isOffboardMode();
    if (!require_offboard_)
        offboard_ = true;

    const bool new_armed = controller_.isArmed();
    if (new_armed != armed_)
    {
        armed_ = new_armed;
        ROS_INFO("PX4 armed=%d", armed_ ? 1 : 0);
    }

    if (offboard_ != offboard_prev_)
    {
        if (offboard_)
            enterOffboard();
        else
            leaveOffboard();
        offboard_prev_ = offboard_;
    }

    if (!offboard_)
    {
        // Requirement: if the RC is not in OFFBOARD, never execute SWA
        // actions. We only keep a neutral position setpoint stream alive so
        // that PX4 accepts the pilot's switch into OFFBOARD. The pilot has
        // full control in Position/Altitude/Stabilized modes.
        state_ = State::WaitOffboard;
        hold_valid_ = false;
        lost_hold_valid_ = false;
        target_was_valid_ = false;
        swa_left_down_ = false;
        resetCmd();

        if (poseFresh())
            publishPosition(current_pos_.pose.position.x,
                            current_pos_.pose.position.y,
                            current_pos_.pose.position.z);
        else
            publishVelocity(0.0, 0.0, 0.0, 0.0);
        return;
    }

    const bool rc_fresh = rcFresh();
    if (rc_fresh && swa_ != Swa::Down)
        swa_left_down_ = true;

    switch (state_)
    {
    case State::WaitOffboard:
        enterOffboard();
        break;

    case State::Hold:
        if (!rc_fresh)
        {
            ROS_WARN_THROTTLE(1.0, "RC data stale: holding, SWA ignored");
            captureHold();
            publishHold();
            break;
        }

        if (swa_ == Swa::Mid)
        {
            if (!poseFresh())
            {
                ROS_WARN_THROTTLE(1.0, "SWA mid but pose stale: holding");
                publishHold();
            }
            else if (landedStateBlocksTracking())
            {
                ROS_WARN_THROTTLE(2.0, "SWA mid but landed_state=%d (on ground/landing): holding",
                                  landed_state_);
                publishHold();
            }
            else if (current_pos_.pose.position.z < min_tracking_height_m_)
            {
                ROS_WARN_THROTTLE(2.0, "SWA mid but z=%.2f < min_tracking_height=%.2f: holding, take off first",
                                  current_pos_.pose.position.z, min_tracking_height_m_);
                publishHold();
            }
            else
            {
                state_ = State::Track;
                lost_hold_valid_ = false;
                target_was_valid_ = false;
                resetCmd();
                ROS_INFO("SWA mid: tracking enabled (z=%.2f)",
                         current_pos_.pose.position.z);
                publishHold();
            }
        }
        else if (swa_ == Swa::Down && swa_left_down_)
        {
            enterLanding();
        }
        else
        {
            publishHold();
            if (swa_ == Swa::Up)
                ROS_WARN_THROTTLE(2.0, "SWA up in OFFBOARD: protection hold "
                                       "(no takeoff, no altitude change)");
        }
        break;

    case State::Track:
        if (!rc_fresh)
        {
            ROS_WARN_THROTTLE(1.0, "RC data stale: braking and holding");
            captureHold();
            resetCmd();
            publishHold();
            break;
        }

        if (landedStateBlocksTracking())
        {
            captureHold();
            state_ = State::Hold;
            resetCmd();
            publishHold();
            ROS_WARN("landed_state=%d while tracking: stopping tracking and holding", landed_state_);
            break;
        }

        if (swa_ == Swa::Up)
        {
            captureHold();
            state_ = State::Hold;
            resetCmd();
            publishHold();
            ROS_WARN("SWA up: protection hold");
        }
        else if (swa_ == Swa::Down && swa_left_down_)
        {
            enterLanding();
        }
        else if (targetFresh())
        {
            last_target_visible_time_ = ros::Time::now();
            target_was_valid_ = true;
            lost_hold_valid_ = false;
            publishTrackCommand(dt);
        }
        else
        {
            const bool in_grace = target_was_valid_ &&
                                  !last_target_visible_time_.isZero() &&
                                  (ros::Time::now() - last_target_visible_time_).toSec() <
                                      target_loss_grace_s_;
            if (in_grace)
            {
                cmd_vx_ = slew(cmd_vx_, 0.0, max_xy_accel_, dt);
                cmd_vz_ = slew(cmd_vz_, 0.0, max_z_accel_, dt);
                cmd_yaw_rate_ = slew(cmd_yaw_rate_, 0.0, max_yaw_accel_, dt);
                cmd_vy_ = 0.0;
                publishVelocity(cmd_vx_, 0.0, cmd_vz_, cmd_yaw_rate_);
            }
            else
            {
                if (target_was_valid_ || !lost_hold_valid_)
                    captureLostHold();
                target_was_valid_ = false;
                publishLostHold();
                ROS_WARN_THROTTLE(1.0, "No primary target: holding position");
            }
        }
        break;

    case State::Land:
        publishLandCommand(dt);
        break;
    }
}

void Safina_Track::enterOffboard()
{
    state_ = State::Hold;
    hold_valid_ = false;
    captureHold();
    lost_hold_valid_ = false;
    target_was_valid_ = false;
    swa_left_down_ = rcFresh() && (swa_ != Swa::Down);
    resetCmd();

    ROS_INFO("OFFBOARD entered: holding at (%.2f, %.2f, %.2f), SWA=%s",
             hold_x_, hold_y_, hold_z_, swaName(swa_));
}

void Safina_Track::leaveOffboard()
{
    state_ = State::WaitOffboard;
    hold_valid_ = false;
    lost_hold_valid_ = false;
    target_was_valid_ = false;
    swa_left_down_ = false;
    resetCmd();

    ROS_INFO("Left OFFBOARD: SWA actions disabled, pilot has control");
}

void Safina_Track::captureHold()
{
    if (!poseFresh())
        return;

    hold_x_ = current_pos_.pose.position.x;
    hold_y_ = current_pos_.pose.position.y;
    hold_z_ = current_pos_.pose.position.z;
    hold_valid_ = true;
}

void Safina_Track::captureLostHold()
{
    if (!poseFresh())
        return;

    lost_hold_x_ = current_pos_.pose.position.x;
    lost_hold_y_ = current_pos_.pose.position.y;
    lost_hold_z_ = current_pos_.pose.position.z;
    lost_hold_valid_ = true;
}

void Safina_Track::enterLanding()
{
    state_ = State::Land;
    land_start_time_ = ros::Time::now();
    auto_land_requested_ = false;
    last_auto_land_try_ = ros::Time(0);
    resetCmd();

    if (poseFresh())
    {
        land_x_ = current_pos_.pose.position.x;
        land_y_ = current_pos_.pose.position.y;
        land_start_z_ = current_pos_.pose.position.z;
    }
    else if (hold_valid_)
    {
        land_x_ = hold_x_;
        land_y_ = hold_y_;
        land_start_z_ = hold_z_;
    }
    else
    {
        land_x_ = 0.0;
        land_y_ = 0.0;
        land_start_z_ = 0.0;
        ROS_WARN("Landing requested without a valid pose; XY hold unavailable");
    }
    land_target_z_ = land_start_z_;

    ROS_INFO("Landing in place at (%.2f, %.2f), z=%.2f", land_x_, land_y_, land_start_z_);
}

void Safina_Track::publishHold()
{
    if (!hold_valid_)
        captureHold();

    if (hold_valid_)
        publishPosition(hold_x_, hold_y_, hold_z_);
    else
        publishVelocity(0.0, 0.0, 0.0, 0.0);
}

void Safina_Track::publishLostHold()
{
    if (!lost_hold_valid_)
        captureLostHold();

    if (lost_hold_valid_)
        publishPosition(lost_hold_x_, lost_hold_y_, lost_hold_z_);
    else
        publishVelocity(0.0, 0.0, 0.0, 0.0);
}

void Safina_Track::publishTrackCommand(double dt)
{
    if (!poseFresh())
    {
        ROS_WARN_THROTTLE(1.0, "Pose stale: braking");
        cmd_vx_ = slew(cmd_vx_, 0.0, max_xy_accel_, dt);
        cmd_vz_ = slew(cmd_vz_, 0.0, max_z_accel_, dt);
        cmd_yaw_rate_ = slew(cmd_yaw_rate_, 0.0, max_yaw_accel_, dt);
        cmd_vy_ = 0.0;
        publishVelocity(cmd_vx_, 0.0, cmd_vz_, cmd_yaw_rate_);
        return;
    }

    if (!hold_valid_)
        captureHold();

    const double current_z = current_pos_.pose.position.z;
    const double target_z = (hold_valid_ ? hold_z_ : current_z) + altitude_offset_m_;

    double vz = altitude_kp_ * (target_z - current_z);
    if (velocity_valid_)
        vz -= altitude_kd_ * current_vz_;
    vz = std::max(-max_descent_speed_, std::min(max_climb_speed_, vz));

    double vx = 0.0;
    if (use_metric_distance_ && distance_valid_ && measured_distance_m_ > 0.0)
    {
        const double err = measured_distance_m_ - target_distance_m_;
        if (std::fabs(err) <= distance_deadband_m_)
            vx = 0.0;
        else
            vx = distance_kp_ * err;
        vx = std::max(-max_backward_speed_, std::min(max_forward_speed_, vx));
    }
    else
    {
        // Fallback: original pixel-radial heuristic.
        vx = tracking_vlxy_ * target_x_offset_;
        vx = std::max(-max_backward_speed_, std::min(max_forward_speed_, vx));
    }

    double yaw_rate = tracking_ryaw_ * target_y_offset_;
    yaw_rate = std::max(-max_yaw_rate_, std::min(max_yaw_rate_, yaw_rate));

    ROS_INFO_THROTTLE(2.0,
        "tracking distance=%.2f m target=%.2f m source=%d valid=%d vx=%.2f",
        measured_distance_m_, target_distance_m_, distance_source_,
        distance_valid_ ? 1 : 0, vx);

    cmd_vx_ = slew(cmd_vx_, vx, max_xy_accel_, dt);
    cmd_vz_ = slew(cmd_vz_, vz, max_z_accel_, dt);
    cmd_yaw_rate_ = slew(cmd_yaw_rate_, yaw_rate, max_yaw_accel_, dt);
    cmd_vy_ = 0.0;

    publishVelocity(cmd_vx_, 0.0, cmd_vz_, cmd_yaw_rate_);
}

void Safina_Track::publishLandCommand(double dt)
{
    if (!poseFresh())
    {
        ROS_WARN_THROTTLE(1.0, "Pose stale during landing: velocity descent only");
        cmd_vx_ = slew(cmd_vx_, 0.0, max_xy_accel_, dt);
        cmd_vz_ = slew(cmd_vz_, -landing_descent_speed_, max_z_accel_, dt);
        cmd_yaw_rate_ = slew(cmd_yaw_rate_, 0.0, max_yaw_accel_, dt);
        cmd_vy_ = 0.0;
        publishVelocity(cmd_vx_, 0.0, cmd_vz_, cmd_yaw_rate_);
        return;
    }

    const double current_z = current_pos_.pose.position.z;

    // Position setpoint holds X/Y exactly and ramps Z down. The target is
    // clamped to stay a little below the measured height so PX4 keeps
    // descending even if it lags behind the ramp.
    land_target_z_ -= landing_descent_speed_ * dt;
    const double lead_z = current_z - 0.05;
    if (land_target_z_ > lead_z)
        land_target_z_ = lead_z;
    if (land_target_z_ < 0.03)
        land_target_z_ = 0.03;

    publishPosition(land_x_, land_y_, land_target_z_);

    const double elapsed = (ros::Time::now() - land_start_time_).toSec();
    if (current_z <= landing_switch_height_ || elapsed >= landing_timeout_s_)
    {
        const ros::Time now = ros::Time::now();
        if (!auto_land_requested_ || (now - last_auto_land_try_).toSec() >= 1.0)
        {
            last_auto_land_try_ = now;
            if (controller_.setMode("AUTO.LAND"))
            {
                auto_land_requested_ = true;
                ROS_INFO("AUTO.LAND requested at z=%.2f", current_z);
            }
            else
            {
                ROS_WARN_THROTTLE(1.0, "AUTO.LAND request failed, holding at z=%.2f", current_z);
            }
        }
    }
}

void Safina_Track::publishPosition(double x, double y, double z)
{
    mavros_msgs::PositionTarget cmd;
    cmd.header.stamp = ros::Time::now();
    // mavros interprets FRAME_LOCAL_NED as a local ENU setpoint and converts
    // it to NED for PX4. Keep using this convention (same as land_test).
    cmd.coordinate_frame = mavros_msgs::PositionTarget::FRAME_LOCAL_NED;
    cmd.type_mask = mavros_msgs::PositionTarget::IGNORE_VX |
                    mavros_msgs::PositionTarget::IGNORE_VY |
                    mavros_msgs::PositionTarget::IGNORE_VZ |
                    mavros_msgs::PositionTarget::IGNORE_AFX |
                    mavros_msgs::PositionTarget::IGNORE_AFY |
                    mavros_msgs::PositionTarget::IGNORE_AFZ |
                    mavros_msgs::PositionTarget::FORCE |
                    mavros_msgs::PositionTarget::IGNORE_YAW |
                    mavros_msgs::PositionTarget::IGNORE_YAW_RATE;
    cmd.position.x = x;
    cmd.position.y = y;
    cmd.position.z = z;
    cmd.yaw = 0.0;
    cmd.yaw_rate = 0.0;
    pub_setraw_.publish(cmd);
}

void Safina_Track::publishVelocity(double vx, double vy, double vz, double yaw_rate)
{
    mavros_msgs::PositionTarget cmd;
    cmd.header.stamp = ros::Time::now();
    // mavros treats FRAME_BODY_NED as body FLU and converts it to FRD for
    // PX4, so +x is forward, +y is left, +z is up, and +yaw_rate is CCW.
    cmd.coordinate_frame = mavros_msgs::PositionTarget::FRAME_BODY_NED;
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
    cmd.yaw = 0.0;
    cmd.yaw_rate = yaw_rate;
    pub_setraw_.publish(cmd);
}

void Safina_Track::resetCmd()
{
    cmd_vx_ = 0.0;
    cmd_vy_ = 0.0;
    cmd_vz_ = 0.0;
    cmd_yaw_rate_ = 0.0;
}

double Safina_Track::slew(double current, double target, double max_rate, double dt)
{
    if (max_rate <= 0.0 || dt <= 0.0)
        return target;

    const double max_delta = max_rate * dt;
    if (target > current + max_delta)
        return current + max_delta;
    if (target < current - max_delta)
        return current - max_delta;
    return target;
}

const char *Safina_Track::swaName(Swa swa)
{
    switch (swa)
    {
    case Swa::Up:
        return "up(protection)";
    case Swa::Mid:
        return "mid(follow)";
    case Swa::Down:
        return "down(land/hold)";
    }
    return "unknown";
}

const char *Safina_Track::stateName(State state)
{
    switch (state)
    {
    case State::WaitOffboard:
        return "WAIT_OFFBOARD";
    case State::Hold:
        return "HOLD";
    case State::Track:
        return "TRACK";
    case State::Land:
        return "LAND";
    }
    return "unknown";
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "track");

    ros::NodeHandle node_track;

    Safina_Track safina_track(node_track);

    ROS_INFO("track_test running ...");

    ros::spin();
    return 0;
}
