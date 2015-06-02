#ifndef ACTION_H
#define ACTION_H

#include "vector.h"
#include "player.h"

enum ActionType { NONE = 0x0, MOVE = 0x1, PASS = 0x2, KICK = 0x4 };

struct Action {
  ActionType type;
  union {
    Vector move_pos;
    Vector kick_pos;
    int pass_receiver;
  };

  Action(ActionType type = NONE) : type(type) {}
  Action(const Action &a);
  Action &operator=(const Action &a);
};

Action make_move_action(Vector move_pos);
Action make_kick_action(Vector kick_pos);
Action make_pass_action(int pass_receiver);

struct State;
struct DecisionTable;
struct Decision;
Action gen_move_action(int robot, const State &state, DecisionTable &table);
Action gen_kick_action(int robot, const State &state, DecisionTable &table);
Action gen_pass_action(int robot, const State &state, DecisionTable &table);

// TODO: consider refactoring this function to a better form
// int gen_primary_action(bool kick, const State &state, Player player,
// DecisionTable &table, Decision &decision);
Action gen_primary_action(int robot, const State &state, DecisionTable &table,
                          bool kick);

void apply_to_state(Action action, int robot, State *state);

#endif
