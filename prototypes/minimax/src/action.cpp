#include <vector>
#include <armadillo>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"
#include "discrete.pb.h"

Move::Move(const class Robot &robot) : Action(robot.getId()) {
  Vector position = robot.getLastPlanedPos();
  // srand(time(NULL));//XXX: ????
  next_position = robot.getURandPos(Board::fieldWidth(), Board::fieldHeight());
  //if (rand() % 2 == 1) {
  //  // Move with uniforme distribution
  //  next_position = robot.getURandPos();
  //} else {
  //  // Move with normal distribution
  //  next_position = robot.getNRandPos();
  //}
  // Compute minimum time
  time = robot.getDist(next_position) / MAX_SPEED;
}

float Move::getTime() const { return time; }

void Move::apply(Player p, Board &b) const {
  for (auto &robot : b.getTeam(p).getRobots()) {
    if (robot.getId() == getId()) {
      robot.setPos(next_position);
    }
  }
}

void Pass::apply(Player p, Board &b) const {
  Robot *r, *rcv;
  auto &ball = b.getBall();
  Vector ball_pos = ball.pos();
  for (auto &robot : b.getTeam(p).getRobots()) {
    if (robot.getId() == getId()) {
      robot.setPos(ball_pos);
      r = &robot;
    }
    if (robot.getId() == getRcvId()) {
      rcv = &robot;
    }
  }
  ball.setPos(rcv->pos() - Vector::unit(rcv->pos() - ball.pos()) * (Robot::radius() + Ball::radius()));
}

void Kick::apply(Player p, Board &b) const {
  for (auto &robot : b.getTeam(p).getRobots()) {
    if (robot.getId() == getId()) {
      robot.setPos(b.getBall().pos());
    }
  }
  // TODO: position ball
}

roboime::Command convert(TeamAction team_action){
  roboime::Command cmd;

  for(auto& action: team_action)
    action->discreteAction(cmd.add_action());

  return cmd;
}
