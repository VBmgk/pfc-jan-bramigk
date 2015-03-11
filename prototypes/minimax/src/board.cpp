#include <memory>
#include <vector>
#include <cfloat>
#include <random>
#include <algorithm>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

static std::random_device rd;
std::mt19937 Vector::generator(rd());

bool Board::isGameOver(Player player) const {
  auto with_ball = getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  auto player_with_ball = with_ball.second;

  if (player != player_with_ball)
    return false;

  float total_gap = totalGoalGap(player == MAX ? MIN : MAX, getBall());
  auto enemy_goal = enemyGoalPos(player);
  float distance_to_goal = getBall().getDist(enemy_goal);
  float value = atan2f(total_gap / 2, distance_to_goal);

  if (value >= M_PI * MIN_GAP_TO_KICK / 180) {
    return true;
  }

  return false;
}

TeamAction Board::genKickTeamAction(Player p, MoveTable t, int move_id) const {
  return genActions(p, true, t, move_id);
}

TeamAction Board::genPassTeamAction(Player p, MoveTable t, int move_id) const {
  return genActions(p, false, t, move_id);
}

TeamAction Board::genKickTeamAction(Player p, MoveTable t) const {
  return genActions(p, true, t);
}

TeamAction Board::genPassTeamAction(Player p, MoveTable t) const {
  return genActions(p, false, t);
}

TeamAction Board::genKickTeamAction(Player p) const {
  return genActions(p, true, MoveTable());
}

TeamAction Board::genPassTeamAction(Player p) const {
  return genActions(p, false, MoveTable());
}

// BEGIN util for random acces on iterator
// reference: http://stackoverflow.com/a/16421677/947511
template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter begin, Iter end, RandomGenerator &g) {
  std::uniform_int_distribution<> dis(0, std::distance(begin, end) - 1);
  std::advance(begin, dis(g));
  return begin;
}
template <typename Iter> Iter select_randomly(Iter start, Iter end) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return select_randomly(start, end, gen);
}
// END

TeamAction Board::genActions(Player player, bool kickAction,
                             MoveTable move_table, int move_id) const {
  TeamAction actions;

  auto with_ball = getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  auto player_with_ball = with_ball.second;

  // push an action for the robot with ball, if it's us
  if (player_with_ball == player) {
    // Kick
    if (kickAction) {
      actions.push_back(Action::genKick(*robot_with_ball, player, *this));
      // Pass
    } else {
      bool any_pass = false;
      auto passees = canGetPass(player);
      if (passees.size() > 0) {
        auto passee = *select_randomly(passees.begin(), passees.end());
        actions.push_back(
            Action::makePass(robot_with_ball->getId(), passee->getId()));
        any_pass = true;
      }
      // in the rare case there isn't any possible pass
      // for the robot with ball, we'll make it move or kick
      if (!any_pass) {
        Action action;
        if (KICK_IF_NO_PASS) {
          action = Action::genKick(*robot_with_ball, player, *this);
        } else {
          int robot_id = robot_with_ball->getId();
          if (move_table.count(robot_id) > 0)
            action = move_table[robot_id];
          else
            action =
                Action::genMove(*robot_with_ball, player, *this, move_table);
        }
        actions.push_back(action);
      }
    }
  }

  // push a Move action for every other robot
  for (auto robot : getOtherRobots(player, *robot_with_ball)) {
    int robot_id = robot->getId();

    auto action = Action::genMove(*robot, player, *this, move_table);
    if (move_id == robot_id) {
      actions.push_back(action);
    } else if (move_table.count(robot_id) > 0) {
      actions.push_back(move_table[robot_id]);
    } else {
      actions.push_back(action);
    }
  }

  return actions;
}

std::pair<const Robot *, Player> Board::getRobotWithBall() const {
  return getRobotWithVirtualBall(ball);
}

std::pair<const Robot *, Player>
Board::getRobotWithVirtualBall(const Ball &virt_ball) const {
  return getRobotWithVirtualBall(
      virt_ball, std::make_pair(nullptr, MIN)); // second valeu is useless
}

