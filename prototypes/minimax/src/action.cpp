#include <vector>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"
#include "discrete.pb.h"

void Action::discreteAction(roboime::Action *a) {
  a->set_robot_id(robot_id);
  switch (type) {

  case MOVE: {
    a->set_type(roboime::Action::MOVE);
    auto *move = a->mutable_move();
    move->set_x(move_point[0]);
    move->set_y(move_point[0]);
  } break;

  case PASS: {
    a->set_type(roboime::Action::PASS);
    auto *pass = a->mutable_pass();
    pass->set_robot_id(pass_id);
  } break;

  case KICK: {
    a->set_type(roboime::Action::KICK);
    auto *kick = a->mutable_kick();
    kick->set_x(kick_point[0]);
    kick->set_y(kick_point[0]);
  } break;

  default: {}
  }
}

Action Action::genMove(const Robot & r, Player p, const Board & b, const MoveTable & mtable) {
  Vector pos;
  bool ok = true;
  int rand_r;

again:
  rand_r = 1 + (rand() % 3);
  if (rand_r == 3)
    rand_r = 6;
  pos = r.getLocalRadRandPos(rand_r, Board::fieldWidth(), Board::fieldHeight());

  for (auto other_rob : b.getMax().getRobots()) {
    if (p != MAX && other_rob.getId() != r.getId()) {
      if (other_rob.getDist(pos) <= r.radius() + other_rob.radius())
        goto again;
    }
  }
  for (auto other_rob : b.getMin().getRobots()) {
    if (p != MIN && other_rob.getId() != r.getId()) {
      if (other_rob.getDist(pos) <= r.radius() + other_rob.radius())
        goto again;
    }
  }
  for (auto mov : mtable) {
    if (mov.first != r.getId()) {
      if (mov.second.type == MOVE && (pos - mov.second.move_point).norm() <= r.radius() + b.getBall().radius())
        goto again;
    }
  }

  // Compute minimum time
  //time = robot.getDist(next_position) / MAX_SPEED;

  return Action::makeMove(r.getId(), pos);
}

Action Action::genKick(const Robot & r, Player p, const Board & b) {
  float kx, ky, len = 0;

  kx = b.enemyGoalPos(p)[0];

  for (auto gap : b.getGoalGaps(p == MAX ? MIN : MAX, b.getBall())) {
    auto this_len = std::fabs(gap.first - gap.second);
    if (this_len > len) {
      len = this_len;
      ky = (gap.first + gap.second) / 2;
    }
  }

  Vector kpos(kx, ky);

  return Action::makeKick(r.getId(), kpos);
}

void applyAction(const Action & a, Player p, Board & b) {
  switch (a.type) {
    case MOVE: {
                 for (auto &robot : b.getTeam(p).getRobots()) {
                   if (robot.getId() == a.robot_id)
                     robot.setPos(a.move_point);
                 }
               } break;
    case KICK: {
                 for (auto &robot : b.getTeam(p).getRobots()) {
                   if (robot.getId() == a.robot_id)
                     robot.setPos(b.getBall().pos());
                 }
                 // XXX: better ball position
                 b.getBall().setPos(a.kick_point);
               } break;
    case PASS: {
                 Robot *rcv = nullptr;
                 auto &ball = b.getBall();
                 Vector ball_pos = ball.pos();
                 for (auto &robot : b.getTeam(p).getRobots()) {
                   if (robot.getId() == a.robot_id) {
                     robot.setPos(ball_pos);
                   }
                   if (robot.getId() == a.pass_id) {
                     rcv = &robot;
                   }
                 }
                 if (rcv != nullptr) {
                   ball.setPos(rcv->pos() -
                       (rcv->pos() - ball.pos()).unit() *
                       (Robot::radius() + Ball::radius()));
                 } else {
                   std::cerr << "XXX pass_id " << a.pass_id << " not found... (robot_id = " << a.robot_id << ")" << std::endl;
                 }
               } break;
    default: {}
  }
}
