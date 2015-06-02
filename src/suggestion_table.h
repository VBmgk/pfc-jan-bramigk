#ifndef SUGGESTION_TABLE_H
#define SUGGESTION_TABLE_H

#include "consts.h"
#include "vector.h"
#include "decision.h"
#include "player.h"

struct SuggestionTable {
  char name[256] = "";
  int spots_count = 0;
  Vector spots[MAX_SUGGESTION_SPOTS] = {};
  int usage_count = 0;
};

// allocate new spot, return the index or -1 on failure
int add_spot(SuggestionTable &table);

// delete given spot, return new size or -1 on failure
int del_spot(SuggestionTable &table, int index);

Decision gen_decision(bool kick, const SuggestionTable &table,
                      const struct State *state, struct DecisionTable &dtable,
                      Player player);

#endif
