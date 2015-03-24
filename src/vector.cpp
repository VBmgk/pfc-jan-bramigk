#include <cmath>
#include <random>

#include "vector.h"

template <typename T> T norm2(const VectorT<T> v) { return v.x * v.x + v.y * v.y; }
template <typename T> T norm(const VectorT<T> v) { return std::sqrt(norm2(v)); }
template <typename T> VectorT<T> unit(const VectorT<T> v) { return v / norm(v); }

static std::random_device _rd;
static std::mt19937_64 generator(_rd());

template <typename T> VectorT<T> uniform_rand_vector(T rx, T ry) {
  std::uniform_real_distribution<T> xdistribution(-rx / 2, rx / 2), ydistribution(-ry / 2, ry / 2);
  return VectorT<T>(xdistribution(generator), ydistribution(generator));
}

template <typename T> VectorT<T> normal_rand_vector(const VectorT<T> &v, T s) {
  std::normal_distribution<T> distribution(0.0, s);
  return v + VectorT<T>(distribution(generator), distribution(generator));
}

template <typename T> bool line_segment_cross_circle(VectorT<T> p1, VectorT<T> p2, VectorT<T> c, T r) {
  // Note: this only works because the robot is not a point
  VectorT<T> v_u = unit(p1 - p2);
  VectorT<T> u = p1 - c;
  VectorT<T> w = p2 - c;

  // center of circle behind p1
  if (w * v_u > 0)
    return false;

  // perpendicular vector from center of the cirle to the line
  else if (norm(u - v_u * (u * v_u)) <= r)
    return true;

  // circle far away
  else
    return false;
}

// instantiating templates, so both (float/double) versions can be compiled
// See: http://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
#define INSTANTIATE_TEMPLATES(T)                                                                                       \
  template VectorT<T> uniform_rand_vector(T rx, T ry);                                                                 \
  template VectorT<T> normal_rand_vector(const VectorT<T> &v, T s);                                                    \
  template bool line_segment_cross_circle(VectorT<T> p1, VectorT<T> p2, VectorT<T> c, T r);                            \
  template T norm2(const VectorT<T> v);                                                                                \
  template T norm(const VectorT<T> v);                                                                                 \
  template VectorT<T> unit(const VectorT<T> v);

INSTANTIATE_TEMPLATES(float)
INSTANTIATE_TEMPLATES(double)
