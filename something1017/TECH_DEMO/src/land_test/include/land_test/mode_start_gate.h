#ifndef LAND_TEST_MODE_START_GATE_H
#define LAND_TEST_MODE_START_GATE_H
#include <cmath>
#include <string>

namespace land_task {
// Interpret PX4's reported mode, never raw RC channels, PWM or flight-mode slots.
struct ModeGateConfig {
    std::string ready_mode="POSCTL";
    double ready_hold_seconds=1.2,offboard_hold_seconds=0.2;
    bool valid() const {
        return ready_mode=="POSCTL" && std::isfinite(ready_hold_seconds) && ready_hold_seconds>1.0 &&
            std::isfinite(offboard_hold_seconds) && offboard_hold_seconds>0;
    }
};
enum class ModeEvent { NONE,START,CANCEL };
class ModeStartGate {
public:
    ModeGateConfig config;
    bool ready() const { return ready_; }
    void reset() { ready_=false;baseline_since_=offboard_since_=-1; }
    ModeEvent update(double now,bool valid,const std::string& mode,bool active,bool expected_land=false) {
        if(!config.valid()||!valid||!std::isfinite(now)||now<last_time_) {
            reset();last_time_=now;return active?ModeEvent::CANCEL:ModeEvent::NONE;
        }
        last_time_=now;
        if(active) {
            reset();
            return mode=="OFFBOARD"||(expected_land&&mode=="AUTO.LAND")?ModeEvent::NONE:ModeEvent::CANCEL;
        }
        if(mode==config.ready_mode) {
            offboard_since_=-1;
            if(baseline_since_<0)baseline_since_=now;
            if(now-baseline_since_>=config.ready_hold_seconds)ready_=true;
        } else if(mode=="OFFBOARD"&&ready_) {
            baseline_since_=-1;
            if(offboard_since_<0)offboard_since_=now;
            if(now-offboard_since_>=config.offboard_hold_seconds) {
                reset();return ModeEvent::START;
            }
        } else reset();
        return ModeEvent::NONE;
    }
private:
    bool ready_=false;
    double baseline_since_=-1,offboard_since_=-1,last_time_=-1;
};
}
#endif
