#ifndef FILTER_H
#define FILTER_H

#include "array.h"

// this is a exclusion filter, true means out
// (rationale behind it is that default initialization goes to no
// filter)
struct TeamFilter : TeamArray<bool> {
  int count = N_ROBOTS;
};

void filter_out(TeamFilter &team_filter, int i);

#endif
