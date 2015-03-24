#include "utils.h"
#include "state.h"
#include "vector.h"

State uniform_rand_state() {
  State s;

  FOR_EVERY_ROBOT(i)
  s.robots[i] = uniform_rand_vector(FIELD_WIDTH, FIELD_HEIGHT);

  s.ball = uniform_rand_vector(FIELD_WIDTH, FIELD_HEIGHT);
  return s;
}