// Function to check who has the ball considering
// only the receiver robot and the enemy robots
std::pair<const Robot *, Player>
Board::getRobotWithVirtualBall(const Ball &virt_ball,
                               std::pair<const Robot *, Player> r_rcv) const {
  float min_time = FLT_MAX;
  float time;

  const Robot *robot_with_ball = nullptr;
  Player player_with_ball = r_rcv.second;

#define FOR_ROBOT_IN_TEAM(TEAM, PLAYER)                                        \
  for (const Robot &robot : TEAM) {                                            \
    time = timeToVirtualBall(robot, virt_ball);                                \
    if (time < min_time) {                                                     \
      robot_with_ball = &robot;                                                \
      player_with_ball = PLAYER;                                               \
      min_time = time;                                                         \
    }                                                                          \
  }

  // normal case, consider both teams
  if (r_rcv.first == nullptr) {
    FOR_ROBOT_IN_TEAM(min.robots, MIN)
    FOR_ROBOT_IN_TEAM(max.robots, MAX)
  } else if (MAX == r_rcv.second)
    FOR_ROBOT_IN_TEAM(min.robots, MIN)
  else
    FOR_ROBOT_IN_TEAM(max.robots, MAX)
#undef FOR_ROBOT_IN_TEAM

  if (r_rcv.first != nullptr &&
      timeToVirtualBall(*r_rcv.first, virt_ball) < min_time) {
    robot_with_ball = r_rcv.first;
  }

  return std::make_pair(robot_with_ball, player_with_ball);
}

float Board::timeToBall(const Robot &robot) const {
  return timeToVirtualBall(robot, ball);
}

float Board::timeToVirtualBall(const Robot &robot,
                               const Ball &virt_ball) const {
  return timeToVirtualBall(robot.pos(), robot.maxV2(), virt_ball);
}

float Board::timeToVirtualBall(const Vector &pos, float maxV2,
                               const Ball &virt_ball) const {
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
  float a = (maxV2 - virt_ball.v() * virt_ball.v());
  float c = -((pos - virt_ball.pos()) * (pos - virt_ball.pos()));
  float b_div_2 = virt_ball.v() * (ball.pos() - pos);
  float delta_div_4 = b_div_2 * b_div_2 - a * c;

  if (a != 0) {
    // It's impossible to reach the ball
    if (delta_div_4 < 0)
      return FLT_MAX;
    else if (delta_div_4 == 0) {
      float t = -b_div_2 / a;

      if (t >= 0)
        return t;
      else
        return FLT_MAX;
    } else {
      // delta_div_4 > 0
      float t1 = (-b_div_2 - sqrt(delta_div_4)) / a,
            t2 = (-b_div_2 + sqrt(delta_div_4)) / a;

      float t_min = std::min(t1, t2);
      float t_max = std::max(t1, t2);

      if (t_max < 0)
        return FLT_MAX;
      else if (t_min < 0)
        return t_max;
      else
        return t_min;
    }
  } else {
    // 0 = |pr - pb|^2 + 2.vb.(pb - pr).t
    // 0 =[(pr - pb) + 2.vb.t](pb - pr)
    // TODO
    return 0;
  }
}

Board Board::virtualStep(float time) const {
  Board n_board;
  n_board.ball = Ball(ball.pos() + ball.v() * time, ball.v());

  for (auto &robot : min.getRobots())
    n_board.min.addRobot(
        Robot(robot.getId(), robot.pos() + robot.v() * time, robot.v()));

  for (auto &robot : max.getRobots())
    n_board.max.addRobot(
        Robot(robot.getId(), robot.pos() + robot.v() * time, robot.v()));

  return n_board;
}

