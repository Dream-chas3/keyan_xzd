#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <mavros_msgs/PositionTarget.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

const int demo=1;
bool isTakeOff=false;
bool isLand=false;
bool isArrived=false;
bool isAtYaw_=false;


double pos2point[3]={1,1,1};
double vel2point[3]={1,1,1};
std::vector<std::vector<double>> waypoints = {
        {0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0},
        {1.0, -1.0, 1.0}
    }; 
bool isArrivedWayPoints[3]={false,false,false};
std::vector<double> Yaws = {0.7853,-1.5707,2.3560}; 
bool isAtYaw[3] = {false,false,false};
double max_yaw_rate=0.314;

mavros_msgs::State current_state;
void state_cb(const mavros_msgs::State::ConstPtr& msg){
    current_state = *msg;
}

geometry_msgs::PoseStamped current_pos;
double current_roll=0;
double current_pitch=0;
double current_yaw=0;
void local_pos_cb(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
    current_pos = *msg;
    // 提取四元数
    tf2::Quaternion quat;
    quat.setX(msg->pose.orientation.x);
    quat.setY(msg->pose.orientation.y);
    quat.setZ(msg->pose.orientation.z);
    quat.setW(msg->pose.orientation.w);
    
    // 转换为欧拉角（弧度）
    tf2::Matrix3x3(quat).getRPY(current_roll, current_pitch, current_yaw);
}

// 检查是否到达指定位置
bool isAtPosition(float x, float y, float z, float offset) {

    return std::fabs(current_pos.pose.position.x - x) < offset &&
           std::fabs(current_pos.pose.position.y - y) < offset &&
           std::fabs(current_pos.pose.position.z - z) < offset;
}

// 检查是否到达指定yaw角
double angleDiff(double targetYaw, double currentYaw) {
    double diff = targetYaw - currentYaw;
    // 归一化到 [-pi, pi]
    while (diff > M_PI)  diff -= 2.0 * M_PI;
    while (diff < -M_PI) diff += 2.0 * M_PI;

    return diff;
}

