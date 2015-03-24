#ifndef VECTOR_H
#define VECTOR_H

template <typename T> struct VectorT {
  T x, y;

  VectorT() : x(0), y(0) {}
  VectorT(const T x, const T y) : x(x), y(y) {}
  VectorT(const VectorT<T> &o) : x(o.x), y(o.y) {}

  VectorT<T> operator+=(const VectorT<T> &o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  VectorT<T> operator+(const VectorT<T> &o) const { return VectorT(x + o.x, y + o.y); }
  VectorT<T> operator-(const VectorT<T> &o) const { return VectorT(x - o.x, y - o.y); }
  VectorT<T> operator*(T k) const { return VectorT(x * k, y * k); }
  VectorT<T> operator/(T k) const { return VectorT(x / k, y / k); }
  T operator*(const VectorT<T> &o) const { return x * o.x + y * o.y; }
};

template <typename T> T norm2(const VectorT<T> v);
template <typename T> T norm(const VectorT<T> v);
template <typename T> VectorT<T> unit(const VectorT<T> v);
template <typename T> VectorT<T> uniform_rand_vector(T rx, T ry);
template <typename T> VectorT<T> normal_rand_vector(const VectorT<T> &v, T s = 1.0);
template <typename T> bool line_segment_cross_circle(VectorT<T> p1, VectorT<T> p2, VectorT<T> c, T r);

#ifdef DOUBLE_PRECISION
typedef double fpoint;
#else
typedef float fpoint;
typedef VectorT<fpoint> Vector;
#endif

#endif