std::vector<const Robot *> Board::canGetPass(Player player) const {
  std::vector<const Robot *> robots;

  auto with_ball = getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  auto player_with_ball = with_ball.second;

  // XXX: work around
  // for (auto &robot : getTeam(player_with_ball).getRobots()) {
  //  if (&robot == robot_with_ball)
  //    continue;
  //  robots.push_back(&robot);
  //}
  // return robots;

  // it only makes sense if the player has the ball
  if (player_with_ball == player) {
    float step_time = timeToBall(*robot_with_ball);
    Board vrt_board = virtualStep(step_time);

    // test against every friend except for self
    for (auto &robot : vrt_board.getTeam(player).getRobots()) {
      if (&robot == robot_with_ball)
        continue;

      // create a virtual ball with future position
      Ball vrt_ball = vrt_board.ball;
      vrt_ball.setV((vrt_ball.pos() - robot.pos()).unit() * Robot::kickV());

      // Add robot if the same robot will have the ball after kick
      auto with_ball =
          getRobotWithVirtualBall(vrt_ball, std::make_pair(&robot, player));
      auto robot_with_ball_after_kick = with_ball.first;
      if (&robot == robot_with_ball_after_kick)
        robots.push_back(&robot);
    }
  }

  return robots;
}

float Board::totalGoalGap(Player player, const Body &body) const {
  float len = 0;
  for (auto gap : getGoalGaps(player, body))
    len += std::fabs(gap.first - gap.second);
  return len;
}

float Board::maxGoalGap(Player player, const Body &body) const {
  float len = 0;
  for (auto gap : getGoalGaps(player, body)) {
    auto this_len = std::fabs(gap.first - gap.second);
    if (this_len > len)
      len = this_len;
  }
  return len;
}

std::vector<std::pair<float, float>>
Board::getGoalGaps(Player player, const Body &body) const {
  auto gx = goalPos(player)[0];

  // collect shadows

  std::vector<std::pair<float, float>> shadows;
  for (auto _robot : getRobotsMoving()) {
    auto &robot = *_robot;
    auto d = robot.pos() - body.pos();
    auto k = d * d - Robot::radius() * Robot::radius();

    if (k <= 0)
      continue; // FIXME: should return maximum shadow or no gap
    if (d[0] * gx <= 0)
      continue;

    float tan_alpha = Robot::radius() / std::sqrt(k);
    float tan_theta = d[1] / std::fabs(d[0]);
    float tan_1 = (tan_theta + tan_alpha) / (1 - tan_theta * tan_alpha);
    float tan_2 = (tan_theta - tan_alpha) / (1 + tan_theta * tan_alpha);
    float y_shadow_1 = tan_1 * std::fabs(body.pos()[0] - gx) + body.pos()[1];
    float y_shadow_2 = tan_2 * std::fabs(body.pos()[0] - gx) + body.pos()[1];

    if ((body.pos()[0] - robot.pos()[0] + Robot::radius()) *
            (body.pos()[0] - robot.pos()[0] - Robot::radius()) <
        0) {
      if (body.pos()[1] > robot.pos()[1])
        y_shadow_2 = -std::numeric_limits<float>::infinity();
      else
        y_shadow_1 = std::numeric_limits<float>::infinity();
    }

    float u_shadow = std::max(y_shadow_1, y_shadow_2);
    float d_shadow = std::min(y_shadow_1, y_shadow_2);

    if (u_shadow <= -goalWidth() / 2 || d_shadow >= goalWidth() / 2)
      continue;

    shadows.push_back(std::make_pair(u_shadow, d_shadow));
  }

  // sort shadows in descending order by the first parameter

  std::sort(shadows.begin(), shadows.end(),
            [](std::pair<float, float> s1,
               std::pair<float, float> s2) { return s1 > s2; });

  // merge shadows so no shadow overlap

  std::vector<std::pair<float, float>> shadows_merged;
  std::pair<float, float> current_shadow;
  bool has_first = false;
  for (auto shadow : shadows) {
    if (!has_first) {
      current_shadow = shadow;
      has_first = true;
      continue;
    }

    if (shadow.second >= current_shadow.second) {
      continue;
    }

    if (shadow.first >= current_shadow.second) {
      current_shadow = std::make_pair(current_shadow.first, shadow.second);
    } else {
      shadows_merged.push_back(current_shadow);
      current_shadow = shadow;
    }
  }
  if (has_first) {
    shadows_merged.push_back(current_shadow);
  }

  // gather gaps on the goal from merged shadows

  std::vector<std::pair<float, float>> gaps;
  std::pair<float, float> current_gap =
      std::make_pair(goalWidth() / 2, -goalWidth() / 2);
  bool has_last = true;
  for (auto shadow : shadows_merged) {

    if (shadow.first >= current_gap.first) {
      if (shadow.second <= current_gap.second) {
        has_last = false;
        break;
      }
      current_gap.first = shadow.second;
      continue;
    }

    gaps.push_back(std::make_pair(current_gap.first, shadow.first));
    if (shadow.second <= current_gap.second) {
      has_last = false;
      break;
    }
    current_gap.first = shadow.second;
  }
  if (has_last) {
    gaps.push_back(current_gap);
  }

  return gaps;
}

