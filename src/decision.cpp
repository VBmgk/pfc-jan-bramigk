#include <stdio.h>

#include "decision.h"
#include "decision_table.h"
#include "state.h"
#include "action.h"
#include "consts.h"
#include "utils.h"

void apply_to_state(const Decision decision, Player player, struct State *state) {
  // apply all moves first
  FOR_TEAM_ROBOT(i, player) {
    if (decision.action[ID(i)].type == MOVE)
      apply_to_state(decision.action[ID(i)], i, state);
  }
  // and then all others
  FOR_TEAM_ROBOT(i, player) {
    if (decision.action[ID(i)].type != MOVE)
      apply_to_state(decision.action[ID(i)], i, state);
  }
}

Decision gen_decision(bool kick, const State &state, Player player, DecisionTable *table, int robot_to_move) {

  Decision decision;

  int rwb = robot_with_ball(state);

  // push an action for the robot with ball, if it's us
  if (player == PLAYER_OF(rwb)) {

    if (kick) {
      decision.action[ID(rwb)] = gen_kick_action(rwb, state);
    } else {
      bool any_pass = false;
#if 0
      auto passees = canGetPass(player);
      if (passees.size() > 0) {
        auto passee = *select_randomly(passees.begin(), passees.end());
        if (passee->getId() != robot_with_ball->getId()) {
          actions.push_back(
              Action::makePass(robot_with_ball->getId(), passee->getId()));
          any_pass = true;
        }
      }
      // in the rare case there isn't any possible pass
      // for the robot with ball, we'll make it move or kick
#endif
      if (!any_pass) {
        auto &action = decision.action[ID(rwb)];
        if (KICK_IF_NO_PASS) {
          action = gen_kick_action(rwb, state);
        } else if (robot_to_move == rwb || robot_to_move == -1) {
          action = gen_move_action(rwb, state);
        } else {
          action = table->move[ID(rwb)];
        }
      }
    }
  }

  // push a Move action for every other robot
  FOR_TEAM_ROBOT(i, player) if (i != rwb) {
    auto &action = decision.action[ID(i)];
    if (robot_to_move == i || robot_to_move == -1) {
      action = gen_move_action(i, state);
    } else {
      action = table->move[ID(i)];
    }
  }

  return decision;
}
