#include "land_test/motion_limits.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

int main() {
    // Large errors must respect the horizontal norm and asymmetric Z limits.
    for (double x : {-100.0, -1.0, 0.0, 1.0, 100.0}) {
        for (double y : {-100.0, -1.0, 0.0, 1.0, 100.0}) {
            for (double z : {-100.0, -1.0, 0.0, 1.0, 100.0}) {
                const auto v = land_motion::positionVelocity(
                    x, y, z, 0.4, 0.8, 0.20, 0.25, 0.10);
                assert(std::hypot(v.x, v.y) <= 0.20 + 1e-12);
                assert(v.z <= 0.25 && v.z >= -0.10);
                assert(v.x * x >= 0 && v.y * y >= 0 && v.z * z >= 0);
            }
        }
    }

    // Ideal velocity-tracking model at several callback rates. This exercises
    // convergence, no height overshoot, and holding before descent. It does
    // not model aircraft dynamics and does not connect to ROS or an FCU.
    for (double dt : {0.05, 0.10, 0.25}) {
        land_motion::ArrivalHold hold;
        double x = 0.4, y = -0.3, z = 0.0;
        double first_arrival = -1.0;
        bool finished = false;
        for (double now = 0.0; now < 30.0; now += dt) {
            const auto v = land_motion::positionVelocity(
                -x, -y, 1.2 - z, 0.4, 0.8, 0.20, 0.25, 0.10);
            assert(v.z >= 0.0 && v.z <= 0.25);
            const bool arrived = std::fabs(x) < 0.05 &&
                                 std::fabs(y) < 0.05 && std::fabs(1.2 - z) < 0.05;
            if (arrived && first_arrival < 0.0) first_arrival = now;
            if (hold.update(arrived, now, 2.0)) {
                assert(now >= 1.2 / 0.25);
                assert(now - first_arrival >= 2.0);
                finished = true;
                break;
            }
            x += v.x * dt;
            y += v.y * dt;
            z += v.z * dt;
            assert(z <= 1.2 + 1e-12);
        }
        assert(finished);
    }

    land_motion::ArrivalHold hold;
    assert(!hold.update(true, 1.0, 2.0));
    assert(!hold.update(true, 2.9, 2.0));
    assert(!hold.update(false, 3.0, 2.0));
    assert(!hold.update(true, 4.0, 2.0));
    assert(!hold.update(true, 5.9, 2.0));
    assert(hold.update(true, 6.0, 2.0));
    assert(!hold.update(true, 0.0, 2.0));

    const auto invalid = land_motion::positionVelocity(
        std::numeric_limits<double>::quiet_NaN(), 0, 1, 0.4, 0.8, 0.2, 0.25, 0.1);
    assert(invalid.x == 0 && invalid.y == 0 && invalid.z == 0);
    std::cout << "Motion limits and takeoff arrival hold checks passed\n";
}
