#ifndef ADAPTATIVE_CONTROL
#define ADAPTATIVE_CONTROL
#define NUM_BOARD_EVALS 5
#define VAR_NUM 13
#define NUM_BOARD_EVALS_N 10000
#define DELTA 0.1
#include "minimax.h"

namespace AdaptativeControl{
float f_dist (float);
float f_gap (float);

/*
 * curr:      Current last element inserted
 *            on the circular buffer evals
 * var_index: Current index of the variable
 *            been changed to measure derivative
 * evals:     Buffer of adaptative control
 *            evaluation function values
 * add_eval:  Function to insert and move
 *            curr variable properly
 */

struct AdaptativeControlEval {
  // TODO: add time
  bool overflow = false, wait_change = false;
  int curr = -1, curr_n = -1, var_index = 0, change_curr_n = 0;
  float evals[NUM_BOARD_EVALS] = {}, evals_n[NUM_BOARD_EVALS_N] = {},
        // values to avoid going to a bad state
        PREV_VALS[VAR_NUM] = {}, *CONST_ADDRS[VAR_NUM] = {}, sum = 0;

  // functions
  // Saving address of variables to enable further modification
  AdaptativeControlEval(void) { save_addrs(); }

  float eval(const Board& board); 
  float get_eval_n (void) { return evals_n[curr_n]; }
  void gen_var_index(void);
  void change_var(float delta = DELTA);
  void go_back_var(float delta = DELTA);
  void save_addrs(void);
  void add_eval(float); 
  void add_eval_n(float); 
  void load(void);
  void save(void);
};
}
#endif
