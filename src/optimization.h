#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "valued_decision.h"
#include "decision_table.h"
#include "state.h"
#include "player.h"

struct Optimization {
  DecisionTable table;
};

ValuedDecision decide(Optimization &opt, State state, Player player);

#endif
