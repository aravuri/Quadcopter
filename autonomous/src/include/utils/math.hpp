//
// Created by Muralidhar Ravuri on 10/27/18.
//

#ifndef SENSOR_MATH_HPP_
#define SENSOR_MATH_HPP_

#include <math.h>
#include <cstring>
#include <stdint.h>
#include <string>
#include <sstream>

#define     DEGREE_TO_RAD       (M_PI / 180.0)
#define     RAD_TO_DEGREE       (180.0 / M_PI)
#define     EPSILON             1e-8

using namespace std;

class Vector3 {
private:
    double value[3];

public:
    Vector3() {
        zero();
    }

    Vector3(double x, double y, double z) {
        value[0] = x;
        value[1] = y;
        value[2] = z;
    }

    const Vector3 &operator+=(const Vector3 &vec) {
        for (int i = 0; i < 3; i++) {
            value[i] += vec.value[i];
        }
        return *this;
    }

    const Vector3 &operator-=(const Vector3 &vec) {
        for (int i = 0; i < 3; i++) {
            value[i] -= vec.value[i];
        }
        return *this;
    }

    Vector3 &operator*=(const Vector3 &vec) {
        value[0] *= vec.value[0];
        value[1] *= vec.value[1];
        value[2] *= vec.value[2];

        return *this;
    }

    Vector3 &operator*=(const double val) {
        value[0] *= val;
        value[1] *= val;
        value[2] *= val;

        return *this;
    }

    const Vector3 operator*(const Vector3 &vec) const {
        Vector3 result = *this;
        result *= vec;
        return result;
    }

    const Vector3 operator*(const double val) const {
        Vector3 result = *this;
        result *= val;
        return result;
    }

    const Vector3 operator+(const Vector3 &vec) const {
        Vector3 result = *this;
        result += vec;
        return result;
    }

    const Vector3 operator-(const Vector3 &vec) const {
        Vector3 result = *this;
        result -= vec;
        return result;
    }

    Vector3 &operator=(const Vector3 &vec) {
        if (this == &vec) {
            return *this;
        }
        value[0] = vec.value[0];
        value[1] = vec.value[1];
        value[2] = vec.value[2];
        return *this;
    }

    double length() const {
        return sqrt(value[0] * value[0] + value[1] * value[1] + value[2] * value[2]);
    }

    void normalize() {
        double len = length();
        if (len <= EPSILON) {
            return;
        }

        value[0] /= len;
        value[1] /= len;
        value[2] /= len;
    }

    void zero() {
        for (int i = 0; i < 3; i++) {
            value[i] = 0;
        }
    }

    bool isZero() {
        for (int i = 0; i < 3; i++) {
            if (value[i] != 0) {
                return false;
            }
        }
        return true;
    }

    static double dotProduct(const Vector3 &a, const Vector3 &b) {
        return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
    }

    static void crossProduct(const Vector3 &a, const Vector3 &b, Vector3 &d) {
        d.setX(a.y() * b.z() - a.z() * b.y());
        d.setY(a.z() * b.x() - a.x() * b.z());
        d.setZ(a.x() * b.y() - a.y() * b.x());
    }

    string displayDegrees(const char *label) {
        char output[1000];
        sprintf(output, "%s: roll:%f, pitch:%f, yaw:%f", label, x() * RAD_TO_DEGREE, y() * RAD_TO_DEGREE,
                z() * RAD_TO_DEGREE);
        return string(output);
    }

    inline double x() const { return value[0]; }

    inline double y() const { return value[1]; }

    inline double z() const { return value[2]; }

    inline void setX(const double val) { value[0] = val; }

    inline void setY(const double val) { value[1] = val; }

    inline void setZ(const double val) { value[2] = val; }

    inline void fromArray(double *val) { memcpy(value, val, 3 * sizeof(double)); }

    inline void toArray(double *val) const { memcpy(val, value, 3 * sizeof(double)); }

    string toString() {
        stringstream ss;
        ss << value[0] << "," << value[1] << "," << value[2];
        return ss.str();
    }

};

