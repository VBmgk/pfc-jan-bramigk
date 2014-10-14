#include <vector>
#include <armadillo>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

using namespace std;

bool Board::isGameOver() const {
  if (openGoalArea() > MIN_AREA_TO_MARK)
    return true;

  return false;
}

Player Board::currentPlayer() const { return player; }

TeamAction Board::genActions(bool kickAction) const {
  TeamAction actions;

  if (playerWithBall() == player) {
    Robot robotWithBall = getRobotWithBall();

    if (kickAction) // Kick
      actions.push_back(*new Kick(robotWithBall));
    else { // Pass
      for (auto robot : canGetPass()) {
        Pass *pass = new Pass(robotWithBall, robot);
        actions.push_back(*pass);
      }
    }
  }

  // Move
  for (auto robot : getRobots2Move()) {
    actions.push_back(*new Move(robot));
  }

  return actions;
}

TeamAction Board::genKickTeamAction() const { return genActions(true); }

TeamAction Board::genPassTeamAction() const { return genActions(false); }

const Robot &Board::getRobotWithBall() const {
  return getRobotWithVirtualBall(ball);
}

const Robot &Board::getRobotWithVirtualBall(const Ball &virt_ball) const {
  // TODO: don't concatenate vectors
  vector<Robot> robots;

  // preallocate memory
  // robots.reserve(min.getRobots().size() + max.getRobots().size());
  // robots.insert(robots.end(), min.getRobots().begin(),
  // min.getRobots().end());
  // robots.insert(robots.end(), max.getRobots().begin(),
  // max.getRobots().end());

  float min_time = FLT_MAX;
  // Robot robotWithBall(-1); // Negative Id
  const Robot *robotWithBall(&min.robots[0]);

  float time;
#define FOR_ROBOT_IN_TEAM(TEAM)                                                \
  for (const Robot &robot : TEAM) {                                            \
    time = timeToVirtualBall(robot, virt_ball);                                \
    if (time < min_time) {                                                     \
      robotWithBall = &robot;                                                  \
      min_time = time;                                                         \
    }                                                                          \
  }
  FOR_ROBOT_IN_TEAM(min.robots)
  FOR_ROBOT_IN_TEAM(max.robots)

  return *robotWithBall;
}

const Robot &Board::getRobotWithVirtualBall(const Ball &virt_ball,
                                            const Robot &r_rcv) const {
  float min_time = FLT_MAX;
  // Robot robotWithBall(-1); // Negative Id
  const Robot *robotWithBall(&min.robots[0]);

  float time;
// vector<Robot> robots = (r_rcv.getPlayer() == Player::MIN ? max :
// min).getRobots();

#define FOR_ROBOT_IN_TEAM(TEAM)                                                \
  for (const Robot &robot : TEAM) {                                            \
    time = timeToVirtualBall(robot, virt_ball);                                \
    if (time < min_time) {                                                     \
      robotWithBall = &robot;                                                  \
      min_time = time;                                                         \
    }                                                                          \
  }
  FOR_ROBOT_IN_TEAM(min.robots)
  FOR_ROBOT_IN_TEAM(max.robots)

  if (timeToVirtualBall(r_rcv, virt_ball) < min_time) {
    robotWithBall = &r_rcv;
  }

  return *robotWithBall;
}

