#ifndef ACTION_H
#define ACTION_H

#include "vector.h"

enum ActionType { NONE, MOVE, PASS, KICK };

struct Action {
  ActionType type;
  union {
    Vector move_pos;
    Vector kick_pos;
    int pass_receiver;
  };

  Action() : type(NONE) {}
  Action(const Action &a);
  Action &operator=(const Action &a);
};

Action make_move_action(Vector move_pos);
Action make_kick_action(Vector kick_pos);
Action make_pass_action(int pass_receiver);

struct State;
Action gen_move_action(int robot, const State &state);
Action gen_kick_action(int robot, const State &state);
Action gen_pass_action(int robot, const State &state);

namespace roboime {
class Action;
}
void to_proto_action(Action &action, roboime::Action *ptb_action, int robot_id);

#endif
