#ifndef MINIMAX_BODY_H
#define MINIMAX_BODY_H

class Board;

class Body {
  Vector position;
  Vector speed;

public:
  Body() {}
  Body(Vector p, Vector s) : position(p), speed(s) {}

  const Vector &pos() const { return position; }

  const Vector &v() const { return speed; }

  void setPos(Vector p) { position = p; }

  void setV(Vector s) { speed = s; }

  float getDist(Vector target) const { return (position - target).norm(); }
};

class Robot : public Body {
  int id;
  Vector last_planed_pos;

public:
  Robot(int id) : id(id) {}

  Robot(int id, Vector pos, Vector speed) : Body(pos, speed), id(id) {}

  int getId() const { return id; }

  static constexpr float local_normal_scale() { return 1.0; }

  Vector getNRandPos() const { return Vector::getNRand(last_planed_pos); }
  Vector getURandPos() const { return Vector::getURand(); }
  Vector getURandPos(float rx, float ry) const {
    return Vector::getURand(rx, ry);
  }
  Vector getLocalRandPos(float rx, float ry) const {
    Vector p;
    do {
      p = Vector::getNRand(pos(), local_normal_scale());
    } while (std::abs(p[0]) > rx / 2 || std::abs(p[1]) > ry / 2);
    return p;
  }

  Vector getLocalRadRandPos(float r, float rx, float ry) const {
    Vector p, ux(1, 0), uy(0, 1);
    do {
      // TODO: may be improved
      int decimal = 10000;
      float theta = rand() % ((int)(2 * M_PI * decimal));
      float rand_r = rand() % ((int)(r * decimal));
      theta /= decimal;
      rand_r /= decimal;

      p = ux * (rand_r * cos(theta)) + uy * (rand_r * sin(theta)) + pos();
    } while (std::abs(p[0]) > rx / 2 || std::abs(p[1]) > ry / 2);

    return p;
  }

  Vector getLastPlanedPos() const { return last_planed_pos; }

  static constexpr float maxV2() { return 4 * 4; }

  static constexpr float kickV() { return 7; }

  static constexpr float radius() { return 0.180 / 2; }
};

class Ball : public Body {
public:
  Ball() {}
  Ball(Vector p, Vector v) : Body(p, v) {}
  static constexpr float radius() { return 0.043 / 2; }
};

#endif
