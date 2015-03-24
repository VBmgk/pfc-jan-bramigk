#include <stdio.h>

#include "action.h"
#include "discrete.pb.h"

inline void update(Action &a, const Action &b) {
  switch (b.type) {
  case MOVE:
    a.move_pos = b.move_pos;
    break;
  case KICK:
    a.kick_pos = b.kick_pos;
    break;
  case PASS:
    a.pass_receiver = b.pass_receiver;
    break;
  default:
    break;
  }
}

Action::Action(const Action &a) : type(a.type) { update(*this, a); }
Action &Action::operator=(const Action &a) {
  type = a.type;
  update(*this, a);
  return *this;
}

Action make_move_action(Vector move_pos) {
  Action a;
  a.type = MOVE;
  a.move_pos = move_pos;
  return a;
}

Action make_kick_action(Vector kick_pos) {
  Action a;
  a.type = KICK;
  a.kick_pos = kick_pos;
  return a;
}

Action make_pass_action(int pass_receiver) {
  Action a;
  a.type = PASS;
  a.pass_receiver = pass_receiver;
  return a;
}

Action gen_move_action(int robot, const State &state);
Action gen_kick_action(int robot, const State &state);
Action gen_pass_action(int robot, const State &state);

void to_proto_action(Action &action, roboime::Action *ptb_action, int robot_id) {
  ptb_action->set_robot_id(robot_id);
  switch (action.type) {

  case MOVE: {
    ptb_action->set_type(roboime::Action::MOVE);
    auto *move = ptb_action->mutable_move();
    move->set_x(action.move_pos.x);
    move->set_y(action.move_pos.y);
  } break;

  case PASS: {
    ptb_action->set_type(roboime::Action::PASS);
    auto *pass = ptb_action->mutable_pass();
    pass->set_robot_id(action.pass_receiver);
  } break;

  case KICK: {
    ptb_action->set_type(roboime::Action::KICK);
    auto *kick = ptb_action->mutable_kick();
    kick->set_x(action.kick_pos.x);
    kick->set_y(action.kick_pos.y);
  } break;

  case NONE:
    fprintf(stderr, "WARNING: tried to dispatch NONE action, ignored");
  }
}
