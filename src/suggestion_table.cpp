#include <stdio.h>
#include <limits>

#include "suggestion_table.h"
#include "utils.h"
#include "state.h"
#include "vector.h"
#include "action.h"

int add_spot(SuggestionTable &table) {
  if (table.spots_count < MAX_SUGGESTION_SPOTS) {
    table.spots_count++;
    return table.spots_count;
  } else {
    return -1;
  }
}

int del_spot(SuggestionTable & /*table*/, int /*index*/) {
  // TODO: implement, has to move all subsequent spots, PITA
  return -1;
}

Decision gen_decision(bool kick, const SuggestionTable &table,
                      const State *state, DecisionTable &dtable,
                      Player player) {
  // TODO: improve this algorithm, currently it gets a reasonable
  // solutions but
  // not the best
  Decision decision;
  bool filter[MAX_SUGGESTION_SPOTS] = {};

  int rwb = robot_with_ball(*state);

  FOR_TEAM_ROBOT(i, player) if (i != rwb) {
    auto pos = state->robots[i];
    int best_j = -1;
    Vector best_spot = {};
    float best_dist2 = std::numeric_limits<float>::infinity();
    FOR_N_IN(j, table.spots_count, filter) {
      auto spot = table.spots[j];
      float dist2 = norm2(spot - pos);
      if (dist2 < best_dist2) {
        best_j = j;
        best_dist2 = dist2;
        best_spot = spot;
      }
    }
    if (best_j >= 0) {
      filter[best_j] = true;
      decision.action[i] = make_move_action(best_spot);
    } else {
      decision.action[i] = gen_move_action(i, *state, dtable);
    }
  }

  if (player == PLAYER_OF(rwb)) {
    decision.action[rwb] = gen_primary_action(rwb, *state, dtable, kick);
  }

  return decision;
}
