#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "valued_decision.h"
#include "decision_table.h"
#include "state.h"
#include "player.h"
#include "consts.h"
#include "gradient.h"

struct Optimization {
  DecisionTable table;
  int robot_to_move = 0;
  bool table_initialized = false;
};

ValuedDecision decide(Optimization &opt, State state, Player player,
                      struct Suggestions *suggestions, int *ramification_count);

float evaluate_with_decision(Player player, const State &state,
                             const Decision &decision,
                             const DecisionTable &table,
                             float *values = nullptr);

Gradient evaluate_with_decision_gradient(Player player, const State &state,
                                         const Decision &decision,
                                         const DecisionTable &table);

ValuedDecision optimize_decision(Player player, const State &state,
                                 const ValuedDecision &valued_decision,
                                 const DecisionTable &table);

#endif
