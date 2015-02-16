#ifndef MINIMAX_ACTION_H
#define MINIMAX_ACTION_H

#include <memory>
#include "discrete.pb.h"

enum Player { MIN, MAX };

class Action {
  int robot_id;

public:
  Action(int r_id) : robot_id(r_id) {}

  int getId() const { return robot_id; }

  virtual float getTime() const { return 0; }

  virtual void apply(Player, Board &) const = 0;

  enum ActionType { MOVE, PASS, KICK };
  virtual ActionType type() const = 0;

  virtual void discreteAction(roboime::Action *) const = 0;

  Vector pos() { return Vector(); }
};

class Move : public Action {
  Vector next_position;
  static constexpr float MAX_SPEED = 4; // meters per second
  float time;

public:
  Move(const Robot &robot);

  float getTime() const;

  void apply(Player, Board &) const;

  ActionType type() const { return MOVE; }

  Vector pos() { return next_position; }

  void discreteAction(roboime::Action *a) const {
    Vector ux(1, 0), uy(0, 1);

    a->set_robot_id(getId());
    a->set_type(roboime::Action::MOVE);

    roboime::Action::Move *move = a->mutable_move();
    move->set_x(ux * next_position);
    move->set_y(uy * next_position);
  }
};

class Pass : public Action {
  int rcv_id;

public:
  Pass(const Robot &r_b, const Robot &r_rcv)
      : Action(r_b.getId()), rcv_id(r_rcv.getId()) {}

  void apply(Player, Board &) const;

  ActionType type() const { return PASS; }

  int getRcvId() const { return rcv_id; }

  void discreteAction(roboime::Action *a) const {
    a->set_robot_id(Action::getId());
    a->set_type(roboime::Action::PASS);

    roboime::Action::Pass *pass = a->mutable_pass();
    pass->set_robot_id(rcv_id);
  }
};

class Kick : public Action {
  float speed, angle;
  static constexpr float DEFAULT_SPEED = 10; // meters per second

public:
  Kick(const Robot &robot) : Action(robot.getId()), speed(DEFAULT_SPEED) {}

  void apply(Player, Board &) const;

  ActionType type() const { return KICK; }

  Vector point2Kick() const {
    Vector v;
    // TODO: get some kick point
    return v;
  }

  void discreteAction(roboime::Action *a) const {
    Vector ux(1, 0), uy(0, 1);

    a->set_robot_id(getId());
    a->set_type(roboime::Action::KICK);

    roboime::Action::Kick *kick = a->mutable_kick();
    Vector point_to_kick = point2Kick();
    kick->set_x(ux * point_to_kick);
    kick->set_y(uy * point_to_kick);
  }
};

typedef std::vector<std::shared_ptr<Action>> TeamAction;
typedef std::map<int, std::shared_ptr<Move>> MoveTable;

#endif
