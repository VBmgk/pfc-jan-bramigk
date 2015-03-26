#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "valued_decision.h"
#include "decision_table.h"
#include "state.h"
#include "player.h"

struct Optimization {
  DecisionTable table;
  int robot_to_move = 0;
};

ValuedDecision decide(Optimization &opt, State state, Player player);

#endif
