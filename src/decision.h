#ifndef DECISION_H
#define DECISION_H

#include "consts.h"
#include "action.h"
#include "array.h"
#include "player.h"

struct State;
struct DecisionTable;

struct Decision {
  TeamArray<Action> action = {};
};

void apply_to_state(const Decision decision, Player player, State *state);

Decision gen_decision(bool kick, const State &state, Player player,
                      DecisionTable &table, int robot_to_move = -1);

Decision from_decision_table(DecisionTable &table, const State &state,
                             Player player, bool kick);

void to_proto_command(const Decision &decision, Player player,
                      class CommandMessage &ptb_command,
                      const struct IdTable &table);

#endif
