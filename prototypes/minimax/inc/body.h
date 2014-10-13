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

typedef enum { MIN, MAX } Player;

class Robot : public Body {
  int id;
  mutable Player player;
  Vector last_planed_pos;

public:
  Robot(int id) : id(id) {
    player = MIN;
    Vector zero;
  }

  Robot(int id, Vector pos, Vector speed) : Body(pos, speed), id(id) {
    player = MIN;
    Vector zero;
  }

  int getId() const { return id; }

  void setPlayer(Player p) const { player = p; }

  Player getPlayer() const { return player; }

  Vector getNRandPos() const { return Vector::getNRand(last_planed_pos); }

  Vector getURandPos() const { return Vector::getURand(); }

  Vector getLastPlanedPos() const { return last_planed_pos; }

  static constexpr float maxV2() { return 4*4; }

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
