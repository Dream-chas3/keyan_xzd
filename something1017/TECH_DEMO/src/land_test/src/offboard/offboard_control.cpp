#include "offboard_control.h"


OffboardController::OffboardController(ros::NodeHandle& nh) : 
    nh_(nh),
    current_state_(),
    current_pose_(),
    arming_client_(nh.serviceClient<mavros_msgs::CommandBool>("mavros/cmd/arming")),
    set_mode_client_(nh.serviceClient<mavros_msgs::SetMode>("mavros/set_mode")) {
    
    initializeSubscribers();
    initializePublishers();

    waitForConnection();
    sendInitialSetpoints();
    

}

OffboardController::~OffboardController() {
    stopOffboard();
}


void OffboardController::initializeSubscribers(){
    state_sub_ = nh_.subscribe<mavros_msgs::State>("mavros/state", 10, &OffboardController::stateCallback, this);
    local_pos_sub_ = nh_.subscribe<geometry_msgs::PoseStamped>("mavros/local_position/pose", 10, &OffboardController::localPositionCallback, this);
}
void OffboardController::initializePublishers(){
    local_pos_pub_ = nh_.advertise<geometry_msgs::PoseStamped>("mavros/setpoint_position/local", 10);

}


void OffboardController::stateCallback(const mavros_msgs::State::ConstPtr& msg) {
    
    current_state_ = *msg;
}


void OffboardController::localPositionCallback(const geometry_msgs::PoseStamped::ConstPtr& msg) {

    current_pose_ = *msg;
}

// 等待MAVROS连接
void OffboardController::waitForConnection() {
    ROS_INFO("Waiting for MAVROS connection...");
    while(ros::ok() && !current_state_.connected) {
        ros::spinOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    ROS_INFO("MAVROS connected!");
}

// 发送初始设定点
void OffboardController::sendInitialSetpoints() {
    ROS_INFO("Sending initial setpoints...");
    for(int i = 100; ros::ok() && i > 0; --i) {
        local_pos_pub_.publish(current_pose_);
        ros::spinOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

// 解锁无人机
bool OffboardController::arm() {
    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;
    
    if(arming_client_.call(arm_cmd) && arm_cmd.response.success) {
        ROS_INFO("Vehicle armed");
        return true;
    } else {
        ROS_ERROR("Arming failed");
        return false;
    }
}

// 上锁无人机
bool OffboardController::disarm() {
    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = false;
    
    if(arming_client_.call(arm_cmd) && arm_cmd.response.success) {
        ROS_INFO("Vehicle disarmed");
        return true;
    } else {
        ROS_ERROR("Disarming failed");
        return false;
    }
}

// 设置飞行模式
bool OffboardController::setMode(const std::string& mode) {
    mavros_msgs::SetMode mode_cmd;
    mode_cmd.request.custom_mode = mode;
    
    if(set_mode_client_.call(mode_cmd) && mode_cmd.response.mode_sent) {
        ROS_INFO_STREAM(mode << " mode enabled");
        return true;
    } else {
        ROS_ERROR_STREAM("Failed to set " << mode << " mode");
        return false;
    }
}

// 进入Offboard模式
bool OffboardController::startOffboard() {
    // 等待连接并发送初始设定点

    ros::Time last_request = ros::Time::now();

    while((!isOffboardMode()||!isArmed()))
    {
    	//请求进入OFFBOARD模式
        if( current_state_.mode != "OFFBOARD" && (ros::Time::now() - last_request > ros::Duration(2.0)))
        {
            if(setMode("OFFBOARD"))
            {
                ROS_INFO("Offboard enabled");
            }
           	last_request = ros::Time::now();	    
       	}
        else 
		{
			//请求解锁
			if( !current_state_.armed && (ros::Time::now() - last_request > ros::Duration(2.0)))
			{

		        if(arm())
		       	{
		            ROS_INFO("Vehicle armed");
		        }
		        last_request = ros::Time::now();	    
			}
		}

		local_pos_pub_.publish(current_pose_);

        ros::spinOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    return true;
}


// 退出Offboard模式
void OffboardController::stopOffboard() {
    setMode("POSCTL");
}



// 检查是否已解锁
bool OffboardController::isArmed() const {
    return current_state_.armed;
}

// 检查是否在Offboard模式
bool OffboardController::isOffboardMode() const {
    return current_state_.mode == "OFFBOARD";
}
