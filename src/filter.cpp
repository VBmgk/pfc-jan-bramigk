#include "filter.h"

void filter_out(TeamFilter &team_filter, int i) {
  if (!team_filter[i]) {
    team_filter.count--;
    team_filter[i] = true;
  }
}
