#ifndef LAND_TEST_RC_START_GATE_H
#define LAND_TEST_RC_START_GATE_H
#include <algorithm>
#include <cmath>

namespace land_task {
// PX4 v1.16 RC calibration and six-position selector, applied to raw RCIn PWM.
// Defaults are the values read from this aircraft, not a guessed auxiliary channel.
struct RcConfig {
    int channel=5,offboard_slot=4;
    double minimum=1000,trim=1500,maximum=2000,deadzone=10;
    bool reversed=false;
    double timeout=0.5,off_hold_seconds=0.3,on_hold_seconds=0.2;
    bool valid() const {
        return channel>=1&&channel<=18&&offboard_slot>=1&&offboard_slot<=6&&
            std::isfinite(minimum)&&std::isfinite(trim)&&std::isfinite(maximum)&&
            std::isfinite(deadzone)&&deadzone>=0&&minimum<trim-deadzone&&trim+deadzone<maximum&&
            std::isfinite(timeout)&&timeout>0&&std::isfinite(off_hold_seconds)&&off_hold_seconds>0&&
            std::isfinite(on_hold_seconds)&&on_hold_seconds>0;
    }
    int slot(unsigned pwm) const {
        if(!valid()||pwm<500||pwm>2500)return 0;
        float normalized=0;
        if(pwm<trim-deadzone)normalized=std::max(-1.0f,float((double(pwm)-(trim-deadzone))/(trim-deadzone-minimum)));
        else if(pwm>trim+deadzone)normalized=std::min(1.0f,float((double(pwm)-(trim+deadzone))/(maximum-trim-deadzone)));
        if(reversed)normalized=-normalized;
        // Same six-slot boundaries as PX4 RCUpdate (including endpoint margins).
        const float half=1.0f/6.0f;
        const float index=((normalized+1.05f)*6.0f+half)/2.1f+half+1.0f;
        return std::max(1,std::min(6,int(index)));
    }
};

enum class RcEvent { NONE,START,CANCEL };
class RcStartGate {
public:
    explicit RcStartGate(const RcConfig& config=RcConfig()) : config(config) {}
    RcConfig config;
    bool ready() const {return ready_;}
    RcEvent update(double now,bool valid,bool offboard_selected,bool active) {
        if(!valid||!std::isfinite(now)||now<last_time_){
            ready_=false;off_since_=on_since_=-1;last_time_=now;
            return active?RcEvent::CANCEL:RcEvent::NONE;
        }
        last_time_=now;
        if(!offboard_selected){
            on_since_=-1;
            if(off_since_<0)off_since_=now;
            if(active)ready_=false;
            else if(now-off_since_>=config.off_hold_seconds)ready_=true;
            return active?RcEvent::CANCEL:RcEvent::NONE;
        }
        off_since_=-1;
        if(active){ready_=false;on_since_=-1;return RcEvent::NONE;}
        if(!ready_){on_since_=-1;return RcEvent::NONE;}
        if(on_since_<0)on_since_=now;
        if(now-on_since_>=config.on_hold_seconds){
            ready_=false;on_since_=-1;return RcEvent::START;
        }
        return RcEvent::NONE;
    }
private:
    bool ready_=false;
    double off_since_=-1,on_since_=-1,last_time_=-1;
};
}
#endif
