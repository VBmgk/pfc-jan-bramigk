#ifndef ADAPTATIVE_CONTROL
#define ADAPTATIVE_CONTROL
#define NUM_BOARD_EVALS 5
#define NUM_BOARD_EVALS_N 10000
#define DELTA_GRAD 0.1
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
  int curr = -1, curr_n = -1, var_index = 0;
  float evals[NUM_BOARD_EVALS] = {},
        evals_n[NUM_BOARD_EVALS_N] = {},
        sum = 0;
         //time[NUM_BOARD_EVAL + 1],
  bool overflow = false;
  // values to avoid going to a bad state
  float PREV_VALS[13] = {}, *CONST_ADDRS[13] = {};

  // functions
  float eval(const Board& board); 
  float get_eval_n (void) { return evals_n[curr_n]; }
  int scale(void) { return NUM_BOARD_EVALS_N/(curr + NUM_BOARD_EVALS_N); }

  float part_deriv(int var_index) {
    save_vars();
    *CONST_ADDRS[var_index] += DELTA_GRAD;
    return 0;
  }

  void save_vars(void) {
    for(int i=0; i<13 ; i++) PREV_VALS[i] = *CONST_ADDRS[i];
  }
  
  void save_addrs(void) {
    CONST_ADDRS[0]  = &Board::KICK_POS_VARIATION;
    CONST_ADDRS[1]  = &Board::MIN_GAP_TO_KICK;
    CONST_ADDRS[2]  = &Board::WEIGHT_MOVE_DIST_TOTAL;
    CONST_ADDRS[3]  = &Board::WEIGHT_MOVE_DIST_MAX;
    CONST_ADDRS[4]  = &Board::WEIGHT_MOVE_CHANGE;
    CONST_ADDRS[5]  = &Board::TOTAL_MAX_GAP_RATIO;
    CONST_ADDRS[6]  = &Board::WEIGHT_ATTACK;
    CONST_ADDRS[7]  = &Board::WEIGHT_SEE_ENEMY_GOAL;
    CONST_ADDRS[8]  = &Board::WEIGHT_BLOCK_GOAL;
    CONST_ADDRS[9]  = &Board::WEIGHT_BLOCK_ATTACKER;
    CONST_ADDRS[10] = &Board::WEIGHT_RECEIVERS_NUM;
    CONST_ADDRS[11] = &Board::DIST_GOAL_PENAL;
    CONST_ADDRS[12] = &Board::DIST_GOAL_TO_PENAL;
  }

  void add_eval(float); 
  void add_eval_n(float); 
  //double init_time();
  //double get_time(int);
  //void add_time(double);
  
  /* (Sum_k g_k * time_k) / (Sum_k time_k) */
  //double eval_with_time (void);
};
 
}
#endif