void convertToVector(const unsigned char *rawData, Vector3 &vec, double scale, bool bigEndian) {
    if (bigEndian) {
        vec.setX((double) ((int16_t) (((uint16_t) rawData[0] << 8) | (uint16_t) rawData[1])) * scale);
        vec.setY((double) ((int16_t) (((uint16_t) rawData[2] << 8) | (uint16_t) rawData[3])) * scale);
        vec.setZ((double) ((int16_t) (((uint16_t) rawData[4] << 8) | (uint16_t) rawData[5])) * scale);
    } else {
        vec.setX((double) ((int16_t) (((uint16_t) rawData[1] << 8) | (uint16_t) rawData[0])) * scale);
        vec.setY((double) ((int16_t) (((uint16_t) rawData[3] << 8) | (uint16_t) rawData[2])) * scale);
        vec.setZ((double) ((int16_t) (((uint16_t) rawData[5] << 8) | (uint16_t) rawData[4])) * scale);
    }
}

class Quaternion {
private:
    double value[4];

public:
    Quaternion() {
        zero();
    }

    Quaternion(double scalar, double x, double y, double z) {
        value[0] = scalar;
        value[1] = x;
        value[2] = y;
        value[3] = z;
    }

    Quaternion(const Vector3 &vec) {
        value[0] = 0;
        value[1] = vec.x();
        value[2] = vec.y();
        value[3] = vec.z();
    }

    Quaternion &operator+=(const Quaternion &quat) {
        for (int i = 0; i < 4; i++) {
            value[i] += quat.value[i];
        }
        return *this;
    }

    Quaternion &operator+=(const double val) {
        for (int i = 0; i < 4; i++) {
            value[i] += val;
        }
        return *this;
    }

    Quaternion &operator-=(const Quaternion &quat) {
        for (int i = 0; i < 4; i++) {
            value[i] -= quat.value[i];
        }
        return *this;
    }

    Quaternion &operator-=(const double val) {
        for (int i = 0; i < 4; i++) {
            value[i] -= val;
        }
        return *this;
    }

    Quaternion &operator*=(const Quaternion &qb) {
        Quaternion qa = *this;

        value[0] = qa.scalar() * qb.scalar() - qa.x() * qb.x() - qa.y() * qb.y() - qa.z() * qb.z();
        value[1] = qa.scalar() * qb.x() + qa.x() * qb.scalar() + qa.y() * qb.z() - qa.z() * qb.y();
        value[2] = qa.scalar() * qb.y() - qa.x() * qb.z() + qa.y() * qb.scalar() + qa.z() * qb.x();
        value[3] = qa.scalar() * qb.z() + qa.x() * qb.y() - qa.y() * qb.x() + qa.z() * qb.scalar();

        return *this;
    }

    Quaternion &operator*=(const double val) {
        value[0] *= val;
        value[1] *= val;
        value[2] *= val;
        value[3] *= val;

        return *this;
    }

    Quaternion &operator=(const Quaternion &quat) {
        if (this == &quat) {
            return *this;
        }

        value[0] = quat.value[0];
        value[1] = quat.value[1];
        value[2] = quat.value[2];
        value[3] = quat.value[3];

        return *this;
    }

    const Quaternion operator*(const Quaternion &qb) const {
        Quaternion result = *this;
        result *= qb;
        return result;
    }

    const Quaternion operator*(const Vector3 &vec) const {
        Quaternion result = *this;
        Quaternion qvec(vec);
        result *= qvec;
        return result;
    }

    const Quaternion operator*(const double val) const {
        Quaternion result = *this;
        result *= val;
        return result;
    }

    const Quaternion operator+(const Quaternion &qb) const {
        Quaternion result = *this;
        result += qb;
        return result;
    }

    const Quaternion operator+(const double val) const {
        Quaternion result = *this;
        result += val;
        return result;
    }

    const Quaternion operator-(const Quaternion &qb) const {
        Quaternion result = *this;
        result -= qb;
        return result;
    }

    const Quaternion operator-(const double val) const {
        Quaternion result = *this;
        result -= val;
        return result;
    }

    void zero() {
        for (int i = 0; i < 4; i++) {
            value[i] = 0;
        }
    }

