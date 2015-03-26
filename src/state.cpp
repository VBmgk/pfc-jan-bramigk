#include <cmath>
#include <limits>
#include <algorithm>

#include "utils.h"
#include "state.h"
#include "vector.h"
#include "consts.h"
#include "segment.h"

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

#if 0
float Board::gap_value(Body object, Player player) const {
  auto goal = goalPos(player);
  float distance_to_goal = getBall().getDist(goal);
  float total_gap =
      2 * atan2f(totalGoalGap(player, object) / 2, distance_to_goal);
  if (total_gap < 0)
    total_gap = 0;
  if (total_gap > M_PI)
    total_gap = M_PI;
  float max_gap = 2 * atan2f(maxGoalGap(player, object) / 2, distance_to_goal);
  if (max_gap < 0)
    max_gap = 0;
  if (max_gap > M_PI)
    max_gap = M_PI;
  return TOTAL_MAX_GAP_RATIO * total_gap + (1 - TOTAL_MAX_GAP_RATIO) * max_gap;
}
#endif

float evaluate_with_decision(const State &state, const Decision &decision, Player player) {
#if 0
  Player enemy = player == MAX ? MIN : MAX;
  auto enemy_goal = goalPos(enemy);

  float value = 0.0;

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

  // penalty for exposing own goal
  for (auto &robot : getTeam(enemy).getRobots()) {
    value -= WEIGHT_BLOCK_GOAL * gap_value(robot, player);
  }

  // bonus for having more robots able to receive a pass
  if (has_ball) {
    float receivers_num = canGetPass(player).size();
    value += WEIGHT_RECEIVERS_NUM * receivers_num;
  }

  return value;
#endif
  static float val = 99.0;
  return val++;
}
