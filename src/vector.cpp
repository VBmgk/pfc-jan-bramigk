#include <cmath>
#include <random>

#include "vector.h"

float norm2(const Vector v) { return v.x * v.x + v.y * v.y; }
float norm(const Vector v) { return std::sqrt(norm2(v)); }
Vector unit(const Vector v) { return v / norm(v); }

static std::random_device _rd;
static std::mt19937_64 generator(_rd());

Vector uniform_rand_vector(float rx, float ry) {
  std::uniform_real_distribution<float> xdistribution(-rx / 2, rx / 2),
      ydistribution(-ry / 2, ry / 2);
  return Vector(xdistribution(generator), ydistribution(generator));
}

Vector normal_rand_vector(const Vector &v, float s) {
  std::normal_distribution<float> distribution(0.0, s);
  return v + Vector(distribution(generator), distribution(generator));
}

Vector rand_vector_bounded(const Vector _vec, float radius, float xb,
                           float yb) {
  Vector pos;
  Vector vec(_vec);

  if (std::abs(vec.x) > xb) {
    vec *= xb / vec.x;
  }
  if (std::abs(vec.y) > yb) {
    vec *= yb / vec.y;
  }

  static std::uniform_real_distribution<float> angle_dice(-M_PI, M_PI);
  std::uniform_real_distribution<float> radius_dice(0, radius);

  do {
    float theta = angle_dice(generator);
    float rand_r = radius_dice(generator);

    pos = vec + Vector(rand_r * cos(theta), rand_r * sin(theta));
  } while (std::abs(pos.x) > xb || std::abs(pos.y) > yb);

  return pos;
}

bool line_segment_cross_circle(Vector p1, Vector p2, Vector c, float r) {
  // Note: this only works because the robot is not a point
  Vector v_u = unit(p1 - p2);
  Vector u = p1 - c;
  Vector w = p2 - c;

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

float dist(const Vector v1, const Vector v2) { return norm(v2 - v1); }
