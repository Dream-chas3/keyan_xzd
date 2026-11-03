#include "land_test/landing_task.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

struct Sample {land_task::Input in;};
std::vector<std::string> split(const std::string& line) {
    std::istringstream stream(line);std::string value;std::vector<std::string> out;
    while(std::getline(stream,value,',')) {
        if(!value.empty()&&value.back()=='\r')value.pop_back();
        out.push_back(value);
    }
    return out;
}
std::vector<Sample> read(const char* path) {
    std::ifstream file(path);assert(file);std::string line;
    do{assert(std::getline(file,line));}while(line.empty()||line[0]=='#');
    auto fields=split(line);std::map<std::string,size_t> index;
    for(size_t i=0;i<fields.size();++i)index[fields[i]]=i;
    std::vector<Sample> out;double first=0;
    while(std::getline(file,line)) {
        if(line.empty()||line[0]=='#')continue;
        auto row=split(line);
        auto num=[&](const char* key){return std::stod(row.at(index.at(key)));};
        Sample sample;auto& i=sample.in;
        if(out.empty())first=num("ros_time");
        i.now=100+num("ros_time")-first;i.mode=row[index.at("mode")];i.armed=num("armed")!=0;
        i.x=num("x");i.y=num("y");i.z=num("z");i.yaw=num("yaw");
        i.vx=num("vx");i.vy=num("vy");i.vz=num("vz");i.yaw_rate=num("yaw_rate");
        i.target_valid=num("target_valid")!=0;i.target_time=num("target_age")<0?-1:i.now-num("target_age");
        i.target_x=num("target_x");i.target_y=num("target_y");i.target_yaw=num("marker_yaw");i.target_distance=num("marker_depth");
        i.state_ok=i.pose_ok=i.velocity_ok=i.extended_ok=i.connected=i.gimbal_ready=true;
        out.push_back(sample);
    }
    assert(out.size()>200);return out;
}

land_task::Input interpolate(const Sample& a,const Sample& b,double now,double ground) {
    auto i=a.in;const double fraction=(now-a.in.now)/(b.in.now-a.in.now);
    auto mix=[&](double x,double y){return x+(y-x)*fraction;};
    i.now=now;i.x=mix(a.in.x,b.in.x);i.y=mix(a.in.y,b.in.y);i.z=mix(a.in.z,b.in.z);
    i.vx=mix(a.in.vx,b.in.vx);i.vy=mix(a.in.vy,b.in.vy);i.vz=mix(a.in.vz,b.in.vz);
    i.yaw_rate=mix(a.in.yaw_rate,b.in.yaw_rate);i.yaw=mix(a.in.yaw,b.in.yaw);
    // Target validity/receipt are discrete observed events, not fabricated new frames.
    i.landed_state=i.z-ground>0.10?2:1;
    return i;
}

struct Result {double translated_at=-1;land_task::Phase end;bool toward_marker=false;};
Result replay(const std::vector<Sample>& samples,bool old_gate) {
    land_task::Config cfg;cfg.auto_takeoff=true;cfg.takeoff_altitude=1.5;
    cfg.takeoff_speed=cfg.max_z_speed=old_gate?0.25:0.28;
    if(old_gate) {
        cfg.takeoff_xy_tolerance=.08;cfg.takeoff_z_tolerance=.05;
        cfg.takeoff_stable_xy_speed=cfg.takeoff_stable_z_speed=.06;cfg.takeoff_stable_yaw_rate=.03;
    }
    land_task::Task task(cfg);auto initial=samples.front().in;initial.landed_state=1;
    std::string reason;assert(task.enable(initial,reason));Result result;
    size_t j=0;double ground=initial.z;
    for(double now=initial.now;now<samples.back().in.now;now+=1.0/30.0) {
        while(j+1<samples.size()-1&&samples[j+1].in.now<=now)++j;
        auto i=interpolate(samples[j],samples[j+1],now,ground);
        auto out=task.step(i);
        if(out.phase==land_task::Phase::TRANSLATE&&result.translated_at<0)result.translated_at=now-initial.now;
        if(out.phase==land_task::Phase::TRANSLATE&&std::hypot(out.velocity.x,out.velocity.y)>.01 &&
           out.velocity.x*(i.target_x-i.x)+out.velocity.y*(i.target_y-i.y)>0)result.toward_marker=true;
    }
    result.end=task.phase();return result;
}
int main(int argc,char** argv) {
    assert(argc==2);auto samples=read(argv[1]);auto before=replay(samples,true),after=replay(samples,false);
    assert(before.translated_at<0&&before.end==land_task::Phase::TAKEOFF);
    assert(after.translated_at>0&&after.translated_at<20&&after.toward_marker);
    std::cout<<"Recorded-flight replay: old gate remains TAKEOFF; new gate reaches TRANSLATE at +"
        <<after.translated_at<<" s and commands motion toward the accepted marker. "
        <<"5 Hz measurements interpolated for offline policy regression only; no aircraft dynamics validation.\n";
}
