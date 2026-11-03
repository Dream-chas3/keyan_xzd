#include "pid_control.h"


SimpleVelocityPID::SimpleVelocityPID(double kp,double ki,double kd): kp_(kp),ki_(ki),kd_(kd){};

mavros_msgs::PositionTarget SimpleVelocityPID::computePID(
        const mavros_msgs::PositionTarget& target_vel,
        const geometry_msgs::TwistStamped& current_vel) {
        
        mavros_msgs::PositionTarget output;
        
        // X 方向 PID
        double error_x = target_vel.velocity.x - current_vel.twist.linear.x;
        integral_x_ += error_x;
        double derivative_x = (error_x - prev_error_x_);
        output.velocity.x = kp_ * error_x + ki_ * integral_x_ + kd_ * derivative_x;
        prev_error_x_ = error_x;

        // Y 方向 PID
        double error_y = target_vel.velocity.y - current_vel.twist.linear.y;
        integral_y_ += error_y;
        double derivative_y = (error_y - prev_error_y_);
        output.velocity.y = kp_ * error_y + ki_ * integral_y_ + kd_ * derivative_y;
        prev_error_y_ = error_y;

        // Z 方向 PID
        double error_z = target_vel.velocity.z - current_vel.twist.linear.z;
        integral_z_ += error_z;
        double derivative_z = (error_z - prev_error_z_);
        output.velocity.z = kp_ * error_z + ki_ * integral_z_ + kd_ * derivative_z;
        prev_error_z_ = error_z;

        // yaw_rate 直接用期望值
        output.yaw_rate = target_vel.yaw_rate;



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