float Board::timeToBall(const Robot &robot) const {
  return timeToVirtualBall(robot, this->ball);
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

Player Board::playerWithBall() const { return getRobotWithBall().getPlayer(); }

Player Board::playerWithVirtualBall(const Ball &virt_ball,
                                    const Robot robot) const {
  return getRobotWithVirtualBall(virt_ball, robot).getPlayer();
}

Board Board::virtualStep(float time) const {
  Board n_board;
  n_board.ball = Ball(ball.pos() + ball.v() * time, ball.v());

  for (auto &robot : min.getRobots())
    n_board.min.addRobot(
        Robot(robot.getId(), robot.pos() + robot.v() * time, robot.v()));

  Team n_max(MAX);
  for (auto &robot : max.getRobots())
    n_board.max.addRobot(
        Robot(robot.getId(), robot.pos() + robot.v() * time, robot.v()));

  return n_board;
}

vector<Robot> Board::canGetPass() const {
  vector<Robot> robots;
  Robot robot_with_ball = getRobotWithBall();

  if (robot_with_ball.getPlayer() == player) {
    float step_time = timeToBall(getRobotWithBall());
    Board vrt_board = virtualStep(step_time);
    Ball vrt_ball = vrt_board.ball;

    for (auto &robot : vrt_board.getTeam(player).getRobots()) {
      if (robot.getId() != robot_with_ball.getId()) {
        vrt_ball.setV(Vector::unit(robot.pos() - vrt_ball.pos()) *
                      Robot::kickV());

        // Add robot if the atual player still
        // have the ball after kick
        if (vrt_board.playerWithVirtualBall(vrt_ball, robot) == player)
          robots.push_back(robot);
      }
    }
  }

  return robots;
}

float Board::openGoalArea() const {
  float area = 0;

  for(int i=0; i<NUM_SAMPLE_POINTS; i++)
    if(freeKickLine(i)) area++;

  return area;
}

bool Board::freeKickLine(int point_index) const {
  for(auto& robot: getRobotsMoving())
    if(kickLineCrossRobot(point_index, robot)) return false;

  return true;
}

vector<Robot> Board::getRobotsMoving() const {
  vector<Robot> robotsMoving;
  Robot robot_with_ball = getRobotWithBall();

  if(playerWithBall() == Player::MAX){
    robotsMoving = min.getRobots();

    for (auto &robot : max.getRobots())
      if (robot.getId() != robot_with_ball.getId())
        robotsMoving.push_back(robot);
  } else {
    robotsMoving = max.getRobots();

    for (auto &robot : min.getRobots())
      if (robot.getId() != robot_with_ball.getId())
        robotsMoving.push_back(robot);
  }

  return robotsMoving;
}

bool Board::kickLineCrossRobot(const int point_index, const Robot &robot) const {
  Vector point(goalX(), goalY() + point_index * goalWidth() * 1.0 / NUM_SAMPLE_POINTS);

  if(Vector::lineSegmentCrossCircle(point, getRobotWithBall().pos(), robot.pos(), Robot::radius()))
    return true;

  return false;
}

float Board::evaluate() const {
  float goal_area = openGoalArea();
  float receivers_num = canGetPass().size();

  Robot robot_with_ball = getRobotWithBall();

  float value = WEIGHT_GOAL_OPEN_AREA   * goal_area +
                WEIGHT_RECEIVERS_NUM    * receivers_num +
                WEIGHT_DISTANCE_TO_GOAL * robot_with_ball.distanceToEnemyGoal();

  if(robot_with_ball.getPlayer() == Player::MAX) return value;

  return -value;
}

Board Board::applyTeamAction(const TeamAction &actions) const {
  Board new_board(*this);
  for (auto &action : actions) {
    action.apply(new_board);
  }

  return new_board;
}

float Board::teamActionsTime(const TeamAction &actions) const {
  float time = FLT_MIN;

  for(auto &action: actions)
    if(action.getTime() > time) time = action.getTime();

  return time;
}

vector<Robot> Board::getRobots2Move() const {
  const Robot &robot_with_ball = getRobotWithBall();

  // preallocate memory
  if (robot_with_ball.getPlayer() == player) {
    vector<Robot> robots;

    for (auto &robot : getTeam(player).getRobots()) {
      if (robot.getId() != robot_with_ball.getId())
        robots.push_back(robot);
    }

    return robots;
  } else {
    return getTeam(player).getRobots();
  }
}

// print operator for Vector
std::ostream &operator<<(std::ostream &os, const Vector &v) {
  os << "\t" << v.a_v[0] << "\n\t" << v.a_v[1] << "\n";

  return os;
}
