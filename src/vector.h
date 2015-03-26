#ifndef VECTOR_H
#define VECTOR_H

template <typename T> struct VectorT {
  T x, y;

  constexpr VectorT() : x(0), y(0) {}
  constexpr VectorT(const T x, const T y) : x(x), y(y) {}
  constexpr VectorT(const VectorT<T> &o) : x(o.x), y(o.y) {}

  constexpr VectorT<T> operator+(const VectorT<T> &o) const { return VectorT(x + o.x, y + o.y); }
  constexpr VectorT<T> operator-(const VectorT<T> &o) const { return VectorT(x - o.x, y - o.y); }
  constexpr VectorT<T> operator*(T k) const { return VectorT(x * k, y * k); }
  constexpr VectorT<T> operator/(T k) const { return VectorT(x / k, y / k); }
  constexpr T operator*(const VectorT<T> &o) const { return x * o.x + y * o.y; }
  VectorT<T> operator+=(const VectorT<T> &o) {
    x += o.x;
    y += o.y;
    return *this;
  }
};

template <typename T> T norm2(const VectorT<T> v);
template <typename T> T norm(const VectorT<T> v);
template <typename T> VectorT<T> unit(const VectorT<T> v);
template <typename T> VectorT<T> uniform_rand_vector(T rx, T ry);
template <typename T> VectorT<T> normal_rand_vector(const VectorT<T> &v, T sigma = 1.0);
template <typename T> VectorT<T> rand_vector_bounded(const VectorT<T> vec, T radius, T xbound, T ybound);
template <typename T> bool line_segment_cross_circle(VectorT<T> p1, VectorT<T> p2, VectorT<T> c, T r);
template <typename T> T dist(const VectorT<T> v1, const VectorT<T> v2);

#ifdef DOUBLE_PRECISION
using Vector = VectorT<double>;
#else
using Vector = VectorT<float>;
#endif

#endif
