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

  // Overloading output stream operator
  friend std::ostream &operator<<(std::ostream &os, const Vector &v);
};

#endif
