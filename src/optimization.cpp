#include <stdio.h>
#include <limits>

#include "optimization.h"
#include "utils.h"
#include "consts.h"
#include "vector.h"

ValuedDecision stub_decision(const State state, Player player) {
  ValuedDecision vd;

  Player enemy = ENEMY_FOR(player);
  int rwb = robot_with_ball(state);
  float linear_gap = total_gap_len_from_pos(state, state.ball, enemy, rwb);
  float dist_to_goal = dist(state.ball, GOAL_POS(enemy));
  float angular_gap = DEGREES(2 * atan2f(linear_gap / 2, dist_to_goal));
  vd.value = angular_gap;

  bool kick = can_kick_directly(state, player);
  FOR_TEAM_ROBOT(i, player) {
    Vector pos;
    bool the_one = false;
    if (i % N_ROBOTS == 0) {
      the_one = true;
      pos = Vector(-(FIELD_WIDTH / 2 - ROBOT_RADIUS), 0);
    } else {
      pos = Vector(-1.0, (FIELD_HEIGHT - 2 * ROBOT_RADIUS) * ((i % N_ROBOTS) - (N_ROBOTS / 2.0)) / (N_ROBOTS - 2));
    }
    vd.decision.action[i % N_ROBOTS] = make_move_action(pos);
    if (the_one && kick)
      vd.decision.action[i % N_ROBOTS] = make_kick_action(Vector(FIELD_WIDTH / 2, 0));
  }
  return vd;
}

ValuedDecision decide(Optimization &opt, State state, Player player) {

  opt.robot_to_move = ROBOT_WITH_PLAYER((opt.robot_to_move + 1) % N_ROBOTS, player);
  bool kick = can_kick_directly(state, player);

  ValuedDecision vd;
  vd.value = -std::numeric_limits<float>::infinity();

  FOR_N(i, RAMIFICATION_NUMBER) {
    Decision decision = gen_decision(kick, state, player, &opt.table, opt.robot_to_move);

    State next_state = state;
    apply_to_state(decision, player, &next_state);

    float value = evaluate_with_decision(next_state, decision, player);

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