    double length() const {
        return sqrt(value[0] * value[0] + value[1] * value[1] + value[2] * value[2] + value[3] * value[3]);
    }

    void normalize() {
        double len = length();
        if ((len <= EPSILON) || (len == 1)) {
            return;
        }

        value[0] /= len;
        value[1] /= len;
        value[2] /= len;
        value[3] /= len;
    }

    Quaternion conjugate() const {
        Quaternion q;
        q.setScalar(value[0]);
        q.setX(-value[1]);
        q.setY(-value[2]);
        q.setZ(-value[3]);
        return q;
    }

    Quaternion inverse() const {
        Quaternion q = conjugate();
        double len2 = value[0] * value[0] + value[1] * value[1] + value[2] * value[2] + value[3] * value[3];
        if (len2 == 1) {
            return q;
        }
        return q * (1.0 / len2);
    }

    Quaternion rotate(Vector3 &vec) const {
        Quaternion q = *this;
        return q * vec * q.inverse();
    }

    void toEuler(Vector3 &vec) {
        vec.setX(atan2(2.0 * (value[2] * value[3] + value[0] * value[1]),
                       1 - 2.0 * (value[1] * value[1] + value[2] * value[2])));
        vec.setY(asin(2.0 * (value[0] * value[2] - value[1] * value[3])));
        vec.setZ(atan2(2.0 * (value[1] * value[2] + value[0] * value[3]),
                       1 - 2.0 * (value[2] * value[2] + value[3] * value[3])));
    }

    void fromEuler(Vector3 &vec) {
        double cosX2 = cos(vec.x() / 2.0);
        double sinX2 = sin(vec.x() / 2.0);
        double cosY2 = cos(vec.y() / 2.0);
        double sinY2 = sin(vec.y() / 2.0);
        double cosZ2 = cos(vec.z() / 2.0);
        double sinZ2 = sin(vec.z() / 2.0);

        value[0] = cosX2 * cosY2 * cosZ2 + sinX2 * sinY2 * sinZ2;
        value[1] = sinX2 * cosY2 * cosZ2 - cosX2 * sinY2 * sinZ2;
        value[2] = cosX2 * sinY2 * cosZ2 + sinX2 * cosY2 * sinZ2;
        value[3] = cosX2 * cosY2 * sinZ2 - sinX2 * sinY2 * cosZ2;
        normalize();
    }

    void toAngleVector(double &angle, Vector3 &vec) {
        double halfTheta = acos(value[0]);
        double sinHalfTheta = sin(halfTheta);

        if (sinHalfTheta == 0) {
            vec.setX(1.0);
            vec.setY(0);
            vec.setZ(0);
        } else {
            vec.setX(value[1] / sinHalfTheta);
            vec.setY(value[1] / sinHalfTheta);
            vec.setZ(value[1] / sinHalfTheta);
        }
        angle = 2.0 * halfTheta;
    }

    void fromAngleVector(const double &angle, const Vector3 &vec) {
        Vector3 nVec = vec;
        nVec.normalize();

        double sinHalfTheta = sin(angle / 2.0);
        value[0] = cos(angle / 2.0);
        value[1] = nVec.x() * sinHalfTheta;
        value[2] = nVec.y() * sinHalfTheta;
        value[3] = nVec.z() * sinHalfTheta;
    }

    inline double scalar() const { return value[0]; }

    inline double x() const { return value[1]; }

    inline double y() const { return value[2]; }

    inline double z() const { return value[3]; }

    inline Vector3 vector() const { return Vector3(value[1], value[2], value[3]); }

    inline void setScalar(const double val) { value[0] = val; }

    inline void setX(const double val) { value[1] = val; }

    inline void setY(const double val) { value[2] = val; }

    inline void setZ(const double val) { value[3] = val; }

    inline void fromArray(double *val) { memcpy(value, val, 4 * sizeof(double)); }

    inline void toArray(double *val) const { memcpy(val, value, 4 * sizeof(double)); }

    string toString() {
        stringstream ss;
        ss << value[0] << "," << value[1] << "," << value[2] << "," << value[3];
        return ss.str();
    }

};

#endif /* SENSOR_MATH_HPP_ */
