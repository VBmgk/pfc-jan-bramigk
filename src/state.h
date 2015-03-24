#ifndef STATE_H
#define STATE_H

#include "consts.h"
#include "vector.h"

struct State {
  Vector ball;
  Vector ball_v;
  Vector robots[2 * N_ROBOTS];
  Vector robots_v[2 * N_ROBOTS];
};

State uniform_rand_state();

#endif
