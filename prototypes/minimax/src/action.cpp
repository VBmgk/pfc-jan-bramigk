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

void Move::apply(Player, Board &b) const {
  // TODO!!!!!!! Just do it!!!!
}

void Pass::apply(Player, Board &b) const {
  // TODO!!!!!!! Just do it!!!!
}

void Kick::apply(Player, Board &b) const {
  // TODO!!!!!!! Just do it!!!!
}

roboime::Command convert(TeamAction team_action){
  roboime::Command cmd;

  for(auto& action: team_action)
    action->discreteAction(cmd.add_action());

  return cmd;
}
