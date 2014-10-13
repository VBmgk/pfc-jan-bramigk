#include <vector>
#include <armadillo>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

Move::Move(const class Robot &robot) : Action(robot.getPlayer(), robot.getId()) {
  Vector position = robot.getLastPlanedPos();
  // srand(time(NULL));//XXX: ????
  if (rand() % 2 == 1) {
    // Move with uniforme distribution
    nextPosition = robot.getURandPos();
  } else {
    // Move with normal distribution
    nextPosition = robot.getNRandPos();
  }
  // Compute minimum time
  time = robot.getDist(nextPosition) / MAX_SPEED;
}

float Move::getTime() { return time; }

void Move::apply(Board &b) const {
  //b.getRobot(player, robot_id).setPosition(nextPosition);
}

void Pass::apply(Board &b) const {}

void Kick::apply(Board &b) const {}
