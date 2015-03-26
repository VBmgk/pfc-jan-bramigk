#ifndef DECISION_H
#define DECISION_H

#include "consts.h"
#include "action.h"
#include "player.h"

struct State;
struct DecisionTable;

struct Decision {
  Action action[N_ROBOTS] = {};
  inline Action &action_for(int robot) { return action[robot % N_ROBOTS]; }
};

void apply_to_state(const Decision decision, Player player, State *state);

Decision gen_decision(bool kick, const State &state, Player player, DecisionTable *table, int robot_to_move = -1);

#endif
