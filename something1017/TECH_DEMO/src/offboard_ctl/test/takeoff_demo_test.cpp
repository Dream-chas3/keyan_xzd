#include "offboard_ctl/takeoff_demo.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <limits>
using offboard_demo::Phase;
struct Rig {
    offboard_demo::Task task;offboard_demo::Input i;
    Rig() {i.now=100;i.healthy=i.control_clear=i.on_ground=true;i.mode="POSCTL";i.x=4;i.y=-3;i.z=0.7;i.yaw=1.1;}
    offboard_demo::Command tick() {i.now+=1.0/30;return task.step(i);}
    void ready() {for(int j=0;j<42;++j){auto c=tick();assert(c.publish&&!c.request_arm&&!c.request_land);assert(c.z==0.7);}assert(task.phase()==Phase::READY);}
    void start() {ready();i.mode="OFFBOARD";for(int j=0;j<10;++j)tick();assert(task.phase()==Phase::ARMING);}
};
int main() {
    {Rig r;r.i.mode="OFFBOARD";for(int j=0;j<300;++j){auto c=r.tick();assert(!c.publish&&!c.request_arm&&!c.request_land);}r.i.mode="POSCTL";r.start();}
    {Rig r;r.start();r.i.armed=true;auto c=r.tick();assert(r.task.phase()==Phase::TAKEOFF);assert(c.x==4&&c.y==-3&&c.z==1.7&&c.yaw==1.1);
     r.i.z=1.7;r.i.on_ground=false;
     for(int j=0;j<59;++j){c=r.tick();assert(!c.request_land);} // Not before two stable seconds.
     for(int j=0;j<4;++j)c=r.tick();
     assert(r.task.phase()==Phase::LANDING);assert(r.task.landRequested());
     r.i.mode="AUTO.LAND";assert(!r.tick().publish);r.i.on_ground=true;r.i.armed=false;
     assert(!r.tick().publish);assert(r.task.phase()==Phase::COMPLETE);
     r.i.mode="OFFBOARD";for(int j=0;j<90;++j){c=r.tick();assert(!c.publish&&!c.request_arm&&!c.request_land);}}
    {Rig r;r.start();r.i.mode="POSCTL";auto c=r.tick();assert(!c.publish&&!c.request_arm&&!c.request_land);assert(!r.task.active());
     r.i.mode="OFFBOARD";for(int j=0;j<90;++j)assert(!r.tick().publish);r.i.mode="POSCTL";r.ready();}
    {Rig r;r.start();r.i.healthy=false;assert(!r.tick().publish);r.i.healthy=true;for(int j=0;j<50;++j)assert(!r.tick().publish);}
    {Rig r;r.i.on_ground=false;for(int j=0;j<50;++j)assert(!r.tick().publish);assert(r.task.phase()!=Phase::READY);}
    {Rig r;r.i.control_clear=false;for(int j=0;j<50;++j)assert(!r.tick().publish);assert(r.task.phase()!=Phase::READY);}
    {Rig r;r.ready();r.i.mode="OFFBOARD";r.tick();r.i.mode="ALTCTL";assert(!r.tick().publish);r.i.mode="OFFBOARD";for(int j=0;j<50;++j)assert(!r.tick().publish);}
    {Rig r;r.start();r.i.now+=0.5;assert(!r.tick().publish);assert(!r.task.active());}
    {Rig r;r.start();r.i.x=std::numeric_limits<double>::quiet_NaN();assert(!r.tick().publish);}
    {Rig r;r.start();int requests=0;for(int j=0;j<400;++j)requests+=r.tick().request_arm;assert(!r.task.active());assert(requests<12);}
    {Rig r;r.start();r.i.armed=true;r.tick();r.i.z=1.7;r.i.on_ground=false;
     for(int j=0;j<45;++j)assert(!r.tick().request_land);
     r.i.vz=.2;r.tick();r.i.vz=0;
     for(int j=0;j<50;++j)assert(!r.tick().request_land);
     assert(r.task.phase()==Phase::HOVER);}
    std::cout<<"offboard demo: mode interlock, relative anchor/yaw, two-second hover, landing, pilot override, no retry, stale data, competition and timeouts passed\n";
}
