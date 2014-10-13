#include <vector>
#include <armadillo>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

Move::Move(const class Robot &robot) : Action(robot.getId()) {
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

void Move::apply(Player, Board &b) const {}

void Pass::apply(Player, Board &b) const {}

void Kick::apply(Player, Board &b) const {}
