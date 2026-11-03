#include "land_test/rc_start_gate.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
using land_task::RcEvent;
int main() {
    land_task::RcConfig config;
    assert(config.valid());assert(config.slot(1000)==1);assert(config.slot(1514)==4);assert(config.slot(2000)==6);
    assert(config.slot(0)==0);config.reversed=true;assert(config.slot(1000)==6);assert(config.slot(2000)==1);
    config.reversed=false;
    land_task::RcStartGate gate(config);
    for(int j=0;j<100;++j)assert(gate.update(j*.02,true,true,false)==RcEvent::NONE);
    assert(!gate.ready()); // Starting with the switch already ON must never launch.
    assert(gate.update(2.1,true,false,false)==RcEvent::NONE);
    assert(gate.update(2.5,true,false,false)==RcEvent::NONE);assert(gate.ready());
    assert(gate.update(2.6,true,true,false)==RcEvent::NONE);
    assert(gate.update(2.9,true,true,false)==RcEvent::START);
    for(int j=0;j<100;++j)assert(gate.update(3+j*.02,true,true,true)==RcEvent::NONE);
    // Pilot/FCU cancelled without moving the RC selector: no self-restart.
    for(int j=0;j<100;++j)assert(gate.update(5+j*.02,true,true,false)==RcEvent::NONE);
    assert(gate.update(7.1,true,false,true)==RcEvent::CANCEL);
    assert(gate.update(7.2,true,true,false)==RcEvent::NONE); // A brief bounce cannot retrigger.
    assert(gate.update(7.6,true,true,false)==RcEvent::NONE);
    gate.update(8,true,false,false);gate.update(8.5,true,false,false);assert(gate.ready());
    gate.update(8.6,false,false,false);assert(!gate.ready()); // RC loss clears the rising-edge permission.
    assert(gate.update(8.7,true,true,false)==RcEvent::NONE);
    gate.update(9,true,false,false);gate.update(9.5,true,false,false);
    gate.update(9.6,true,true,false);assert(gate.update(9.9,true,true,false)==RcEvent::START);
    assert(gate.update(10,false,true,true)==RcEvent::CANCEL);
    config.offboard_slot=7;assert(!config.valid());assert(config.slot(1500)==0);
    std::cout<<"RC selector calibration, startup interlock, edge debounce, sticky cancellation and signal-loss checks passed\n";
}
