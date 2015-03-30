#ifndef ARRAY_H
#define ARRAY_H

#include "consts.h"

template <typename T> struct GameArray {
  T _[2 * N_ROBOTS] = {};
  constexpr T operator[](int i) const { return _[i]; }
  inline T &operator[](int i) { return _[i]; }
};

template <typename T> struct TeamArray {
  T _[2 * N_ROBOTS] = {};
  constexpr T operator[](int i) const { return _[i % N_ROBOTS]; }
  inline T &operator[](int i) { return _[i % N_ROBOTS]; }
};

#endif
