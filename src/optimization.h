#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "valued_decision.h"
#include "decision_table.h"
#include "state.h"
#include "player.h"
#include "consts.h"

struct Optimization {
  DecisionTable table;
  int robot_to_move = 0;
  bool table_initialized = false;
};

ValuedDecision decide(Optimization &opt, State state, Player player, struct Suggestions *suggestions,
                      int *ramification_count);

#endif
