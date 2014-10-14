#ifndef MINIMAX_ACTION_H
#define MINIMAX_ACTION_H

// dimensions in meters
#define FIELD_WIDTH 4.0
#define FIELD_HIGHT 6.0

enum Player { MIN, MAX };

class Action {
  int robot_id;

public:
  Action(int r_id) : robot_id(r_id) {}

  virtual float getTime() const { return 0; }

  virtual void apply(Player, Board &) const {};
};

class Move : public Action {
  Vector nextPosition;
  static constexpr float MAX_SPEED = 4; // meters per second
  float time;

public:
  Move(const class Robot &robot);

  float getTime() const;

  void apply(Player, Board &) const;
};

class Pass : public Action {
  int rcv_id;

public:
  Pass(Robot r_b, Robot r_rcv) : Action(r_b.getId()), rcv_id(r_rcv.getId()) {}

  void apply(Player, Board &) const;
};

class Kick : public Action {
  float speed, angle;
  static constexpr float DEFAULT_SPEED = 10; // meters per second

public:
  Kick(Robot robot) : Action(robot.getId()), speed(DEFAULT_SPEED) {}

  void apply(Player, Board &) const;
};

typedef std::vector<Action> TeamAction;

#endif
