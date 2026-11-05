#ifndef SAFINA_LAND_TEST_H
#define SAFINA_LAND_TEST_H
#include "land_test/landing_task.h"
#include "land_test/mode_start_gate.h"
#include "gcu/gcu_control.h"
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/ExtendedState.h>
#include <mavros_msgs/PositionTarget.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/CommandBool.h>
#include <std_srvs/SetBool.h>
#include <std_msgs/String.h>
#include <nav_msgs/Path.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>
#include <memory>

class Safina_land {
public:
    explicit Safina_land(ros::NodeHandle node);
    ~Safina_land();
private:
    ros::NodeHandle node_;
    ros::Subscriber state_sub_,pose_sub_,velocity_sub_,extended_sub_,marker_sub_;
    ros::Publisher command_pub_,diagnostic_pub_,header_pub_,goal_pub_,target_pub_,path_pub_;
    ros::ServiceServer enable_service_;
    ros::ServiceClient mode_client_,arm_client_;
    ros::WallTimer timer_;
    land_task::Config config_;
    std::unique_ptr<land_task::Task> task_;
    land_task::ModeStartGate mode_gate_;
    std::string start_trigger_="px4_mode";
    bool standby_clear_=false;
    double standby_check_at_=-1,mode_tick_at_=-1;
    std::mutex mutex_;
    std::condition_variable jobs_;
    std::thread worker_;
    bool stopping_=false,pending_=false,worker_busy_=false,gimbal_ready_=false;
    std::string job_,job_mode_;
    unsigned long generation_=0,job_generation_=0;
    mavros_msgs::State state_;
    mavros_msgs::ExtendedState extended_;
    geometry_msgs::PoseStamped pose_;
    geometry_msgs::TwistStamped velocity_;
    struct TimedPose { geometry_msgs::PoseStamped message; double received; };
    std::deque<TimedPose> poses_;
    double state_at_=-1,pose_at_=-1,velocity_at_=-1,extended_at_=-1,marker_rx_at_=-1;
    double marker_valid_at_=-1,target_x_=0,target_y_=0,target_body_x_=0,target_body_y_=0,marker_yaw_=0,marker_distance_=-1;
    std::string marker_reason_="no_marker_messages",diagnostic_header_;
    double marker_source_age_=-1,marker_pair_dt_=-1,marker_jump_=0,target_z_=0;
    unsigned long marker_received_=0,marker_accepted_=0;
    bool pose_valid_=false,velocity_valid_=false,marker_valid_=false,target_filter_ready_=false;
    double yaw_=0,yaw_rate_=0,last_pose_yaw_=0,last_pose_time_=-1;
    double state_timeout_=1.5,pose_timeout_=0.3,velocity_timeout_=0.3,extended_timeout_=1.5;
    double source_stamp_timeout_=0.4,pose_image_tolerance_=0.15,target_alpha_=0.4,image_latency_=0.0;
    double max_marker_range_=10.0,max_target_jump_=0.5;
    double camera_offset_x_=0,camera_offset_y_=0,camera_offset_z_=0;
    bool command_gimbal_down_=true,gimbal_down_on_startup_=true;
    std::string gimbal_status_="not_requested";
    std::string gimbal_ip_="192.168.144.108",diagnostic_path_;
    int gimbal_port_=2332;
    double diagnostic_hz_=5,last_diagnostic_=-1;
    std::ofstream diagnostic_file_;
    land_task::Phase last_phase_=land_task::Phase::IDLE;
    void loadParameters();
    void openDiagnostics();
    land_task::Input input(double now) const;
    bool enable(std_srvs::SetBool::Request&,std_srvs::SetBool::Response&);
    void tick(const ros::WallTimerEvent&);
    void stateCallback(const mavros_msgs::State::ConstPtr&);
    void poseCallback(const geometry_msgs::PoseStamped::ConstPtr&);
    void velocityCallback(const geometry_msgs::TwistStamped::ConstPtr&);
    void markerCallback(const geometry_msgs::PoseStamped::ConstPtr&);
    void extendedCallback(const mavros_msgs::ExtendedState::ConstPtr&);
    bool startTask(double now,std::string& reason);
    void queueJob(const std::string&,double now);
    void workerLoop();
    void diagnostics(const land_task::Input&,const land_task::Command&);
};
#endif
