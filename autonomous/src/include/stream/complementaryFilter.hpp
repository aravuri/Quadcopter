//
// Created by Muralidhar Ravuri on 11/1/18.
//

#ifndef SENSOR_COMPLEMENTARYFILTER_HPP
#define SENSOR_COMPLEMENTARYFILTER_HPP

#include <cmath>

/**
 * x(t + 1) = alpha * (x(t) + omega_t * delta_t) + (1 - alpha) * atan2(a_x, a_y)
 */
class ComplementaryFilter {
private:
    const double alpha;

    double x;

public:
    explicit ComplementaryFilter(double alpha) : alpha(alpha), x(0) {
    }

    double get() {
        return x;
    }

    double apply(double delta_t, double omega_t, double a_x, double a_y) {
        x = alpha * (x + omega_t * delta_t) + (1.0 - alpha) * atan2(a_x, a_y);
        return x;
    }
};

#endif //SENSOR_COMPLEMENTARYFILTER_HPP
