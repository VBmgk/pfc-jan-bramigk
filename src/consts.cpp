#define _CONST_IMPL(TYPE, NAME, DEFAULT)                                       \
  TYPE NAME = DEFAULT;                                                         \
  TYPE _V_##NAME[] = {DEFAULT, DEFAULT, DEFAULT, DEFAULT};
#include "consts.h"

int param_group;
const int *const PARAM_GROUP = &param_group;
bool PARAM_GROUP_AUTOSELECT = true;
bool PARAM_GROUP_CONQUER = true;
float PARAM_GROUP_THRESHOLD = 1.000;    // 1m
float PARAM_GROUP_CONQUER_TIME = 0.500; // 0.1s

FineOptimize FINE_OPTIMIZE = OPTIMIZE_BEST;

static void change_param_group(int new_param_group) {
  int oldp = param_group;
  int newp = new_param_group;
#define PARAM_SWAP(NAME)                                                       \
  _V_##NAME[oldp] = NAME;                                                      \
  NAME = _V_##NAME[newp];
  PARAM_SWAP(CONSTANT_RATE);
  PARAM_SWAP(KICK_IF_NO_PASS);
  PARAM_SWAP(DECISION_RATE);
  PARAM_SWAP(RAMIFICATION_NUMBER);
  PARAM_SWAP(FULL_CHANGE_PERCENTAGE);
  PARAM_SWAP(MAX_DEPTH);
  PARAM_SWAP(KICK_POS_VARIATION);
  PARAM_SWAP(MIN_GAP_TO_KICK);
  PARAM_SWAP(DESIRED_PASS_DIST);
  PARAM_SWAP(WEIGHT_BALL_POS);
  PARAM_SWAP(WEIGHT_MOVE_DIST_MAX);
  PARAM_SWAP(WEIGHT_MOVE_DIST_TOTAL);
  PARAM_SWAP(WEIGHT_MOVE_CHANGE);
  PARAM_SWAP(WEIGHT_PASS_CHANGE);
  PARAM_SWAP(WEIGHT_KICK_CHANGE);
  PARAM_SWAP(TOTAL_MAX_GAP_RATIO);
  PARAM_SWAP(WEIGHT_CLOSE_TO_BALL);
  PARAM_SWAP(WEIGHT_ENEMY_CLOSE_TO_BALL);
  PARAM_SWAP(WEIGHT_HAS_BALL);
  PARAM_SWAP(WEIGHT_ATTACK);
  PARAM_SWAP(WEIGHT_SEE_ENEMY_GOAL);
  PARAM_SWAP(WEIGHT_BLOCK_GOAL);
  PARAM_SWAP(WEIGHT_BLOCK_ATTACKER);
  PARAM_SWAP(WEIGHT_GOOD_RECEIVERS);
  PARAM_SWAP(WEIGHT_RECEIVERS_NUM);
  PARAM_SWAP(WEIGHT_ENEMY_RECEIVERS_NUM);
  PARAM_SWAP(DIST_GOAL_PENAL);
  PARAM_SWAP(DIST_GOAL_TO_PENAL);
  PARAM_SWAP(MOVE_RADIUS_0);
  PARAM_SWAP(MOVE_RADIUS_1);
  PARAM_SWAP(MOVE_RADIUS_2);
#undef PARAM_SWAP
  param_group = new_param_group;
}

void set_param_group(int new_param_group) {
  // only change if needed
  if (param_group != new_param_group)
    change_param_group(new_param_group);
}
