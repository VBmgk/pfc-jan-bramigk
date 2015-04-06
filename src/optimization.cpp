#include <stdio.h>
#include <limits>

#include "optimization.h"
#include "utils.h"
#include "consts.h"
#include "vector.h"

ValuedDecision decide(Optimization &opt, State state, Player player) {

  if (!opt.table_initialized) {
    opt.table_initialized = true;
    FOR_EVERY_ROBOT(i) { opt.table.move[i] = make_move_action(state.robots[i]); }
  }

  opt.robot_to_move = ROBOT_WITH_PLAYER((opt.robot_to_move + 1) % N_ROBOTS, player);
  bool kick = can_kick_directly(state, player);

  ValuedDecision vd;
  vd.value = -std::numeric_limits<float>::infinity();

  FOR_N(i, RAMIFICATION_NUMBER) {
    Decision decision;

    // always consider the previous decision (based on the decision table)
    if (i == 0) {
      decision = from_decision_table(opt.table);
      // on some cases try to move everyone at once, this may lead to better results
    } else if (100.0 * i / RAMIFICATION_NUMBER < FULL_CHANGE_PERCENTAGE) {
      decision = gen_decision(kick, state, player, opt.table);
      // on everything else roun-robin between trying to move each robot
    } else {
      decision = gen_decision(kick, state, player, opt.table, opt.robot_to_move);
    }

    State next_state = state;
    apply_to_state(decision, player, &next_state);

    float value = evaluate_with_decision(player, next_state, decision, opt.table);

    if (value > vd.value) {
      vd.value = value;
      vd.decision = decision;
    }
  }

  // update the decision table
  opt.table.kick_robot = -1;
  opt.table.pass_robot = -1;
  FOR_TEAM_ROBOT(i, player) {
    auto action = vd.decision.action[i];
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
      opt.table.move[i] = action;
    } break;
    case NONE:
      break;
    }
  }

  return vd;
}
