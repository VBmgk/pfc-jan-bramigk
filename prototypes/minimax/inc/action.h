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

  virtual void discreteAction(roboime::Action*) const = 0;
};

class Move : public Action {
  Vector next_position;
  static constexpr float MAX_SPEED = 4; // meters per second
  float time;

public:
  Move(const class Robot &robot);

  float getTime() const;

  void apply(Player, Board &) const;

  ActionType type() const { return MOVE; }

  Vector pos() { return next_position; }

  void discreteAction(roboime::Action* a) const {
    Vector ux(1,0), uy(0,1);

    a->set_robot_id(getId());
    a->set_type(roboime::Action::MOVE);

    roboime::Action::Move *move = new roboime::Action::Move();
    move->set_x(ux * next_position);
    move->set_y(uy * next_position);

    a->set_allocated_move(move);
  }
};

class Pass : public Action {
  int rcv_id;

public:
  Pass(Robot r_b, Robot r_rcv) : Action(r_b.getId()), rcv_id(r_rcv.getId()) {}

  void apply(Player, Board &) const;

  ActionType type() const { return PASS; }

  int getId() { return rcv_id; }

  void discreteAction(roboime::Action* a) const {
    a->set_robot_id(Action::getId());
    a->set_type(roboime::Action::PASS);

    roboime::Action::Pass *pass = new roboime::Action::Pass();
    pass->set_robot_id(rcv_id);

    a->set_allocated_pass(pass);
  }
};

class Kick : public Action {
  float speed, angle;
  static constexpr float DEFAULT_SPEED = 10; // meters per second

public:
  Kick(Robot robot) : Action(robot.getId()), speed(DEFAULT_SPEED) {}

  void apply(Player, Board &) const;

  ActionType type() const { return KICK; }

  Vector point2Kick() const {
    Vector v;
    // TODO: get some kick point
    return v;
  }

  void discreteAction(roboime::Action* a) const {
    Vector ux(1,0), uy(0,1);

    a->set_robot_id(getId());
    a->set_type(roboime::Action::KICK);

    roboime::Action::Kick *kick = new roboime::Action::Kick();
    Vector point_to_kick = point2Kick();
    kick->set_x(ux * point_to_kick);
    kick->set_y(uy * point_to_kick);

    a->set_allocated_kick(kick);
  }
};

typedef std::vector<std::shared_ptr<Action>> TeamAction;

#endif
