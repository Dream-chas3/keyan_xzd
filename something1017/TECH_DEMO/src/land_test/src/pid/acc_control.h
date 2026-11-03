#ifndef ACC_CONTROLLER_H
#define ACC_CONTROLLER_H

#include <ros/ros.h>
#include <mavros_msgs/PositionTarget.h>

class SimpleVelocityACC {
private:
    // ACC参数
    double xyAcc_ = 0.020;
    double zAcc_ = 0.020;
    double yawAcc_= 0.0020;  
    

public:
    SimpleVelocityACC(double xyAcc_,double zAcc_,double yawAcc_);
    ~SimpleVelocityACC();

    mavros_msgs::PositionTarget computeACC(
        const mavros_msgs::PositionTarget& target_vel,
        const mavros_msgs::PositionTarget& current_vel);
    
};
#endif
