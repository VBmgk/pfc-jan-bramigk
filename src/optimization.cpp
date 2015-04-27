#include <stdio.h>
#include <string.h>
#include <limits>
#include <chrono>

#include "optimization.h"
#include "utils.h"
#include "consts.h"
#include "vector.h"
#include "suggestions.h"

ValuedDecision decide(Optimization &opt, State state, Player player, const Suggestions *suggestions,
                      int *ramification_count) {

  using namespace std::chrono;

  if (!opt.table_initialized) {
    opt.table_initialized = true;
    FOR_EVERY_ROBOT(i) { opt.table.move[i] = make_move_action(state.robots[i]); }
  }

  opt.robot_to_move = ROBOT_WITH_PLAYER((opt.robot_to_move + 1) % N_ROBOTS, player);
  bool kick = can_kick_directly(state, player);

  ValuedDecision vd;
  vd.value = -std::numeric_limits<float>::infinity();

  const duration<double> max_delta{1.0 / DECISION_RATE};
  const auto start = steady_clock::now();

  int i = 0;
  while (true) {
    // FOR_N(i, RAMIFICATION_NUMBER) {
    Decision decision;

    // always consider the previous decision (based on the decision table)
    // unless it's a kick action, those can only happen if kick
    if (suggestions && i < suggestions->tables_count) {
      // decision = from_decision_table(opt.suggestions[i]);
      // TODO
    } else if (i == (suggestions ? suggestions->tables_count : 0) && (kick || opt.table.kick_robot == -1)) {
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

    // check stop condition
    i++;
    if (CONSTANT_RATE) {
      const auto now = steady_clock::now();
      if (now - start >= max_delta)
        break;
    } else if (i >= RAMIFICATION_NUMBER) {
      break;
    }
  }
  *ramification_count = i;

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
