#ifndef DECISION_TABLE_H
#define DECISION_TABLE_H

#include "consts.h"
#include "action.h"
#include "array.h"

struct DecisionTable {
  int kick_robot = -1, pass_robot = -1;
  Action kick, pass;
  TeamArray<Action> move = {};
};

#endif
