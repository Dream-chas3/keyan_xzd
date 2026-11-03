#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <ros/ros.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/PositionTarget.h>

class SimpleVelocityPID {
private:
    // PID参数
    double kp_ = 0.3;
    double ki_ = 0.0;
    double kd_ = 0.02;
    
    // 积分项
    double integral_x_ = 0;
    double integral_y_ = 0;
    double integral_z_ = 0;
    
    // 上一次误差
    double prev_error_x_ = 0;
    double prev_error_y_ = 0;
    double prev_error_z_ = 0;

public:
    SimpleVelocityPID(double kp,double ki,double kd);
    ~SimpleVelocityPID();
    /**
     * @param target_vel 目标速度
     * @param current_vel 当前速度
     * @return 控制输出速度
     */
    mavros_msgs::PositionTarget computePID(
        const mavros_msgs::PositionTarget& target_vel,
        const geometry_msgs::TwistStamped& current_vel);
    
    /**
     * @brief 重置PID积分项
     */
    void reset() ;
};
#endif
