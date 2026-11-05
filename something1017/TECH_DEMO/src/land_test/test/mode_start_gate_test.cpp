#include "land_test/mode_start_gate.h"
#include "land_test/landing_task.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <limits>
using land_task::ModeEvent;
int main() {
    land_task::ModeStartGate gate;
    assert(gate.config.valid());
    for(int j=0;j<100;++j)assert(gate.update(j*.02,true,"OFFBOARD",false)==ModeEvent::NONE);
    assert(!gate.ready()); // Startup already OFFBOARD is not an edge.
    gate.update(2.1,true,"POSCTL",false);gate.update(3.4,true,"POSCTL",false);assert(gate.ready());
    gate.update(3.5,true,"OFFBOARD",false);assert(gate.update(3.6,true,"OFFBOARD",false)==ModeEvent::NONE);
    gate.update(3.65,true,"POSCTL",false); // Bounce back cannot start.
    gate.update(3.7,true,"OFFBOARD",false);assert(gate.update(4.0,true,"OFFBOARD",false)==ModeEvent::START);
    for(int j=0;j<100;++j)assert(gate.update(4.1+j*.02,true,"OFFBOARD",true)==ModeEvent::NONE);
    for(int j=0;j<100;++j)assert(gate.update(6.1+j*.02,true,"OFFBOARD",false)==ModeEvent::NONE);
    assert(gate.update(8.2,true,"POSCTL",true)==ModeEvent::CANCEL);
    gate.update(8.3,true,"POSCTL",false);gate.update(9.6,true,"POSCTL",false);assert(gate.ready());
    gate.update(9.7,false,"POSCTL",false);assert(!gate.ready());
    assert(gate.update(9.8,true,"OFFBOARD",false)==ModeEvent::NONE);
    gate.update(10,true,"POSCTL",false);gate.update(11.3,true,"POSCTL",false);
    gate.update(11.4,true,"ALTCTL",false);assert(!gate.ready());
    assert(gate.update(11.7,true,"OFFBOARD",false)==ModeEvent::NONE);
    assert(gate.update(12,true,"AUTO.LAND",true,true)==ModeEvent::NONE);
    assert(gate.update(12.1,true,"AUTO.LAND",true,false)==ModeEvent::CANCEL);
    assert(gate.update(12.2,false,"OFFBOARD",true)==ModeEvent::CANCEL);
    gate.update(13,true,"POSCTL",false);gate.update(14.3,true,"POSCTL",false);assert(gate.ready());
    gate.update(12,true,"POSCTL",false);assert(!gate.ready()); // Clock reversal.
    gate.update(std::numeric_limits<double>::quiet_NaN(),true,"POSCTL",false);assert(!gate.ready());
    gate.config.ready_mode="OFFBOARD";assert(!gate.config.valid());
    // Mode-triggered landing begins in actual OFFBOARD and must never request that mode itself.
    land_task::Config cfg;cfg.auto_takeoff=false;land_task::Task task(cfg);land_task::Input i;
    i.now=100;i.mode="OFFBOARD";i.connected=i.state_ok=i.pose_ok=i.velocity_ok=i.extended_ok=true;
    i.armed=true;i.landed_state=2;i.z=1.5;i.gimbal_ready=true;std::string reason;
    assert(task.enable(i,reason));
    for(int j=0;j<50;++j) {i.now+=1.0/30;const auto c=task.step(i);assert(!c.request_offboard&&!c.request_arm);}
    i.mode="POSCTL";i.now+=1.0/30;assert(!task.step(i).publish);assert(!task.active());
    std::cout<<"PX4 mode gate: startup, debounce, cancellation, no self-restart, telemetry loss, AUTO.LAND and airborne takeover passed\n";
}
