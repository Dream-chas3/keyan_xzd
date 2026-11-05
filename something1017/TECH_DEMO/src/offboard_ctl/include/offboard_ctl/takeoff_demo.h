#ifndef OFFBOARD_CTL_TAKEOFF_DEMO_H
#define OFFBOARD_CTL_TAKEOFF_DEMO_H
#include <cmath>
#include <string>

namespace offboard_demo {
enum class Phase { WAITING,READY,ARMING,TAKEOFF,HOVER,LANDING,COMPLETE,CANCELLED };
inline const char* name(Phase p) {
    switch(p) {
    case Phase::WAITING:return "WAITING";case Phase::READY:return "READY";
    case Phase::ARMING:return "ARMING";case Phase::TAKEOFF:return "TAKEOFF";
    case Phase::HOVER:return "HOVER";case Phase::LANDING:return "LANDING";
    case Phase::COMPLETE:return "COMPLETE";default:return "CANCELLED";
    }
}
struct Input {
    double now=0,x=0,y=0,z=0,yaw=0,vx=0,vy=0,vz=0;
    bool healthy=false,control_clear=false,armed=false,on_ground=false;
    std::string mode;
};
struct Command {
    bool publish=false,request_arm=false,request_land=false;
    double x=0,y=0,z=0,yaw=0;
};
class Task {
public:
    Phase phase() const { return phase_; }
    bool active() const { return phase_==Phase::ARMING||phase_==Phase::TAKEOFF||phase_==Phase::HOVER||phase_==Phase::LANDING; }
    bool landRequested() const { return land_requested_; }
    const std::string& reason() const { return reason_; }
    void cancel(const std::string& why) {
        phase_=Phase::CANCELLED;reason_=why;baseline_since_=on_since_=stable_since_=-1;land_requested_=false;
    }
    Command step(const Input& i) {
        Command out;
        const double dt=i.now-last_tick_;last_tick_=i.now;
        const bool finite=std::isfinite(i.now)&&std::isfinite(i.x)&&std::isfinite(i.y)&&std::isfinite(i.z)&&
            std::isfinite(i.yaw)&&std::isfinite(i.vx)&&std::isfinite(i.vy)&&std::isfinite(i.vz);
        if(!finite||!i.healthy||(active()&&(!std::isfinite(dt)||dt<0||dt>0.25))) {
            cancel("telemetry_invalid_or_control_gap");return out;
        }
        if(!active()&&(dt<0||dt>0.25)) {
            baseline_since_=on_since_=-1;
            if(phase_==Phase::READY)phase_=Phase::WAITING;
        }
        if(active()) {
            if(i.mode!="OFFBOARD"&&!(phase_==Phase::LANDING&&land_requested_&&i.mode=="AUTO.LAND")) {
                cancel("pilot_or_fcu_left_offboard");return out;
            }
            if(phase_==Phase::LANDING&&i.mode=="AUTO.LAND") {
                if(i.on_ground&&!i.armed) { phase_=Phase::COMPLETE;reason_="landed_and_disarmed"; }
                return out; // PX4 owns AUTO.LAND; no more setpoints or services.
            }
            if(phase_!=Phase::ARMING&&!i.armed) {cancel("vehicle_disarmed");return out;}
            if(i.now-phase_since_>((phase_==Phase::TAKEOFF||phase_==Phase::HOVER)?30.0:10.0)) {
                cancel("phase_timeout");return out;
            }
        } else {
            const bool baseline=i.control_clear&&i.on_ground&&!i.armed&&i.mode=="POSCTL";
            if(baseline) {
                on_since_=-1;
                x_=i.x;y_=i.y;ground_z_=i.z;yaw_=i.yaw;
                if(baseline_since_<0)baseline_since_=i.now;
                phase_=i.now-baseline_since_>=1.2?Phase::READY:Phase::WAITING;
                reason_=phase_==Phase::READY?"RC gate READY in POSCTL":"prestream_in_posctl";
                out=hold(ground_z_);return out;
            }
            if(phase_==Phase::READY&&i.control_clear&&i.on_ground&&i.mode=="OFFBOARD") {
                if(on_since_<0)on_since_=i.now;
                if(i.now-on_since_<0.2)return hold(ground_z_);
                phase_=Phase::ARMING;phase_since_=i.now;last_request_=-1e9;
                baseline_since_=on_since_=-1;land_requested_=false;reason_="px4_offboard_edge";
            } else {
                baseline_since_=on_since_=-1;
                if(phase_==Phase::READY||phase_==Phase::WAITING)phase_=Phase::WAITING;
                return out; // Includes startup already OFFBOARD and failed/completed attempts.
            }
        }
        if(phase_==Phase::ARMING) {
            if(!i.on_ground){cancel("arming_requires_ground");return out;}
            out=hold(ground_z_);
            if(!i.armed) { if(retry(i.now))out.request_arm=true;return out; }
            phase_=Phase::TAKEOFF;phase_since_=i.now;stable_since_=-1;reason_="takeoff_one_metre";
        }
        out=hold(ground_z_+1.0);
        if(phase_==Phase::TAKEOFF||phase_==Phase::HOVER) {
            const bool settled=std::hypot(i.x-x_,i.y-y_)<0.1&&std::fabs(i.z-ground_z_-1.0)<0.1&&
                std::hypot(i.vx,i.vy)<0.1&&std::fabs(i.vz)<0.1;
            if(!settled){stable_since_=-1;phase_=Phase::TAKEOFF;}
            else {
                if(stable_since_<0)stable_since_=i.now;
                phase_=Phase::HOVER;reason_="hover_two_seconds";
                if(i.now-stable_since_>=2.0) {
                    phase_=Phase::LANDING;phase_since_=i.now;last_request_=-1e9;reason_="request_auto_land";
                }
            }
        }
        if(phase_==Phase::LANDING&&retry(i.now)) {out.request_land=true;land_requested_=true;}
        return out;
    }
private:
    Phase phase_=Phase::WAITING;
    std::string reason_="select_posctl_on_ground_then_offboard";
    double x_=0,y_=0,ground_z_=0,yaw_=0,baseline_since_=-1,on_since_=-1,stable_since_=-1;
    double phase_since_=0,last_tick_=-1,last_request_=-1e9;
    bool land_requested_=false;
    bool retry(double now) {if(now-last_request_<1.0)return false;last_request_=now;return true;}
    Command hold(double z) const {Command c;c.publish=true;c.x=x_;c.y=y_;c.z=z;c.yaw=yaw_;return c;}
};
}
#endif
