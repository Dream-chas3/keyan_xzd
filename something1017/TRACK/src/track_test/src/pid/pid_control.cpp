#include "pid_control.h"


SimpleVelocityPID::SimpleVelocityPID(double kp,double ki,double kd): kp_(kp),ki_(ki),kd_(kd){};

geometry_msgs::TwistStamped SimpleVelocityPID::computePID(
        const geometry_msgs::TwistStamped& target_vel,
        const geometry_msgs::TwistStamped& current_vel) {
        
        geometry_msgs::TwistStamped output;
        
        // X方向PID
        double error_x = target_vel.twist.linear.x - current_vel.twist.linear.x;
        integral_x_ += error_x;
        double derivative_x = (error_x - prev_error_x_);
        output.twist.linear.x = kp_ * error_x + ki_ * integral_x_ + kd_ * derivative_x;
        prev_error_x_ = error_x;
        
        // Y方向PID
        double error_y = target_vel.twist.linear.y - current_vel.twist.linear.y;
        integral_y_ += error_y ;
        double derivative_y = (error_y - prev_error_y_);
        output.twist.linear.y = kp_ * error_y + ki_ * integral_y_ + kd_ * derivative_y;
        prev_error_y_ = error_y;
        
        // Z方向PID（注意：NED坐标系下上升为负值）
        double error_z = target_vel.twist.linear.z - current_vel.twist.linear.z;
        integral_z_ += error_z ;
        double derivative_z = (error_z - prev_error_z_);
        output.twist.linear.z = kp_ * error_z + ki_ * integral_z_ + kd_ * derivative_z;
        prev_error_z_ = error_z;

        output.twist.angular.z=target_vel.twist.angular.z;
        
        return output;
}
void SimpleVelocityPID::reset() {
    integral_x_ = 0;
    integral_y_ = 0;
    integral_z_ = 0;
    prev_error_x_ = 0;
    prev_error_y_ = 0;
    prev_error_z_ = 0;
}
SimpleVelocityPID::~SimpleVelocityPID()
{
    reset();

}