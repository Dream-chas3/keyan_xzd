#include "acc_control.h"


SimpleVelocityACC::SimpleVelocityACC(double xyAcc,double zAcc): xyAcc_(xyAcc),zAcc_(zAcc){};

mavros_msgs::PositionTarget SimpleVelocityACC::computeACC(
        const mavros_msgs::PositionTarget& target_vel,
        const mavros_msgs::PositionTarget& current_vel) {
        
        mavros_msgs::PositionTarget output;
        
        // X方向ACC
        double error_x = target_vel.velocity.x - current_vel.velocity.x;
        if(error_x>xyAcc_){
            output.velocity.x=current_vel.velocity.x+xyAcc_;
        }
        else if(error_x<-xyAcc_){
            output.velocity.x=current_vel.velocity.x-xyAcc_;
        }
        else{
            output.velocity.x=target_vel.velocity.x;
        }
        
        // Y方向ACC
        double error_y = target_vel.velocity.y - current_vel.velocity.y;
        if(error_y>xyAcc_){
            output.velocity.y=current_vel.velocity.y+xyAcc_;
        }
        else if(error_y<-xyAcc_){
            output.velocity.y=current_vel.velocity.y-xyAcc_;
        }
        else{
            output.velocity.y=target_vel.velocity.y;
        }
        
        // Z方向ACC（注意：NED坐标系下上升为负值）
        double error_z = target_vel.velocity.z - current_vel.velocity.z;
        if(error_z>zAcc_){
            output.velocity.z=current_vel.velocity.z+zAcc_;
        }
        else if(error_z<-zAcc_){
            output.velocity.z=current_vel.velocity.z-zAcc_;
        }
        else{
            output.velocity.z=target_vel.velocity.z;
        }

        output.yaw_rate=target_vel.yaw_rate;
        
        return output;
}

SimpleVelocityACC::~SimpleVelocityACC()
{}