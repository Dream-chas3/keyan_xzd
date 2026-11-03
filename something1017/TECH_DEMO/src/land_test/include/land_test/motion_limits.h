#ifndef LAND_TEST_MOTION_LIMITS_H
#define LAND_TEST_MOTION_LIMITS_H

#include <algorithm>
#include <cmath>

namespace land_motion {

struct Velocity {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// The horizontal limit applies to the vector norm, including diagonal motion.
inline Velocity positionVelocity(double dx, double dy, double dz,
                                 double gain_xy, double gain_z,
                                 double max_xy, double max_up, double max_down) {
    Velocity result;
    if (!std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz)) {
        return result;
    }
    result.x = gain_xy * dx;
    result.y = gain_xy * dy;
    const double horizontal_speed = std::hypot(result.x, result.y);
    if (horizontal_speed > max_xy) {
        result.x *= max_xy / horizontal_speed;
        result.y *= max_xy / horizontal_speed;
    }
    result.z = std::max(-max_down, std::min(max_up, gain_z * dz));
    return result;
}

// Complete the hold only after a continuous interval inside the arrival box.
class ArrivalHold {
public:
    bool update(bool arrived, double now, double duration) {
        if (!arrived || (started_ && now < last_time_)) {
            started_ = false;
        }
        last_time_ = now;
        if (!arrived) {
            return false;
        }
        if (!started_) {
            start_time_ = now;
            started_ = true;
        }
        return now - start_time_ >= duration;
    }

private:
    bool started_ = false;
    double start_time_ = 0.0;
    double last_time_ = 0.0;
};

}  // namespace land_motion
#endif
