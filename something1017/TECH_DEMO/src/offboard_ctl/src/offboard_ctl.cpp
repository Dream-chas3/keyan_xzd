#include "offboard_ctl/takeoff_demo.h"
#include <ros/ros.h>
#include <ros/master.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/ExtendedState.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace {
double steady() {return ros::SteadyTime::now().toSec();}
bool recent(double at,double now,double timeout) {return at>=0&&now>=at&&now-at<=timeout;}
bool sourceFresh(const ros::Time& stamp) {
    const double age=(ros::Time::now()-stamp).toSec();
    return !stamp.isZero()&&age>=-0.1&&age<=0.4;
}
bool controlClear() {
    XmlRpc::XmlRpcValue args,result,payload;args[0]=ros::this_node::getName();
    if(!ros::master::execute("getSystemState",args,result,payload,false))return false;
    for(int i=0;i<payload[0].size();++i) {
        const std::string topic=payload[0][i][0];
        if(topic!="/mavros/setpoint_position/local"&&topic!="/mavros/setpoint_raw/local"&&
           topic!="/mavros/setpoint_raw/attitude"&&topic!="/mavros/setpoint_attitude/attitude"&&
           topic!="/mavros/setpoint_velocity/cmd_vel"&&topic!="/mavros/setpoint_velocity/cmd_vel_unstamped")continue;
        for(int j=0;j<payload[0][i][1].size();++j) {
            const std::string node=payload[0][i][1][j];
            if(node!=ros::this_node::getName()) {
                ROS_ERROR_THROTTLE(3,"Another controller publishes %s: %s",topic.c_str(),node.c_str());return false;
            }
        }
    }
    return true;
}
class DemoNode {
public:
    DemoNode() {
        pub_=nh_.advertise<geometry_msgs::PoseStamped>("/mavros/setpoint_position/local",1);
        state_sub_=nh_.subscribe("/mavros/state",10,&DemoNode::state,this);
        pose_sub_=nh_.subscribe("/mavros/local_position/pose",10,&DemoNode::pose,this);
        velocity_sub_=nh_.subscribe("/mavros/local_position/velocity_local",10,&DemoNode::velocity,this);
        extended_sub_=nh_.subscribe("/mavros/extended_state",10,&DemoNode::extended,this);
        arm_=nh_.serviceClient<mavros_msgs::CommandBool>("/mavros/cmd/arming");
        mode_=nh_.serviceClient<mavros_msgs::SetMode>("/mavros/set_mode");
        worker_=std::thread(&DemoNode::work,this);
        timer_=nh_.createWallTimer(ros::WallDuration(1.0/30.0),&DemoNode::tick,this);
        ROS_WARN("offboard_ctl: on ground, disarmed, select POSCTL and wait for RC gate READY. "
                 "Then manually switch to OFFBOARD: arm, rise 1 m, hover 2 s, AUTO.LAND. "
                 "Modes come only from PX4; this node never requests OFFBOARD. Startup already OFFBOARD cannot trigger.");
    }
    ~DemoNode() {
        timer_.stop();
        {std::lock_guard<std::mutex> lock(mutex_);stopping_=true;pending_=false;++generation_;}
        jobs_.notify_all();if(worker_.joinable())worker_.join();
    }
private:
    ros::NodeHandle nh_;
    ros::Subscriber state_sub_,pose_sub_,velocity_sub_,extended_sub_;
    ros::Publisher pub_;ros::ServiceClient arm_,mode_;ros::WallTimer timer_;
    mavros_msgs::State state_;mavros_msgs::ExtendedState extended_;
    geometry_msgs::PoseStamped pose_;geometry_msgs::TwistStamped velocity_;
    double state_at_=-1,pose_at_=-1,velocity_at_=-1,extended_at_=-1,clear_at_=-1;
    bool clear_=false,pose_valid_=false,velocity_valid_=false,stopping_=false,pending_=false,busy_=false;
    std::mutex mutex_;std::condition_variable jobs_;std::thread worker_;
    std::string job_;unsigned long generation_=0,job_generation_=0;
    offboard_demo::Task task_;
    offboard_demo::Input input(double now) const {
        offboard_demo::Input i;i.now=now;i.mode=state_.mode;i.armed=state_.armed;
        i.on_ground=extended_.landed_state==mavros_msgs::ExtendedState::LANDED_STATE_ON_GROUND;
        i.healthy=state_.connected&&recent(state_at_,now,1.5)&&pose_valid_&&recent(pose_at_,now,0.3)&&
            velocity_valid_&&recent(velocity_at_,now,0.3)&&recent(extended_at_,now,1.5);
        i.control_clear=clear_&&recent(clear_at_,now,1.5)&&!busy_&&!pending_;
        const auto& p=pose_.pose.position;const auto& q=pose_.pose.orientation;
        i.x=p.x;i.y=p.y;i.z=p.z;
        const double n=q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;
        i.yaw=std::atan2(2*(q.w*q.z+q.x*q.y),n-2*(q.y*q.y+q.z*q.z));
        i.vx=velocity_.twist.linear.x;i.vy=velocity_.twist.linear.y;i.vz=velocity_.twist.linear.z;
        return i;
    }
    void cancel(const char* why) {task_.cancel(why);pending_=false;++generation_;}
    void state(const mavros_msgs::State::ConstPtr& m) {
        std::lock_guard<std::mutex> lock(mutex_);state_=*m;state_at_=steady();
        if(task_.active()&&(!m->connected||(m->mode!="OFFBOARD"&&
           !(task_.phase()==offboard_demo::Phase::LANDING&&task_.landRequested()&&m->mode=="AUTO.LAND"))))
            cancel("pilot_or_fcu_left_offboard");
    }
    void extended(const mavros_msgs::ExtendedState::ConstPtr& m) {
        std::lock_guard<std::mutex> lock(mutex_);extended_=*m;extended_at_=steady();
    }
    void pose(const geometry_msgs::PoseStamped::ConstPtr& m) {
        std::lock_guard<std::mutex> lock(mutex_);pose_=*m;pose_at_=steady();
        const auto& p=m->pose.position;const auto& q=m->pose.orientation;
        const double n=q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;
        pose_valid_=std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z)&&std::isfinite(n)&&
            n>0.64&&n<1.44&&sourceFresh(m->header.stamp);
    }
    void velocity(const geometry_msgs::TwistStamped::ConstPtr& m) {
        std::lock_guard<std::mutex> lock(mutex_);velocity_=*m;velocity_at_=steady();
        const auto& v=m->twist.linear;
        velocity_valid_=std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z)&&sourceFresh(m->header.stamp);
    }
    void tick(const ros::WallTimerEvent&) {
        std::lock_guard<std::mutex> lock(mutex_);const double now=steady();
        if(!task_.active()&&now-clear_at_>=1.0) {clear_=controlClear();clear_at_=steady();}
        const auto before=task_.phase();const auto out=task_.step(input(steady()));
        if(before!=task_.phase()) {
            ROS_WARN("offboard_ctl %s -> %s: %s",offboard_demo::name(before),offboard_demo::name(task_.phase()),task_.reason().c_str());
            if(before==offboard_demo::Phase::READY||task_.phase()==offboard_demo::Phase::CANCELLED||
               task_.phase()==offboard_demo::Phase::COMPLETE)++generation_;
        }
        if(!task_.active())pending_=false;
        if(out.publish) {
            geometry_msgs::PoseStamped p;p.header.stamp=ros::Time::now();p.header.frame_id="map";
            p.pose.position.x=out.x;p.pose.position.y=out.y;p.pose.position.z=out.z;
            p.pose.orientation.z=std::sin(out.yaw/2);p.pose.orientation.w=std::cos(out.yaw/2);pub_.publish(p);
        }
        if((out.request_arm||out.request_land)&&!pending_&&!busy_) {
            job_=out.request_arm?"arm":"land";job_generation_=generation_;pending_=true;jobs_.notify_one();
        }
    }
    void work() {
        for(;;) {
            std::unique_lock<std::mutex> lock(mutex_);
            jobs_.wait(lock,[this]{return stopping_||pending_;});if(stopping_)return;
            const auto generation=job_generation_;const std::string kind=job_;pending_=false;
            const auto i=input(steady());
            if(generation!=generation_||!task_.active()||!i.healthy||i.mode!="OFFBOARD")continue;
            if(kind=="arm"&&(task_.phase()!=offboard_demo::Phase::ARMING||i.armed||!i.on_ground))continue;
            if(kind=="land"&&task_.phase()!=offboard_demo::Phase::LANDING)continue;
            busy_=true;lock.unlock();bool accepted=false;
            if(kind=="arm") {mavros_msgs::CommandBool c;c.request.value=true;accepted=arm_.call(c)&&c.response.success;}
            else {mavros_msgs::SetMode c;c.request.custom_mode="AUTO.LAND";accepted=mode_.call(c)&&c.response.mode_sent;}
            lock.lock();busy_=false;
            ROS_INFO("offboard_ctl request %s accepted=%d generation=%lu current=%lu",kind.c_str(),accepted,generation,generation_);
        }
    }
};
}
int main(int argc,char** argv) {
    ros::init(argc,argv,"offb_node");DemoNode demo;
    ros::AsyncSpinner spinner(2);spinner.start();ros::waitForShutdown();spinner.stop();return 0;
}