// bool Board::freeKickLine(int point_index) const {
//  for (auto robot : getRobotsMoving())
//    if (kickLineCrossRobot(point_index, *robot))
//      return false;
//  return true;
//}

std::vector<const Robot *> Board::getRobotsMoving() const {
  auto robot_with_ball = getRobotWithBall().first;
  auto robots = getOtherRobots(MIN, *robot_with_ball);
  auto robots2 = getOtherRobots(MAX, *robot_with_ball);
  robots.insert(robots.end(), robots2.begin(), robots2.end());
  return robots;
}

// bool Board::kickLineCrossRobot(const int point_index,
//                               const Robot &robot) const {
//  auto robot_player = getRobotWithBall();
//  auto robot_with_ball = robot_player.first;
//  auto player_with_ball = robot_player.second;
//
//  Vector point(0,
//               goalWidth() * (1 / 2 - point_index * 1.0 / NUM_SAMPLE_POINTS));
//
//  point = enemyGoalPos(player_with_ball) + point;
//
//  if (Vector::lineSegmentCrossCircle(point, robot_with_ball->pos(),
//  robot.pos(),
//                                     Robot::radius()))
//    return true;
//
//  return false;
//}

float Board::evaluate() const {
  // special case for it not to crash when no robots on a team
  if (min.size() == 0 || max.size() == 0)
    return 0.0;

  // return evaluate_for(MAX) - evaluate_for(MIN);
  return evaluate_for(MAX);
}

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

float Board::evaluate_for(Player player) const {
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
}

Board Board::applyTeamAction(const TeamAction &max_a,
                             const TeamAction &min_a) const {
  Board new_board(*this);

#define APPLY_TEAM_ACTION(T, TA)                                               \
  /* apply all moves first */                                                  \
  for (auto action : TA) {                                                     \
    if (action.type == MOVE) {                                                 \
      applyAction(action, T, new_board);                                       \
    }                                                                          \
  }                                                                            \
  for (auto action : TA) {                                                     \
    if (action.type != MOVE) {                                                 \
      applyAction(action, T, new_board);                                       \
    }                                                                          \
  }
  APPLY_TEAM_ACTION(MAX, max_a)
  APPLY_TEAM_ACTION(MIN, min_a)
#undef APPLY_TEAM_ACTION

  return new_board;
}

float Board::teamActionTime(const TeamAction &actions) const {
  return 1.0;
  // XXX: TODO: action times
  float time = FLT_MIN;

  for (auto &action : actions) {
    // float a_time = action->getTime();
    float a_time = 0;
    if (a_time > time)
      time = a_time;
  }

  return time;
}

std::vector<const Robot *>
Board::getOtherRobots(Player player, const Robot &robot_with_ball) const {
  // preallocate memory
  std::vector<const Robot *> robots;

  for (auto &robot : getTeam(player).getRobots()) {
    if (&robot != &robot_with_ball)
      robots.push_back(&robot);
  }

  return robots;
}

// print operator for Vector
std::ostream &operator<<(std::ostream &os, const Vector &v) {
  os << "\t" << v.x() << "\n\t" << v.y() << "\n";

  return os;
}
