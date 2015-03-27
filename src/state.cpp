#include <cmath>
#include <limits>
#include <algorithm>

#include "utils.h"
#include "state.h"
#include "vector.h"
#include "consts.h"
#include "segment.h"
#include "decision.h"
#include "decision_table.h"
#include "update.pb.h"
#include "id_table.h"

State uniform_rand_state() {
  State s;

  FOR_EVERY_ROBOT(i)
  s.robots[i] = uniform_rand_vector(FIELD_WIDTH, FIELD_HEIGHT);

  s.ball = uniform_rand_vector(FIELD_WIDTH, FIELD_HEIGHT);
  return s;
}

bool can_kick_directly(State state, Player player) {
  int rwb = robot_with_ball(state);
  if (player != PLAYER_OF(rwb))
    return false;

  Player enemy = ENEMY_FOR(player);
  float linear_gap = total_gap_len_from_pos(state, state.ball, enemy, rwb);
  float dist_to_goal = dist(state.ball, GOAL_POS(enemy));
  float angular_gap = DEGREES(2 * atan2f(linear_gap / 2, dist_to_goal));

  if (angular_gap >= MIN_GAP_TO_KICK)
    return true;

  return false;
}

// Function to check which robot has the ball considering only the receiver robot and the enemy robots
#if 0
std::pair<const Robot *, Player>
Board::getRobotWithVirtualBall(const Ball &virt_ball,
                               std::pair<const Robot *, Player> r_rcv) const;
#endif

float time_to_pos(Vector robot_p, Vector robot_v, Vector pos, Vector pos_v) {
  // TODO: take into account robot_v

  /*
   * vb.t + pb = vr.t + pr, t_min? vr?
   * => 0 = (vr^2 - vb^2)t^2 - |pr - pb|^2 - 2.vb.(pb - pr).t
   * => delta = 4.[ (vb.(pb - pr))^2 + (vr^2 - vb^2).|pr - pb|^2]
   * b = -2.vb.(pb - pr) = 2.vb.(pr - pb)
   * b_div_2 = vb.(pr - pb)
   * a = (vr^2 - vb^2)
   * c = -|pr - pb|^2
   * t = -b_div_2 +- sqrt(delta_div_4)
   *    -----------------
   *           a
   */

  // vb.(pb - pr)
  float a = SQ(ROBOT_MAX_SPEED) - norm2(pos_v);
  float c = -norm2(robot_p - pos);
  float b_div_2 = pos_v * (pos - robot_p);
  float delta_div_4 = b_div_2 * b_div_2 - a * c;

  if (a != 0) {
    // It's impossible to reach the ball
    if (delta_div_4 < 0)
      return std::numeric_limits<float>::max();
    else if (delta_div_4 == 0) {
      float t = -b_div_2 / a;

      if (t >= 0)
        return t;
      else
        return std::numeric_limits<float>::max();
    } else {
      // delta_div_4 > 0
      float t1 = (-b_div_2 - sqrt(delta_div_4)) / a, t2 = (-b_div_2 + sqrt(delta_div_4)) / a;

      float t_min = std::min(t1, t2);
      float t_max = std::max(t1, t2);

      if (t_max < 0)
        return std::numeric_limits<float>::max();
      else if (t_min < 0)
        return t_max;
      else
        return t_min;
    }
  } else {
    // 0 = |pr - pb|^2 + 2.vb.(pb - pr).t
    // 0 = [(pr - pb) + 2.vb.t](pb - pr)
    // TODO
    return 0;
  }
}

float time_to_obj(const State state, int r, Vector obj, Vector obj_v) {
  return time_to_pos(state.robots[r], state.robots_v[r], obj, obj_v);
}

int robot_with_ball(const State state) {
  int robot = 0;
  float min_time = std::numeric_limits<float>::max();

  FOR_EVERY_ROBOT(i) {
    float t = time_to_obj(state, i, state.ball, state.ball_v);
    if (t < min_time) {
      robot = i;
      min_time = t;
    }
  }

  return robot;
}

