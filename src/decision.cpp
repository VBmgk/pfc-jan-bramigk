#include <stdio.h>

#include "decision.h"
#include "decision_table.h"
#include "state.h"
#include "action.h"
#include "consts.h"
#include "utils.h"
#include "discrete.pb.h"
#include "id_table.h"

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

Decision gen_decision(bool kick, const State &state, Player player, DecisionTable *table, int robot_to_move) {

  Decision decision;

  int rwb = robot_with_ball(state);

  // push an action for the robot with ball, if it's us
  if (player == PLAYER_OF(rwb)) {

    if (kick) {
      decision.action[rwb] = gen_kick_action(rwb, state);
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
        auto &action = decision.action[rwb];
        if (KICK_IF_NO_PASS) {
          action = gen_kick_action(rwb, state);
        } else if (robot_to_move == rwb || robot_to_move == -1) {
          action = gen_move_action(rwb, state);
        } else {
          action = table->move[rwb];
        }
      }
    }
  }

  // push a Move action for every other robot
  FOR_TEAM_ROBOT(i, player) if (i != rwb) {
    auto &action = decision.action[i];
    if (robot_to_move == i || robot_to_move == -1) {
      action = gen_move_action(i, state);
    } else {
      action = table->move[i];
    }
  }

  return decision;
}

Decision from_decision_table(const struct DecisionTable &table) {
  Decision d;
  FOR_N(i, N_ROBOTS) { d.action[i] = table.move[i]; }
  if (table.kick_robot >= 0) {
    d.action[table.kick_robot] = table.kick;
  }
  if (table.pass_robot >= 0) {
    d.action[table.pass_robot] = table.pass;
  }
  return d;
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
