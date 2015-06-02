#include <stdio.h>
#include <string.h>
#include <cmath>
#include <limits>
#include <chrono>
#include <algorithm>

#include "optimization.h"
#include "utils.h"
#include "consts.h"
#include "vector.h"
#include "suggestions.h"
#include "decision_source.h"
#include "app.h"

ValuedDecision decide(Optimization &opt, State state, Player player,
                      Suggestions *suggestions, int *ramification_count) {

  using namespace std::chrono;

  if (!opt.table_initialized) {
    opt.table_initialized = true;
    FOR_EVERY_ROBOT(i) {
      opt.table.move[i] = make_move_action(state.robots[i]);
    }
  }

  opt.robot_to_move =
      ROBOT_WITH_PLAYER((opt.robot_to_move + 1) % N_ROBOTS, player);
  bool kick = can_kick_directly(state, player);

  ValuedDecision best_vd;
  best_vd.value = -std::numeric_limits<float>::infinity();

  const duration<double> max_delta{1.0 / DECISION_RATE};
  const auto start = steady_clock::now();

  // this should point to a suggestion if one leads to the best decision
  SuggestionTable *best_suggestion = nullptr;
  int best_suggestion_i = -1;
  DecisionSource best_source = NO_SOURCE;

  int i = 0;
  while (true) {
    // FOR_N(i, RAMIFICATION_NUMBER) {
    Decision decision;
    ValuedDecision vd;
    DecisionSource source;
    SuggestionTable *local_suggestion = nullptr;
    int local_suggestion_i = -1;

    // always consider the previous decision (based on the decision
    // table)
    // unless it's a kick action, those can only happen if kick
    if (suggestions && i < suggestions->tables_count) {
      local_suggestion = &suggestions->tables[i];
      local_suggestion_i = i;
      vd.decision =
          gen_decision(kick, *local_suggestion, &state, opt.table, player);
      source = SUGGESTION;
    } else if (i == (suggestions ? suggestions->tables_count : 0)) {
      vd.decision = from_decision_table(opt.table, state, player, kick);
      source = TABLE;
      // on some cases try to move everyone at once, this may lead to
      // better
      // results
    } else if (100.0 * i / RAMIFICATION_NUMBER < FULL_CHANGE_PERCENTAGE) {
      vd.decision = gen_decision(kick, state, player, opt.table);
      source = FULL_RANDOM;
      // on everything else roun-robin between trying to move each robot
    } else {
      vd.decision =
          gen_decision(kick, state, player, opt.table, opt.robot_to_move);
      source = SINGLE_RANDOM;
    }

    vd.value = evaluate_with_decision(player, state, vd.decision, opt.table,
                                      vd.values);

    if (FINE_OPTIMIZE == OPTIMIZE_ALL) {
      vd = optimize_decision(player, state, vd, opt.table);
    }

    if (vd.value > best_vd.value) {
      best_vd = vd;
      // best_vd.value = value;
      // FOR_N(i, W_SIZE) best_vd.values[i] = values[i];
      // memcpy(best_vd.values, values, W_SIZE * sizeof(*values));
      // best_vd.decision = decision;
      // save suggestion or otherwise erase it
      best_suggestion = local_suggestion;
      best_suggestion_i = local_suggestion_i;
      best_source = source;
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

  // optimize the best decision
  if (FINE_OPTIMIZE == OPTIMIZE_BEST) {
    best_vd = optimize_decision(player, state, best_vd, opt.table);
  }

  // increment the usage count if decision from a suggestion
  if (best_suggestion) {
    best_suggestion->usage_count++;
    suggestions->last_used = best_suggestion_i;
  }

  *app_decision_source = best_source;

  // update the decision table
  opt.table.kick_robot = -1;
  opt.table.pass_robot = -1;
  FOR_TEAM_ROBOT(i, player) {
    auto action = best_vd.decision.action[i];
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

  return best_vd;
}

float gap_value(const State state, Player player, Vector pos) {
  Vector goal = GOAL_POS(player);
  float dist_to_goal = dist(pos, goal);

  float total_gap_linear = total_gap_len_from_pos(state, pos, player);
  float total_gap = DEGREES(2 * atan2f(total_gap_linear / 2, dist_to_goal));
  while (total_gap < 0)
    total_gap += 360;
  while (total_gap > 360)
    total_gap -= 360;

  float max_gap_linear = max_gap_len_from_pos(state, pos, player);
  float max_gap = DEGREES(2 * atan2f(max_gap_linear / 2, dist_to_goal));
  while (max_gap < 0)
    max_gap += 360;
  while (max_gap > 360)
    max_gap -= 360;

  return TOTAL_MAX_GAP_RATIO * total_gap + (1 - TOTAL_MAX_GAP_RATIO) * max_gap;
}

float evaluate_with_decision(Player player, const State &state,
                             const struct Decision &decision,
                             const struct DecisionTable &table, float *values) {
  State next_state = state;
  apply_to_state(decision, player, &next_state);

  float dumb_values[W_SIZE];
  if (values == nullptr)
    values = dumb_values;

  Player enemy = ENEMY_FOR(player);

  float time_min, time_max;
  int rwb_min, rwb_max;

  // check whether we have the ball
  int rwb =
      robot_with_ball(next_state, &time_min, &time_max, &rwb_min, &rwb_max);
  bool has_ball = PLAYER_OF(rwb) == player;

  float time_player = player == MAX ? time_max : time_min;
  float time_enemy = player == MIN ? time_max : time_min;
  int rwb_player = player == MAX ? rwb_max : rwb_min;
  // int rwb_enemy = player == MIN ? rwb_max : rwb_min;

  float value = 0.0;
#define W(NAME, VAL)                                                           \
  do {                                                                         \
    float v = NAME * VAL;                                                      \
    values[_##NAME] += v;                                                      \
    value += v;                                                                \
  } while (false)

  W(WEIGHT_CLOSE_TO_BALL, 1 / (1 + time_player));
  W(WEIGHT_ENEMY_CLOSE_TO_BALL, -1 / (1 + time_enemy));
  W(WEIGHT_BALL_POS, next_state.ball.x);

  // bonus for having the ball
  if (has_ball) {
    W(WEIGHT_HAS_BALL, 1);
  }

  W(WEIGHT_ATTACK, gap_value(next_state, enemy, next_state.ball));
  W(WEIGHT_BLOCK_ATTACKER, -gap_value(next_state, player, next_state.ball));

  // penalty for exposing own goal
  FOR_TEAM_ROBOT(i, enemy) {
    W(WEIGHT_BLOCK_GOAL, -gap_value(next_state, player, next_state.robots[i]));
  }

  // bonus for having more robots able to receive a pass
  TeamFilter receivers;
  discover_possible_receivers(next_state, &table, player, receivers,
                              has_ball ? rwb : -1);
  W(WEIGHT_RECEIVERS_NUM, receivers.count);

  // penalty for having enemies able to receive a pass
  TeamFilter enemy_receivers;
  discover_possible_receivers(next_state, &table, enemy, enemy_receivers,
                              has_ball ? -1 : rwb);
  W(WEIGHT_ENEMY_RECEIVERS_NUM, -enemy_receivers.count);

  // bonus for seeing enemy goal
  float best_receiver = 0;
  FOR_TEAM_ROBOT(i, player) {
    float gap = gap_value(next_state, enemy, next_state.robots[i]);
    auto robot = next_state.robots[i];
    W(WEIGHT_SEE_ENEMY_GOAL, gap);

    if (rwb_player != i && !receivers[i] &&
        norm2(robot - GOAL_POS(enemy)) > SQ(DEFENSE_RADIUS)) {
      float this_gap = fmin(0.1, gap);
      float good_receiver =
          this_gap /
          (1 + SQ(DESIRED_PASS_DIST - norm(next_state.ball - robot)));
      good_receiver += robot.x + FIELD_WIDTH / 2;
      if (best_receiver < good_receiver)
        best_receiver = good_receiver;
    }

    // penalty for being too close to enemy goal
    if (dist(next_state.robots[i], GOAL_POS(enemy)) < DIST_GOAL_TO_PENAL) {
      value -= DIST_GOAL_PENAL;
      values[_WEIGHT_PENALS] -= DIST_GOAL_PENAL;
    }
  }
  W(WEIGHT_GOOD_RECEIVERS, best_receiver);

  float move_dist_total = 0, move_dist_max = 0, move_change = 0,
        pass_change = 0, kick_change = 0;

  FOR_TEAM_ROBOT(i, player) {
    auto action = decision.action[i];
    auto rpos = state.robots[i];
    switch (action.type) {
    case MOVE: {
      float move_dist = norm(action.move_pos - rpos);
      move_dist_max = std::max(move_dist_max, move_dist);
      move_dist_total += move_dist;

      auto mvec = table.move[i].move_pos - rpos;
      auto nvec = action.move_pos - rpos;
      auto mnd = sqrt(norm2(mvec) * norm2(nvec));
      if (move_dist > ROBOT_RADIUS && mnd > SQ(ROBOT_RADIUS)) {
        // move_change += norm(action.move_pos -
        // table.move[i].move_pos);
        float c = mvec * nvec / mnd;
        if (c - 1.0 < 0.00001)
          c = 1.0;
        move_change += acos(c);
      }
    } break;
    case PASS:
      if (table.pass_robot >= 0) {
        // XXX: assuming everything is ok and the receiver has a move
        // action
        pass_change += norm(decision.action[action.pass_receiver].move_pos -
                            table.move[table.pass.pass_receiver].move_pos);
      }
      break;
    case KICK:
      if (table.kick_robot >= 0) {
        kick_change += norm(action.kick_pos - table.move[i].kick_pos);
      }
      break;
    case NONE:
      break;
    }
  }

  W(WEIGHT_MOVE_DIST_TOTAL, -move_dist_total);
  W(WEIGHT_MOVE_DIST_MAX, -move_dist_max);
  W(WEIGHT_MOVE_CHANGE, -move_change);
  W(WEIGHT_PASS_CHANGE, -pass_change);
  W(WEIGHT_KICK_CHANGE, -kick_change);

#undef W
  return value;
}

Gradient evaluate_with_decision_gradient(Player player, const State &state,
                                         const Decision &decision,
                                         const DecisionTable &table) {

  static constexpr float EPSILON =
      std::numeric_limits<float>::epsilon() * FIELD_WIDTH;
  Gradient grad;

  FOR_TEAM_ROBOT(i, player) {
    auto action = decision.action[i];
    if (action.type != MOVE)
      continue;

    // x+ɛ
    Decision xp_decision{decision};
    xp_decision.action[i].move_pos.x += EPSILON;
    float xp_val = evaluate_with_decision(player, state, xp_decision, table);

    // x-ɛ
    Decision xm_decision{decision};
    xm_decision.action[i].move_pos.x -= EPSILON;
    float xm_val = evaluate_with_decision(player, state, xm_decision, table);

    // ∂x
    grad.deltas[i].x = (xp_val - xm_val) / (2 * EPSILON);

    // y+ɛ
    Decision yp_decision{decision};
    yp_decision.action[i].move_pos.y += EPSILON;
    float yp_val = evaluate_with_decision(player, state, yp_decision, table);

    // y-ɛ
    Decision ym_decision{decision};
    ym_decision.action[i].move_pos.y -= EPSILON;
    float ym_val = evaluate_with_decision(player, state, ym_decision, table);

    // ∂y
    grad.deltas[i].y = (yp_val - ym_val) / (2 * EPSILON);
  }

#if 0
  printf("grad =\n");
  FOR_TEAM_ROBOT(i, player) {
    printf("  delta[%02i] = %f, %f\n", i, grad.deltas[i].x, grad.deltas[i].y);
  }
#endif

  return grad;
}

ValuedDecision optimize_decision(Player player, const State &state,
                                 const ValuedDecision &valued_decision,
                                 const DecisionTable &table) {

#if 1
  // Gradient descent (sort of)
  Gradient grad = evaluate_with_decision_gradient(
      player, state, valued_decision.decision, table);

  ValuedDecision opt_vd;
  static constexpr int MAX_IT = 16;
  static constexpr float GAMMA = 0.4e-4;
  float gamma = GAMMA;

  FOR_N(i, MAX_IT) {
    opt_vd = valued_decision;

    // add gamma * grad to opt_vd
    FOR_TEAM_ROBOT(j, player) {
      auto &action = opt_vd.decision.action[j];
      if (action.type == MOVE) {
        action.move_pos += grad.deltas[j] * gamma;
      }
    }

    opt_vd.value = evaluate_with_decision(player, state, opt_vd.decision, table,
                                          opt_vd.values);

    if (opt_vd.value > valued_decision.value) {
      goto optimized;
    }

    gamma /= 2;
  }

  opt_vd = valued_decision;
  gamma = -1;

optimized:
  printf("gamma = %g\n", gamma);

  return opt_vd;
#endif
}
