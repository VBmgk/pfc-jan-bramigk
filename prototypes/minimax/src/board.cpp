#include <memory>
#include <vector>
#include <armadillo>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

bool Board::isGameOver() const {
  if (openGoalArea() > MIN_AREA_TO_MARK)
    return true;

  return false;
}

TeamAction Board::genKickTeamAction(Player p) const {
  return genActions(p, true);
}

TeamAction Board::genPassTeamAction(Player p) const {
  return genActions(p, false);
}

TeamAction Board::genActions(Player player, bool kickAction) const {
  TeamAction actions;

  auto with_ball = getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  auto player_with_ball = with_ball.second;

  // push an action for the robot with ball, if it's us
  if (player_with_ball == player) {
    // Kick
    if (kickAction) {
      std::shared_ptr<Action> action(new Kick(*robot_with_ball));
      actions.push_back(std::move(action));
      // Pass
    } else {
      bool any_pass = false;
      for (auto robot : canGetPass(player)) {
        std::shared_ptr<Action> action(new Pass(*robot_with_ball, *robot));
        actions.push_back(std::move(action));
        any_pass = true;
        // FIXME: only a single action per robot!
        //        there has to be many TeamAction's for this
        break;
      }
      // in the rare case there isn't any possible pass
      // for the robot with ball, we'll make it move
      if (!any_pass) {
        std::shared_ptr<Action> action(new Move(*robot_with_ball));
        actions.push_back(std::move(action));
      }
    }
  }

  // push a Move action for every other robot
  for (auto robot : getOtherRobots(player, *robot_with_ball)) {
    std::shared_ptr<Action> action(new Move(*robot));
    actions.push_back(std::move(action));
  }

  return actions;
}

std::pair<const Robot *, Player> Board::getRobotWithBall() const {
  return getRobotWithVirtualBall(ball);
}

std::pair<const Robot *, Player>
Board::getRobotWithVirtualBall(const Ball &virt_ball) const {
  return getRobotWithVirtualBall(virt_ball, nullptr);
}

std::pair<const Robot *, Player>
Board::getRobotWithVirtualBall(const Ball &virt_ball,
                               const Robot *r_rcv) const {
  float min_time = FLT_MAX;
  // Robot robot_with_ball(-1); // Negative Id
  const Robot *robot_with_ball(&min.robots[0]);
  Player player_with_ball = MIN;

  float time;

#define FOR_ROBOT_IN_TEAM(TEAM, PLAYER)                                        \
  for (const Robot &robot : TEAM) {                                            \
    time = timeToVirtualBall(robot, virt_ball);                                \
    if (time < min_time) {                                                     \
      robot_with_ball = &robot;                                                \
      player_with_ball = PLAYER;                                               \
      min_time = time;                                                         \
    }                                                                          \
  }
  FOR_ROBOT_IN_TEAM(min.robots, MIN)
  FOR_ROBOT_IN_TEAM(max.robots, MAX)
#undef FOR_ROBOT_IN_TEAM

  if (r_rcv != nullptr && timeToVirtualBall(*r_rcv, virt_ball) < min_time) {
    robot_with_ball = r_rcv;
  }

  return std::make_pair(robot_with_ball, player_with_ball);
}

float Board::timeToBall(const Robot &robot) const {
  return timeToVirtualBall(robot, ball);
}

float Board::timeToVirtualBall(const Robot &robot,
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
  float a = (robot.maxV2() - virt_ball.v() * virt_ball.v());
  float c =
      -((robot.pos() - virt_ball.pos()) * (robot.pos() - virt_ball.pos()));
  float b_div_2 = virt_ball.v() * (ball.pos() - robot.pos());
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
      vrt_ball.setV(Vector::unit(robot.pos() - vrt_ball.pos()) *
                    Robot::kickV());

      // Add robot if the same robot will have the ball after kick
      auto with_ball = getRobotWithVirtualBall(vrt_ball, &robot);
      auto robot_with_ball_after_kick = with_ball.first;
      if (&robot == robot_with_ball_after_kick)
        robots.push_back(&robot);
    }
  }

  return robots;
}

float Board::openGoalArea() const {
  float area = 0;

  for (int i = 0; i < NUM_SAMPLE_POINTS; i++)
    if (freeKickLine(i))
      area++;

  return area;
}

