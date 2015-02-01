#ifndef MINIMAX_BASE_H
#define MINIMAX_BASE_H
#include <iostream>
#include <cmath>
#include <random>

class Vector {
  float v[2];
  static std::default_random_engine generator;

public:
  Vector(const float v[2]) : v{v[0], v[1]} {}

  Vector() : v{0, 0} {}

  Vector(float x, float y) : v{x, y} {}

  // Copy constructor
  Vector(const Vector &o) : v{o.v[0], o.v[1]} {}

  static Vector getURand() {
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    return Vector(distribution(generator), distribution(generator));
  }

  static Vector getURand(float rx, float ry) {
    std::uniform_real_distribution<float> xdistribution(-rx / 2, rx / 2);
    std::uniform_real_distribution<float> ydistribution(-ry / 2, ry / 2);
    return Vector(xdistribution(generator), ydistribution(generator));
  }

  static Vector getNRand(const Vector &v, float s = 1.0) {
    std::normal_distribution<float> distribution(0.0, s);
    return v + Vector(distribution(generator), distribution(generator));
  }

  float x() const { return v[0]; }
  float y() const { return v[1]; }

  float norm2() const { return v[0] * v[0] + v[1] * v[1]; }
  float norm()  const { return std::sqrt(norm2()); }
  Vector unit() const { return (*this) / norm(); }

  Vector operator+(const Vector &o) const { return Vector{v[0] + o.v[0], v[1] + o.v[1]}; }
  Vector operator-(const Vector &o) const { return Vector{v[0] - o.v[0], v[1] - o.v[1]}; }
  Vector operator*(float k) const { return Vector{v[0] * k, v[1] * k}; }
  Vector operator/(float k) const { return Vector{v[0] / k, v[1] / k}; }
  float  operator*(const Vector &o) const { return v[0] * o.v[0] + v[1] * o.v[1]; }

  float operator[](int i) const { return v[i]; }

  static bool lineSegmentCrossCircle(const Vector &p1, const Vector &p2,
                                     const Vector &center, const float radius) {
    // Note: this only works because the robot
    // is not a point
    Vector v_u = (p1 - p2).unit();
    Vector u = p1 - center;
    Vector w = p2 - center;

    // center of circle behind p1
    if (w * v_u > 0)
      return false;

    // perpendicular vector from center of the cirle to the line
    else if ((u - v_u * (u * v_u)).norm() <= radius)
      return true;

    // circle far away
    else
      return false;
  }

  // Overloading output stream operator
  friend std::ostream &operator<<(std::ostream &os, const Vector &v);
};

#endif
