#ifndef VALUED_DECISON_H
#define VALUED_DECISON_H

#include "decision.h"
#include "consts.h"

struct ValuedDecision {
  // both must be filled by the value generation
  float value;
  float values[W_SIZE] = {};

  Decision decision;
};

#endif