std::vector<std::pair<float, float>> Board::getGoalGaps() const {
  auto &ball = getBall();
  auto player_with_ball = getRobotWithBall().second;
  auto gx = enemyGoalPos(player_with_ball)[0];

  // collect shadows

  std::vector<std::pair<float, float>> shadows;
  for (auto _robot : getRobotsMoving()) {
    auto &robot = *_robot;
    auto d = robot.pos() - ball.pos();
    auto k = d * d - Robot::radius() * Robot::radius();

    if (k <= 0)
      continue; // FIXME: should return maximum shadow or no gap
    if (d[0] * gx <= 0)
      continue;

    float tan_alpha = Robot::radius() / std::sqrt(k);
    float tan_theta = d[1] / std::fabs(d[0]);
    float tan_1 = (tan_theta + tan_alpha) / (1 - tan_theta * tan_alpha);
    float tan_2 = (tan_theta - tan_alpha) / (1 + tan_theta * tan_alpha);
    float y_shadow_1 = tan_1 * std::fabs(ball.pos()[0] - gx) + ball.pos()[1];
    float y_shadow_2 = tan_2 * std::fabs(ball.pos()[0] - gx) + ball.pos()[1];

    if ((ball.pos()[0] - robot.pos()[0] + Robot::radius()) *
            (ball.pos()[0] - robot.pos()[0] - Robot::radius()) <
        0) {
      if (ball.pos()[1] > robot.pos()[1])
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

bool Board::freeKickLine(int point_index) const {
  for (auto robot : getRobotsMoving())
    if (kickLineCrossRobot(point_index, *robot))
      return false;

  return true;
}

std::vector<const Robot *> Board::getRobotsMoving() const {
  auto robot_with_ball = getRobotWithBall().first;
  auto robots = getOtherRobots(MIN, *robot_with_ball);
  auto robots2 = getOtherRobots(MAX, *robot_with_ball);
  robots.insert(robots.end(), robots2.begin(), robots2.end());
  return robots;
}

bool Board::kickLineCrossRobot(const int point_index,
                               const Robot &robot) const {
  auto robot_player = getRobotWithBall();
  auto robot_with_ball = robot_player.first;
  auto player_with_ball = robot_player.second;

  Vector point(0,
               goalWidth() * (1 / 2 - point_index * 1.0 / NUM_SAMPLE_POINTS));

  point = enemyGoalPos(player_with_ball) + point;

  if (Vector::lineSegmentCrossCircle(point, robot_with_ball->pos(), robot.pos(),
                                     Robot::radius()))
    return true;

  return false;
}

float Board::evaluate() const {

  // special case for it not to crash when no robots on a team
  if (min.size() == 0 || max.size() == 0)
    return 0.0;

  float goal_area = openGoalArea();
  float receivers_num = canGetPass(MAX).size();

  auto with_ball = getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  auto player_with_ball = with_ball.second;
  auto enemy_goal = enemyGoalPos(player_with_ball);

  float value = WEIGHT_GOAL_OPEN_AREA * goal_area +
                WEIGHT_RECEIVERS_NUM * receivers_num +
                WEIGHT_DISTANCE_TO_GOAL * robot_with_ball->getDist(enemy_goal);

  if (player_with_ball == MAX)
    return value;

  return -value;
}

Board Board::applyTeamAction(const TeamAction &max_a,
                             const TeamAction &min_a) const {
  Board new_board(*this);

#define APPLY_TEAM_ACTION(TA)                                                  \
  /* apply all moves first */                                                  \
  for (auto action : TA) {                                                     \
    if (action->type() == Action::MOVE) {                                      \
      action->apply(MAX, new_board);                                           \
    }                                                                          \
  }                                                                            \
  for (auto action : TA) {                                                     \
    if (action->type() != Action::MOVE) {                                      \
      action->apply(MAX, new_board);                                           \
    }                                                                          \
  }
  APPLY_TEAM_ACTION(max_a)
  APPLY_TEAM_ACTION(min_a)
#undef APPLY_TEAM_ACTION

  return new_board;
}

float Board::teamActionTime(const TeamAction &actions) const {
  float time = FLT_MIN;

  for (auto &action : actions) {
    float a_time = action->getTime();
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
  os << "\t" << v.a_v[0] << "\n\t" << v.a_v[1] << "\n";

  return os;
}
