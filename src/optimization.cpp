#include <stdio.h>
#include <limits>

#include "optimization.h"
#include "utils.h"
#include "consts.h"
#include "vector.h"

ValuedDecision decide(Optimization &opt, State state, Player player) {

  opt.robot_to_move = ROBOT_WITH_PLAYER((opt.robot_to_move + 1) % N_ROBOTS, player);
  bool kick = can_kick_directly(state, player);

  ValuedDecision vd;
  vd.value = -std::numeric_limits<float>::infinity();

  FOR_N(i, RAMIFICATION_NUMBER) {
    Decision decision =
        i == 0 ? from_decision_table(opt.table) : gen_decision(kick, state, player, &opt.table, opt.robot_to_move);

    State next_state = state;
    apply_to_state(decision, player, &next_state);

    float value = evaluate_with_decision(player, next_state, decision, opt.table);

    if (value > vd.value) {
      vd.value = value;
      vd.decision = decision;
      // printf("%i\n", vd.decision.action[0].type);
    }
  }

  // update the decision table
  opt.table.kick_robot = -1;
  opt.table.pass_robot = -1;
  FOR_TEAM_ROBOT(i, player) {
    auto action = vd.decision.action[ID(i)];
    switch (action.type) {
    case KICK: {
      opt.table.kick_robot = i;
      opt.table.kick = action;
    } break;
    case PASS: {
      opt.table.pass_robot = i;
      opt.table.pass = action;
    } break;
    case MOVE: {
      opt.table.move[ID(i)] = action;
    } break;
    case NONE:
      break;
    }
  }

  return vd;
}
