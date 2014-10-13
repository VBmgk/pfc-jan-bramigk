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
    if(kickAction) // Kick
      actions.push_back(*new Kick(robotWithBall));
    else { // Pass
      Robot robotWithBall = getRobotWithBall();

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

TeamAction Board::genKickTeamAction() const {
  return getActions(true);
}

TeamAction Board::genPassTeamAction() const {
  return getActions(false);
}

Robot Board::getRobotWithBall() const { return getRobotWithVirtualBall(ball); }

Robot Board::getRobotWithVirtualBall(const Ball &virt_ball) const {
  // TODO: don't concatenate vectors
  vector<Robot> robots;

  // preallocate memory
  robots.reserve(min.getRobots().size() + max.getRobots().size());
  robots.insert(robots.end(), min.getRobots().begin(), min.getRobots().end());
  robots.insert(robots.end(), max.getRobots().begin(), max.getRobots().end());

  float min_time = FLT_MAX;
  Robot robotWithBall(-1); // Negative Id

  float time;
  for (auto &robot : robots) {
    time = timeToVirtualBall(robot, virt_ball);

    if (time < min_time) {
      robotWithBall = robot;
      min_time = time;
    }
  }

  return robotWithBall;
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

vector<Robot> Board::canGetPass() const {
  // TODO
  // create ball with after kick
  // float getTimeToBall(const Robot& robot, const Ball ball){
}

float Board::openGoalArea() const {
  // TODO
  return 0;
}

float Board::evaluate() const {
  // TODO
  return 0;
}

Board Board::applyTeamActions(const TeamAction &actions) const {
  Board new_board(*this);
  for (auto &action : actions) {
    action.apply(new_board);
  }

  return new_board;
}

float Board::teamActionsTime(const TeamAction &actions) const {
  // TODO: get maximum time
}

vector<class Robot> Board::getRobots2Move() const {
  vector<Robot> robots;

  // preallocate memory
  robots.reserve(min.getRobots().size() + max.getRobots().size());
  robots.insert(robots.end(), min.getRobots().begin(), min.getRobots().end());
  robots.insert(robots.end(), max.getRobots().begin(), max.getRobots().end());

  return robots;
}

// print operator for Vector
std::ostream &operator<<(std::ostream &os, const Vector &v) {
  os << "\t" << v.a_v[0] << "\n\t" << v.a_v[1] << "\n";

  return os;
}
