#ifndef VECTOR_H
#define VECTOR_H

struct Vector {
  float x, y;

  constexpr Vector() : x(0), y(0) {}
  constexpr Vector(const float x, const float y) : x(x), y(y) {}
  constexpr Vector(const Vector &o) : x(o.x), y(o.y) {}

  constexpr Vector operator+(const Vector &o) const {
    return Vector(x + o.x, y + o.y);
  }
  constexpr Vector operator-(const Vector &o) const {
    return Vector(x - o.x, y - o.y);
  }
  constexpr Vector operator*(float k) const { return Vector(x * k, y * k); }
  constexpr Vector operator/(float k) const { return Vector(x / k, y / k); }
  constexpr float operator*(const Vector &o) const { return x * o.x + y * o.y; }
  Vector operator+=(const Vector &o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  Vector operator*=(float k) {
    x *= k;
    y *= k;
    return *this;
  }
  Vector operator/=(float k) {
    x /= k;
    y /= k;
    return *this;
  }
};

float norm2(const Vector v);
float norm(const Vector v);
Vector unit(const Vector v);
Vector uniform_rand_vector(float rx, float ry);
Vector normal_rand_vector(const Vector &v, float sigma = 1.0);
Vector rand_vector_bounded(const Vector vec, float radius, float xbound,
                           float ybound);
bool line_segment_cross_circle(Vector p1, Vector p2, Vector c, float r);
float dist(const Vector v1, const Vector v2);

#endif
