#ifndef DECISION_H
#define DECISION_H

#include "consts.h"
#include "action.h"

struct Decision {
  Action action[N_ROBOTS] = {};
};

#endif