#if 0
std::vector<std::pair<float, float>>
Board::getGoalGaps(Player player, const Body &body) const {
#endif

bool cmp_segments(Segment a, Segment b) { return a.u == b.u ? a.d > b.d : a.u > b.u; }

// returns false if there is no shadow, otherwise return true and write on the given pointer
bool shadow_for_robot_from_pos(Vector rpos, Vector pos, float gx, Segment *shadow) {
  auto d = rpos - pos;
  auto k = norm2(d) - SQ(ROBOT_RADIUS);

  if (k <= 0)
    return false; // FIXME: should return maximum shadow or no gap

  if (d.x * gx <= 0)
    return false;

  float tan_alpha = ROBOT_RADIUS / std::sqrt(k);
  float tan_theta = d.y / std::fabs(d.x);
  float tan_1 = (tan_theta + tan_alpha) / (1 - tan_theta * tan_alpha);
  float tan_2 = (tan_theta - tan_alpha) / (1 + tan_theta * tan_alpha);
  float y_shadow_1 = tan_1 * std::fabs(pos.x - gx) + pos.y;
  float y_shadow_2 = tan_2 * std::fabs(pos.x - gx) + pos.y;

  if ((pos.x - rpos.x + ROBOT_RADIUS) * (pos.x - rpos.x - ROBOT_RADIUS) < 0) {
    if (pos.y > rpos.y)
      y_shadow_2 = -std::numeric_limits<float>::infinity();
    else
      y_shadow_1 = std::numeric_limits<float>::infinity();
  }

  float u_shadow = std::max(y_shadow_1, y_shadow_2);
  float d_shadow = std::min(y_shadow_1, y_shadow_2);

  if (u_shadow <= -GOAL_WIDTH / 2 || d_shadow >= GOAL_WIDTH / 2)
    return false;

  shadow->u = u_shadow;
  shadow->d = d_shadow;
  return true;
}

void discover_gaps_from_pos(const State state, Vector pos, Player player, Segment *gaps, int *gaps_count_ptr,
                            int ignore_robot) {

  float gx = GOAL_X(player);

  // collect shadows

  int shadows_count = 0;
  // Segment shadows[2 * N_ROBOTS];
  auto shadows = gaps;

  FOR_EVERY_ROBOT(i) {
    if (i == ignore_robot)
      continue;

    Segment shadow;
    if (shadow_for_robot_from_pos(state.robots[i], pos, gx, &shadow))
      shadows[shadows_count++] = shadow;
  }

  // sort shadows in descending order by the first parameter (Segment.u)
  std::sort(shadows, shadows + shadows_count, cmp_segments);

  // merge shadows so no shadow overlap
  Segment current_shadow;
  int merged_count = 0;
  bool has_first = false;

  FOR_N(i, shadows_count) {
    Segment shadow = shadows[i];

    if (!has_first) {
      current_shadow = shadow;
      has_first = true;
      continue;
    }

    if (shadow.d >= current_shadow.d) {
      continue;
    }

    if (shadow.u >= current_shadow.d) {
      current_shadow.d = shadow.d;
    } else {
      shadows[merged_count++] = current_shadow;
      current_shadow = shadow;
    }
  }
  if (has_first) {
    shadows[merged_count++] = current_shadow;
  }

  // gather gaps on the goal from merged shadows
  Segment current_gap = {GOAL_WIDTH / 2, -GOAL_WIDTH / 2};
  int gaps_count = 0;
  bool has_last = true;

  FOR_N(i, merged_count) {
    Segment shadow = shadows[i];

    if (shadow.u >= current_gap.u) {
      if (shadow.d <= current_gap.d) {
        has_last = false;
        break;
      }
      current_gap.u = shadow.d;
      continue;
    }

    gaps[gaps_count++] = {current_gap.u, shadow.u};
    if (shadow.d <= current_gap.d) {
      has_last = false;
      break;
    }
    current_gap.u = shadow.d;
  }
  if (has_last) {
    gaps[gaps_count++] = current_gap;
  }

  *gaps_count_ptr = gaps_count;
}

float total_gap_len_from_pos(const State state, Vector pos, Player player, int ignore_robot) {
  int gaps_count;
  Segment gaps[N_ROBOTS * 2]; // this should be enough
  discover_gaps_from_pos(state, pos, player, gaps, &gaps_count, ignore_robot);

  float total_len = 0.0;
  FOR_N(i, gaps_count) { total_len += gaps[i].u - gaps[i].d; }

  return total_len;
}

float max_gap_len_from_pos(const State state, Vector pos, Player player, int ignore_robot) {
  int gaps_count;
  Segment gaps[N_ROBOTS * 2]; // this should be enough
  discover_gaps_from_pos(state, pos, player, gaps, &gaps_count, ignore_robot);

  float max_len = 0.0;
  FOR_N(i, gaps_count) {
    float len = gaps[i].u - gaps[i].d;
    if (len > max_len)
      max_len = len;
  }

  return max_len;
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

float evaluate_with_decision(Player player, const State &state, const struct Decision &decision,
                             const struct DecisionTable &table) {
#if 0
  Player enemy = player == MAX ? MIN : MAX;
  auto enemy_goal = goalPos(enemy);
#endif

  float value = 0.0;

#if 0

  // check who has the ball
  bool has_ball;
  {
    auto _with_ball = getRobotWithBall();
    auto robot_with_ball = _with_ball.first;
    auto player_with_ball = _with_ball.second;
    has_ball = player_with_ball == player;
  }

  // bonus for having the ball
  if (has_ball)
    value += WEIGHT_ATTACK * gap_value(getBall(), enemy);
  // or penalty for not having it
  else
    value -= WEIGHT_BLOCK_ATTACKER * gap_value(getBall(), player);

  // bonus for seeing enemy goal
  for (auto &robot : getTeam(player).getRobots()) {
    value += WEIGHT_SEE_ENEMY_GOAL * gap_value(robot, enemy);

    // penalty for being too close to enemy goal
    if (robot.getDist(enemy_goal) < DIST_GOAL_TO_PENAL) {
      value -= DIST_GOAL_PENAL;
    }
  }
#endif

  // penalty for exposing own goal
  FOR_TEAM_ROBOT(i, ENEMY_FOR(player)) { value -= WEIGHT_BLOCK_GOAL * gap_value(state, player, state.robots[i]); }

#if 0
  // bonus for having more robots able to receive a pass
  if (has_ball) {
    float receivers_num = canGetPass(player).size();
    value += WEIGHT_RECEIVERS_NUM * receivers_num;
  }
#endif

  float move_dist_total = 0, move_dist_max = 0, move_change = 0, pass_change = 0, kick_change = 0;

  FOR_TEAM_ROBOT(i, player) {
    auto action = decision.action[i];
    auto rpos = state.robots[i];
    switch (action.type) {
    case MOVE: {
      float move_dist = norm(action.move_pos - rpos);
      move_dist_max = std::max(move_dist_max, move_dist);
      move_dist_total += move_dist;
      move_change += norm(action.move_pos - table.move[i].move_pos);
    } break;
    case PASS:
      if (table.pass_robot >= 0) {
        // XXX: assuming everything is ok and the receiver has a move action
        pass_change +=
            norm(decision.action[action.pass_receiver].move_pos - table.move[table.pass.pass_receiver].move_pos);
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

  value -= WEIGHT_MOVE_DIST_TOTAL * move_dist_total;
  value -= WEIGHT_MOVE_DIST_MAX * move_dist_max;
  value -= WEIGHT_MOVE_CHANGE * move_change;
  value -= WEIGHT_PASS_CHANGE * pass_change;
  value -= WEIGHT_KICK_CHANGE * kick_change;

  return value;
}

void update_from_proto(State &state, roboime::Update &ptb_update, IdTable &table) {
  // TODO: more reliable mapping, what may happen now is that the mapping {0:4, 1:5} may change to {0:5, 1:4} simply
  // because of ordering

  auto ball = ptb_update.ball();
  state.ball = {ball.x(), ball.y()};
  state.ball_v = {ball.vx(), ball.vy()};

  FOR_N(i, ptb_update.min_team_size()) {
    auto robot = ptb_update.min_team(i);
    int r = ROBOT_WITH_PLAYER(i, MIN);
    table.id[r] = robot.i();
    state.robots[r] = {robot.x(), robot.y()};
    state.robots_v[r] = {robot.vx(), robot.vy()};
  }

  FOR_N(i, ptb_update.max_team_size()) {
    auto robot = ptb_update.max_team(i);
    int r = ROBOT_WITH_PLAYER(i, MAX);
    table.id[r] = robot.i();
    state.robots[r] = {robot.x(), robot.y()};
    state.robots_v[r] = {robot.vx(), robot.vy()};
  }
}
