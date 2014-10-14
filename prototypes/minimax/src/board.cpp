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
      // FIXME: this is will lead to memory leak
      actions.push_back(*new Kick(*robot_with_ball));
      // Pass
    } else {
      bool any_pass = false;
      for (auto robot : canGetPass(player)) {
        // FIXME: this is will lead to memory leak
        Pass *pass = new Pass(*robot_with_ball, *robot);
        actions.push_back(*pass);
        any_pass = true;
        // FIXME: only a single action per robot!
        //        there has to be many TeamAction's for this
        break;
      }
      // in the rare case there isn't any possible pass
      // for the robot with ball, we'll make it move
      if (!any_pass) {
        // FIXME: this is will lead to memory leak
        actions.push_back(*new Move(*robot_with_ball));
      }
    }
  }

  // push a Move action for every other robot
  for (auto robot : getOtherRobots(player, *robot_with_ball)) {
    // FIXME: this is will lead to memory leak
    actions.push_back(*new Move(*robot));
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
  Vector point(goalX(),
               goalY() + point_index * goalWidth() * 1.0 / NUM_SAMPLE_POINTS);

  auto robot_with_ball = getRobotWithBall().first;
  if (Vector::lineSegmentCrossCircle(point, robot_with_ball->pos(), robot.pos(),
                                     Robot::radius()))
    return true;

  return false;
}

float Board::evaluate() const {
  float goal_area = openGoalArea();
  float receivers_num = canGetPass(MAX).size();

  auto with_ball = getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  auto player_with_ball = with_ball.second;

  float value = WEIGHT_GOAL_OPEN_AREA * goal_area +
                WEIGHT_RECEIVERS_NUM * receivers_num +
                WEIGHT_DISTANCE_TO_GOAL * robot_with_ball->distanceToEnemyGoal();

  if (player_with_ball == MAX)
    return value;

  return -value;
}

Board Board::applyTeamAction(const TeamAction &max_a,
                             const TeamAction &min_a) const {
  Board new_board(*this);

  for (auto &action : max_a)
    action.apply(MAX, new_board);

  for (auto &action : min_a)
    action.apply(MIN, new_board);

  return new_board;
}

float Board::teamActionsTime(const TeamAction &actions) const {
  float time = FLT_MIN;

  for (auto &action : actions)
    if (action.getTime() > time)
      time = action.getTime();

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
