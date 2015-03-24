#include "adaptative_control.h"
#include "minimax.h"

double AdaptativeControl::f_dist (double d)   { return 1.0 / (1 + d); }
double AdaptativeControl::f_gap  (double gap) { return gap;         }

double AdaptativeControl::AdaptativeControlEval::
 eval (const Board& board) {
  /*
   * nd_t: sum of all robots of our team distance to ball
   * ng  : total gap of enemy goal
   * id_t: sum of all robots of enemy team distance to ball
   * ig  : total gap of our goal
   * curr: index of last inserted element in the circular
   *       buffer
   */
  double nd_t = 0, id_t = 0,
         ng   = board.totalGoalGap(MIN, board.getBall()),
         ig   = board.totalGoalGap(MAX, board.getBall());

  for (auto& robot: board.getTeam(MAX).getRobots()) {
    nd_t += (board.getBall().pos() - robot.pos()).norm();
  }

  for (auto& robot: board.getTeam(MIN).getRobots()) {
    id_t += (board.getBall().pos() - robot.pos()).norm();
  }

  // TODO: add curr time
  double g_eval = f_dist (nd_t) * f_gap (ng) -
                  f_dist (id_t) * f_gap (ig); 

  add_eval( g_eval );
 
  return g_eval;
}

void AdaptativeControl::AdaptativeControlEval::
 add_eval(double new_eval) {
  if (curr < 0) {
    // inicialization of eval
    curr = 0;
    for(int i=0; i<NUM_BOARD_EVAL ; i++) { evals[i] = 0; }
  } else { curr++; curr %= NUM_BOARD_EVAL; }
  
  evals[curr] = new_eval;
}

/* (Sum_k g_k) / N */
double AdaptativeControl::AdaptativeControlEval::
 eval_n (void) {
  double sum = 0;
  for(int i=0; i<NUM_BOARD_EVAL ; i++) { sum += evals[i]; }
  return sum / NUM_BOARD_EVAL;
}

/* (Sum_k g_k * time_k) / (Sum_k time_k) */
/*
double eval_with_time (void) {
  double sum        = 0,
         total_time = -board_eval.init_time();

  for(int i=0 ; i<NUM_BOARD_EVAL ; i++) {
    sum  += board_eval.get_eval(i) * board_eval.get_time(i); 
    total_time += board_eval.get_time(i);
  }

  return sum / total_time;
}
*/
