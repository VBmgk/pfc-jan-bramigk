#include <vector>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"
#include "discrete.pb.h"

Move::Move(const class Robot &robot) : Action(robot.getId()) {
  Vector position = robot.getLastPlanedPos();

  // next_position = robot.getLocalRandPos(Board::fieldWidth(), Board::fieldHeight());
  int rand_r = 1 + (rand() % 3);
  if (rand_r == 3) rand_r = 6;
  next_position = robot.getLocalRadRandPos(rand_r, Board::fieldWidth(), Board::fieldHeight());

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
  ball.setPos(rcv->pos() - (rcv->pos() - ball.pos()).unit() * (Robot::radius() + Ball::radius()));
}

void Kick::apply(Player p, Board &b) const {
  for (auto &robot : b.getTeam(p).getRobots()) {
    if (robot.getId() == getId()) {
      robot.setPos(b.getBall().pos());
    }
  }
  // TODO: position ball
}

roboime::Command convert(TeamAction team_action) {
  roboime::Command cmd;

  for (auto &action : team_action)
    action->discreteAction(cmd.add_action());

  return cmd;
}
