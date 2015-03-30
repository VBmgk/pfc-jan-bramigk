#include <stdio.h>

#include "decision.h"
#include "decision_table.h"
#include "state.h"
#include "action.h"
#include "consts.h"
#include "utils.h"
#include "discrete.pb.h"
#include "id_table.h"
#include "segment.h"

void apply_to_state(const Decision decision, Player player, struct State *state) {
  // apply all moves first
  FOR_TEAM_ROBOT(i, player) {
    if (decision.action[i].type == MOVE)
      apply_to_state(decision.action[i], i, state);
  }
  // and then all others
  FOR_TEAM_ROBOT(i, player) {
    if (decision.action[i].type != MOVE)
      apply_to_state(decision.action[i], i, state);
  }
}

Decision gen_decision(bool kick, const State &state, Player player, DecisionTable &table, int robot_to_move) {
  Decision decision;

  int rwb = robot_with_ball(state);

  // push an action for the robot with ball, if it's us
  if (player == PLAYER_OF(rwb)) {
    decision.action[rwb] = kick ? gen_kick_action(rwb, state) : gen_pass_action(rwb, state, table);
  }

  // push a Move action for every other robot
  FOR_TEAM_ROBOT(i, player) if (i != rwb) {
    decision.action[i] = (robot_to_move == i || robot_to_move == -1) ? gen_move_action(i, state) : table.move[i];
  }

  return decision;
}

Decision from_decision_table(const struct DecisionTable &table) {
  Decision decision;

  FOR_N(i, N_ROBOTS) { decision.action[i] = table.move[i]; }
  if (table.kick_robot >= 0) {
    decision.action[table.kick_robot] = table.kick;
  }

  if (table.pass_robot >= 0) {
    decision.action[table.pass_robot] = table.pass;
  }

  return decision;
}

void to_proto_command(const Decision &decision, Player player, roboime::Command &ptb_command, const IdTable &table) {
  FOR_TEAM_ROBOT(i, MAX) {
    Action &&action = decision.action[i];
    if (action.type != NONE) {
      ::roboime::Action *ptb_action = ptb_command.add_action();
      to_proto_action(action, ptb_action, table.id[i]);
    }
  }
}