//申请offboard模式与解锁
int main(int argc, char **argv)
{
    ros::init(argc, argv, "offb_node");
    ros::NodeHandle nh;

    //subscribe
    ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
            ("mavros/state", 10, state_cb);
    ros::Subscriber local_pos_sub = nh.subscribe<geometry_msgs::PoseStamped>
            ("/mavros/local_position/pose", 10, local_pos_cb);

    //advertise
    ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>
            ("mavros/setpoint_position/local", 10);
    ros::Publisher pub_setraw=nh.advertise<mavros_msgs::PositionTarget>
            ("/mavros/setpoint_raw/local", 10);
    
    //serviceClient
    ros::ServiceClient arming_client = nh.serviceClient<mavros_msgs::CommandBool>
            ("mavros/cmd/arming");
    ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>
            ("mavros/set_mode");

    //the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(30.0);

    // wait for FCU connection
    while(ros::ok() && !current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }

    geometry_msgs::PoseStamped pose;
    pose.pose.position.x = 0;
    pose.pose.position.y = 0;
    pose.pose.position.z = 0;

    //send a few setpoints before starting
    for(int i = 100; ros::ok() && i > 0; --i){
        local_pos_pub.publish(pose);
        ros::spinOnce();
        rate.sleep();
    }

    mavros_msgs::SetMode offb_set_mode;
    offb_set_mode.request.custom_mode = "OFFBOARD";

    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;

    ros::Time last_request = ros::Time::now();

    while(ros::ok()){
        if( current_state.mode != "OFFBOARD" &&
            (ros::Time::now() - last_request > ros::Duration(2.0))){
            if( set_mode_client.call(offb_set_mode) &&
                offb_set_mode.response.mode_sent){
                ROS_INFO("Offboard enabled");
            }
            last_request = ros::Time::now();
        } else {
            if( !current_state.armed &&
                (ros::Time::now() - last_request > ros::Duration(2.0))){
                if( arming_client.call(arm_cmd) &&
                    arm_cmd.response.success){
                    ROS_INFO("Vehicle armed");
                }
                last_request = ros::Time::now();
            }
        }

        local_pos_pub.publish(pose);

        if(current_state.mode == "OFFBOARD" && current_state.armed){
            ROS_INFO("offboard_demo 0 finish!");
            break;
        }

        ros::spinOnce();
        rate.sleep();
    }

    ros::Time last_time = ros::Time::now();
    while(ros::ok())
    {
        
        if((demo == 1))//起飞、悬停与降落
		{ 
            geometry_msgs::PoseStamped pose;
            pose.pose.position.x = 0;
            pose.pose.position.y = 0;
            pose.pose.position.z = 1.0; 
            if(!isTakeOff)
            {

                ROS_INFO("TakeOff!");
                                 
                if(!isAtPosition(pose.pose.position.x,pose.pose.position.y,pose.pose.position.z,0.1))
                {
                    local_pos_pub.publish(pose); 
                }
                else{
                    ROS_INFO("takeoff successful!");
                    isTakeOff=true;
                    last_time= ros::Time::now();
                }

            }
            else if(ros::Time::now() - last_time < ros::Duration(5.0))
            {
                ROS_INFO("Hovering!");
                local_pos_pub.publish(pose); 
            }
            else
            {
                ROS_INFO("Landing!");
                offb_set_mode.request.custom_mode = "AUTO.LAND";
                if(set_mode_client.call(offb_set_mode) && offb_set_mode.response.mode_sent)
                {
                    if(!current_state.armed){
                        ROS_INFO("Land successful!");
                        isLand=true;
                        break;
                    }

                }
            }
        }

        if((demo == 2))//位置控制
		{ 
            mavros_msgs::PositionTarget raw_pos;
            raw_pos.position.x = pos2point[0];
            raw_pos.position.y = pos2point[1];
            raw_pos.position.z = pos2point[2]; 
            raw_pos.yaw=1.57;
            raw_pos.coordinate_frame =  1;
            raw_pos.type_mask = 0b0000001111111000;

            if(!isArrived)
            {
                ROS_INFO("Goto Point!");
                                 
                if(!isAtPosition(pos2point[0],pos2point[1],pos2point[2],0.1))
                {

                    pub_setraw.publish(raw_pos);
                }
                else{
                    ROS_INFO("Arrived!");
                    isArrived=true;
                    last_time= ros::Time::now();
                }
            }
            else if(ros::Time::now() - last_time < ros::Duration(5.0))
            {
                ROS_INFO("Hovering!");
                pub_setraw.publish(raw_pos);
            }
            else
            {
                ROS_INFO("Return && Landing!");
                offb_set_mode.request.custom_mode = "AUTO.LAND";
                if(set_mode_client.call(offb_set_mode) && offb_set_mode.response.mode_sent)
                {
                    if(!current_state.armed){
                        ROS_INFO("Land successful!");
                        isLand=true;
                        break;
                    }

                }
            }
        }

        if((demo == 3))//速度控制
		{ 
            mavros_msgs::PositionTarget raw_vel;
            
            if(!isTakeOff)
            {

                ROS_INFO("TakeOff!");
                                 
                if(!isAtPosition(vel2point[0],vel2point[1],vel2point[2],0.1))
                {
                    raw_vel.velocity.x = -0.5*(current_pos.pose.position.x-vel2point[0]);
                    raw_vel.velocity.y = -0.5*(current_pos.pose.position.y-vel2point[1]);
                    raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-vel2point[2]); 
                    raw_vel.yaw_rate=0.0;
                    raw_vel.coordinate_frame =  1;
                    raw_vel.type_mask = 0b0000011111000111;
                    pub_setraw.publish(raw_vel); 
                }
                else{
                    ROS_INFO("takeoff successful!");
                    isTakeOff=true;
                    last_time= ros::Time::now();
                }

            }
            else if(!isAtYaw_)
            {
                ROS_INFO("Hovering && rotating!!");
                raw_vel.velocity.x = -3.0*(current_pos.pose.position.x-vel2point[0]);
                raw_vel.velocity.y = -3.0*(current_pos.pose.position.y-vel2point[1]);
                raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-vel2point[2]); 
                double diff = angleDiff(3.14, current_yaw);  // 弧度差
                if(fabs(diff)<0.087) isAtYaw_=true;
                double cmd = diff;
                if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                raw_vel.yaw_rate= cmd;
                raw_vel.coordinate_frame =  1;
                raw_vel.type_mask = 0b0000011111000111;
                pub_setraw.publish(raw_vel); 

            }
            else
            {
                ROS_INFO("Return && Landing!");
                offb_set_mode.request.custom_mode = "AUTO.LAND";
                if(set_mode_client.call(offb_set_mode) && offb_set_mode.response.mode_sent)
                {
                    if(!current_state.armed){
                        ROS_INFO("Land successful!");
                        isLand=true;
                        break;
                    }

                }
            }
        }

        if((demo == 4))//三角航点 
		{
            
            /*
                HOME
               * 90 *
             *        *
           *            *
         *                *
     航点B  *  *  *  *  *  航点A


            */

            mavros_msgs::PositionTarget raw_vel;            
            mavros_msgs::PositionTarget raw_pos;

            
            if(!isTakeOff)
            {

                ROS_INFO("TakeOff!");
                                 
                if(!isAtPosition(waypoints[0][0],waypoints[0][1],waypoints[0][2],0.1))
                {
                    raw_pos.position.x = waypoints[0][0];
                    raw_pos.position.y = waypoints[0][1];
                    raw_pos.position.z = waypoints[0][2]; 
                    raw_pos.yaw=0;
                    raw_pos.coordinate_frame =  1;
                    raw_pos.type_mask = 0b0000001111111000;
                    pub_setraw.publish(raw_pos); 
                }
                else{
                    ROS_INFO("takeoff successful!");
                    isTakeOff=true;

                    last_time= ros::Time::now();
                }

            }          
            else if(!isAtYaw[0]&&!isArrivedWayPoints[1])
            {
                ROS_INFO("Hovering && rotating!!");
                raw_vel.velocity.x = -3.0*(current_pos.pose.position.x-waypoints[0][0]);
                raw_vel.velocity.y = -3.0*(current_pos.pose.position.y-waypoints[0][1]);
                raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-waypoints[0][2]); 
                double diff = angleDiff(Yaws[0], current_yaw);  // 弧度差
                if(fabs(diff)<0.087) isAtYaw[0]=true;
                double cmd = diff;
                if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                raw_vel.yaw_rate= cmd;
                raw_vel.coordinate_frame =  1;
                raw_vel.type_mask = 0b0000011111000111;
                pub_setraw.publish(raw_vel); 
            }
            else if(!isArrivedWayPoints[1])
            {
                if(!isAtPosition(waypoints[1][0],waypoints[1][1],waypoints[1][2],0.1))
                {
                    ROS_INFO("Goto Point A!!");
                    raw_vel.velocity.x = -0.5*(current_pos.pose.position.x-waypoints[1][0]);
                    raw_vel.velocity.y = -0.5*(current_pos.pose.position.y-waypoints[1][1]);
                    raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-waypoints[1][2]); 
                    double diff = angleDiff(Yaws[0], current_yaw);  // 弧度差
                    double cmd = diff;
                    if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                    if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                    raw_vel.yaw_rate= cmd;

                    raw_vel.coordinate_frame =  1;
                    raw_vel.type_mask = 0b0000011111000111;
                    pub_setraw.publish(raw_vel); 
                }
                else{
                    ROS_INFO("PointA is arrived!");
                    isArrivedWayPoints[1]=true;
                    last_time= ros::Time::now();
                }

            }
            else if(!isAtYaw[1]&&!isArrivedWayPoints[2])
            {
                ROS_INFO("Hovering && rotating!!");
                raw_vel.velocity.x = -3.0*(current_pos.pose.position.x-waypoints[1][0]);
                raw_vel.velocity.y = -3.0*(current_pos.pose.position.y-waypoints[1][1]);
                raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-waypoints[1][2]); 
                double diff = angleDiff(Yaws[1], current_yaw);  // 弧度差
                if(fabs(diff)<0.087) isAtYaw[1]=true;
                double cmd = diff;
                if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                raw_vel.yaw_rate= cmd;
                raw_vel.coordinate_frame =  1;
                raw_vel.type_mask = 0b0000011111000111;
                pub_setraw.publish(raw_vel); 
            }
            else if(isArrivedWayPoints[1]&&!isArrivedWayPoints[2])
            {
                if(!isAtPosition(waypoints[2][0],waypoints[2][1],waypoints[2][2],0.1))
                {
                    ROS_INFO("Goto Point B!!");
                    raw_vel.velocity.x = -0.5*(current_pos.pose.position.x-waypoints[2][0]);
                    raw_vel.velocity.y = -0.5*(current_pos.pose.position.y-waypoints[2][1]);
                    raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-waypoints[2][2]); 
                    double diff = angleDiff(Yaws[1], current_yaw);  // 弧度差
                    double cmd = diff;
                    if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                    if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                    raw_vel.yaw_rate= cmd;

                    raw_vel.coordinate_frame =  1;
                    raw_vel.type_mask = 0b0000011111000111;
                    pub_setraw.publish(raw_vel);  
                }
                else{
                    ROS_INFO("PointB is arrived!");
                    isArrivedWayPoints[2]=true;
                    last_time= ros::Time::now();
                }

            }
            else if(!isAtYaw[2]&&!isArrivedWayPoints[0])
            {
                ROS_INFO("Hovering && rotating!!");
                raw_vel.velocity.x = -3.0*(current_pos.pose.position.x-waypoints[2][0]);
                raw_vel.velocity.y = -3.0*(current_pos.pose.position.y-waypoints[2][1]);
                raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-waypoints[2][2]); 
                double diff = angleDiff(Yaws[2], current_yaw);  // 弧度差
                if(fabs(diff)<0.087) isAtYaw[2]=true;
                double cmd = diff;
                if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                raw_vel.yaw_rate= cmd;
                raw_vel.coordinate_frame =  1;
                raw_vel.type_mask = 0b0000011111000111;
                pub_setraw.publish(raw_vel); 
            }
            else if(isArrivedWayPoints[2]&&!isArrivedWayPoints[0])
            {
                if(!isAtPosition(waypoints[0][0],waypoints[0][1],waypoints[0][2],0.1))
                {
                    ROS_INFO("Go Home!!");
                    raw_vel.velocity.x = -0.5*(current_pos.pose.position.x-waypoints[0][0]);
                    raw_vel.velocity.y = -0.5*(current_pos.pose.position.y-waypoints[0][1]);
                    raw_vel.velocity.z = -1.0*(current_pos.pose.position.z-waypoints[0][2]); 
                    double diff = angleDiff(Yaws[2], current_yaw);  // 弧度差
                    double cmd = diff;
                    if (cmd >  max_yaw_rate) cmd =  max_yaw_rate;
                    if (cmd < -max_yaw_rate) cmd = -max_yaw_rate;
                    raw_vel.yaw_rate= cmd;

                    raw_vel.coordinate_frame =  1;
                    raw_vel.type_mask = 0b0000011111000111;
                    pub_setraw.publish(raw_vel);  
                }
                else{
                    ROS_INFO("Home is arrived!");
                    isArrivedWayPoints[0]=true;
                    last_time= ros::Time::now();
                }

            }
            else
            {
                ROS_INFO("Return && Landing!");
                offb_set_mode.request.custom_mode = "AUTO.LAND";
                if(set_mode_client.call(offb_set_mode) && offb_set_mode.response.mode_sent)
                {
                    if(!current_state.armed){
                        ROS_INFO("Land successful!");
                        isLand=true;
                        break;
                    }

                }
            }
        }

        ros::spinOnce();
        rate.sleep();
    }
    return 0;
}
