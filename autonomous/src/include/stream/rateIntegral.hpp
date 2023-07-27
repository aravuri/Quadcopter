//
// Created by Muralidhar Ravuri on 11/1/18.
//

#ifndef SENSOR_RATEINTEGRAL_HPP
#define SENSOR_RATEINTEGRAL_HPP

#include <utils/math.hpp>

/**
 * q(t + 1) = q(t) * q_delta
 * q_delta = q(delta_t * || omega_t ||, omega_t / || omega_t ||)
 *
 * Here, q(t) is the rotation from sensor to world frame. To convert to a point v_sensor to world coordinates:
 *  q_world(t + 1) = q(t + 1) * v_sensor * inverse(q(t + 1))
 */
class RateIntegral {
private:
    Quaternion q;

public:
    RateIntegral() : q(1, 0, 0, 0) {
    }

    inline Quaternion get() {
        return q;
    }

    Quaternion apply(double delta_t, const Vector3 &omega_t, Quaternion *pastQ = nullptr) {
        Quaternion q_delta;
        const double length = omega_t.length();
        q_delta.fromAngleVector(delta_t * length, omega_t);

        if (!pastQ) {
            q *= q_delta;
        } else {
            q = *pastQ * q_delta;
        }
        return q;
    }
};

#endif //SENSOR_RATEINTEGRAL_HPP
