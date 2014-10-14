#ifndef MINIMAX_BASE_H
#define MINIMAX_BASE_H
#include <armadillo>
#include <iostream>

class Vector {
  arma::vec a_v;
  static const unsigned int VEC_SIZE = 2;

public:
  Vector(const arma::vec &v) : a_v(v) {}

  Vector() : a_v(arma::zeros<arma::vec>(Vector::VEC_SIZE)) {}

  Vector(float x, float y) : a_v({x, y}) {}

  // Copy constructor
  Vector(const Vector &vec) : a_v(vec.a_v) {}

  static Vector getURand() {
    arma::arma_rng::set_seed_random();

    return Vector(arma::randu<arma::vec>(Vector::VEC_SIZE));
  }

  static Vector getURand(float rx, float ry) {
    arma::arma_rng::set_seed_random();
    Vector v(arma::randu<arma::vec>(Vector::VEC_SIZE));
    v.a_v[0] = v.a_v[0] * rx - rx / 2;
    v.a_v[1] = v.a_v[1] * ry - ry / 2;
    return v;
  }

  static Vector getNRand(const Vector &v) {
    arma::arma_rng::set_seed_random();

    // Center Distribution on last value
    return Vector(v.a_v + arma::randn<arma::vec>(Vector::VEC_SIZE));
  }

  float norm() { return (float)arma::norm(a_v); }

  Vector operator+(const Vector &v2) const {
    Vector vr(a_v + v2.a_v);
    return vr;
  }

  Vector operator-(const Vector &v2) const {
    Vector vr(a_v - v2.a_v);
    return vr;
  }

  float operator*(const Vector &v2) const {
    // Using trace to avoid conversion operator
    return arma::trace((a_v.t() * v2.a_v));
  }

  Vector operator*(float k) const { return Vector(a_v * k); }

  float operator[](int i) const {
    // return (float) a_v[i];// this is a fast but unsafe alternative
    return (float)a_v(i);
  }

  static Vector unit(Vector v) {
    Vector vec(arma::normalise(v.a_v));

    return vec;
  }

  static bool lineSegmentCrossCircle(const Vector &p1, const Vector &p2,
                                     const Vector &center, const float radius) {
    // Note: this only works because the robot
    // is not a point
    Vector v_u = unit(p1 - p2);
    Vector u = p1 - center;
    Vector w = p2 - center;

    // center of circle behind p1
    if (w * v_u < 0)
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
