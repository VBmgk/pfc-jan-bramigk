#ifndef ADAPTATIVE_CONTROL
#define ADAPTATIVE_CONTROL
#define NUM_BOARD_EVAL 5
#include "minimax.h"

namespace AdaptativeControl{
double f_dist (double);
double f_gap (double);

struct AdaptativeControlEval {
  // TODO: add time
  int curr = -1;
  double evals[NUM_BOARD_EVAL];
         //time[NUM_BOARD_EVAL + 1],

  // functions
  double eval(const Board& board); 
  void add_eval(double); 
  //double init_time();
  //double get_time(int);
  //void add_time(double);
  /* (Sum_k g_k) / N */
  double eval_n (void);
  
  /* (Sum_k g_k * time_k) / (Sum_k time_k) */
  //double eval_with_time (void);
};
}
#endif
