#ifndef OFFBOARD_CONTROL_H
#define OFFBOARD_CONTROL_H

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>

#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <thread>
#include <chrono>


class OffboardController {
public:

    explicit OffboardController(ros::NodeHandle& nh);
    
    ~OffboardController();

    // 模式设置
    bool arm();
    bool disarm();
    bool setMode(const std::string& mode);
    bool startOffboard();
    void stopOffboard();

    // 辅助函数
    void waitForConnection();
    void sendInitialSetpoints();


    
    // 状态检查
    bool isArmed() const;
    bool isOffboardMode() const;
    bool isAutoLandMode() const;
    
    //初始化
    mavros_msgs::State current_state_;
    geometry_msgs::PoseStamped current_pose_;


private:
    // ROS通信成员
    ros::NodeHandle& nh_;
    ros::Subscriber state_sub_;
    ros::Subscriber local_pos_sub_;
    ros::Publisher local_pos_pub_;


    ros::ServiceClient arming_client_;
    ros::ServiceClient set_mode_client_;

    void initializeSubscribers();
    void initializePublishers();

    // 回调函数
    void stateCallback(const mavros_msgs::State::ConstPtr& msg);
    void localPositionCallback(const geometry_msgs::PoseStamped::ConstPtr& msg);
    
};

#endif // OFFBOARD_CONTROL_H