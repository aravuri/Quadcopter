//
// Created by Muralidhar Ravuri on 10/30/18.
//

#ifndef SENSOR_EWMA_HPP
#define SENSOR_EWMA_HPP

#include <math.h>

class EWMA {
private:
    static const int maxOutlierCount = 10;
    static const int maxWarmupCount = 100;

    // parameters that define the behavior of EWMA
    const double alpha;
    const double outlierFraction;

    // main computed state value
    double movingAverage;

    // additional state variables managed by EWMA
    int outlierCount;
    bool warmup;
    bool nonOutlier;

public:
    explicit EWMA(const double alpha = 0.9f, const double outlierFraction = 0.5f)
        : alpha(alpha), outlierFraction(outlierFraction), movingAverage(0),
        outlierCount(0), warmup(true), nonOutlier(true) {
    }

    double get() {
        return movingAverage;
    }

    /**
     * Wait for warmup to be done so we have a steady moving average.
     * After warmup, see if the value is an outlier. If yes, start outlier counter and mark that we did not get
     * a nonOutlier. If no, mark that we just got a nonOutlier.
     * If we got a nonOutlier after any contiguous set of outliers, reset the outlier count to zero. In this case,
     * we want to return the value itself while still computing the moving average.
     * We don't update the moving average only when we have a contiguous set of maxOutlierCount outliers.
     *
     * @param value
     * @return
     */
    double apply(double value) {
        if (warmup) {
            outlierCount++;
            if (outlierCount >= maxWarmupCount) {
                outlierCount = 0;
                warmup = false;
            }
            movingAverage = alpha * movingAverage + (1 - alpha) * value;
            return value;
        } else {
            bool isOutlier = (abs(value) < outlierFraction * abs(movingAverage) ||
                              abs(value) > abs(movingAverage) / outlierFraction);
            if (isOutlier) {
                outlierCount++;
                nonOutlier = false;
            } else {
                nonOutlier = true;
            }
            if (nonOutlier && outlierCount > 0) {
                outlierCount = 0;
            }
            if (outlierCount > 0 and outlierCount < maxOutlierCount) {
                return movingAverage;
            } else {
                if (outlierCount > maxOutlierCount) {
                    outlierCount = maxOutlierCount;
                }
                movingAverage = alpha * movingAverage + (1 - alpha) * value;
                return value;
            }
        }
    }

};

#endif // SENSOR_EWMA_HPP
