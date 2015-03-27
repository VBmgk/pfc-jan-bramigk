#include "adaptative_control.h"
#include "minimax.h"

float AdaptativeControl::f_dist (float d)   { return 1.0 / (1 + d); }
float AdaptativeControl::f_gap  (float gap) { return gap;         }

float AdaptativeControl::AdaptativeControlEval::
 eval (const Board& board) {
  /*
   * nd_t: sum of all robots of our team distance to ball
   * ng  : total gap of enemy goal
   * id_t: sum of all robots of enemy team distance to ball
   * ig  : total gap of our goal
   * curr: index of last inserted element in the circular
   *       buffer
   */
  float nd_t = 0, id_t = 0,
         ng   = board.totalGoalGap(MIN, board.getBall()),
         ig   = board.totalGoalGap(MAX, board.getBall());

  for (auto& robot: board.getTeam(MAX).getRobots()) {
    nd_t += (board.getBall().pos() - robot.pos()).norm();
  }

  for (auto& robot: board.getTeam(MIN).getRobots()) {
    id_t += (board.getBall().pos() - robot.pos()).norm();
  }

  float g_eval = f_dist (nd_t) * f_gap (ng) -
                  f_dist (id_t) * f_gap (ig); 

  add_eval( g_eval );
 
  return g_eval;
}

void AdaptativeControl::AdaptativeControlEval::
 add_eval(float new_eval) {
  curr++; curr %= NUM_BOARD_EVALS;
  
  evals[curr] = new_eval;

  sum += new_eval;
  // Eval considering time
  // sum_with_time += new_eval * (curr_time - last_time);
  // total_time += curr_time - last_time; last_time = curr_time;
  // adding evaluation of to the array of evaluations 
  if (curr == (NUM_BOARD_EVALS-1) && overflow == false) {
    // Mean of evals (Sum_k g_k) / N
    evals_n[++curr_n] = sum / NUM_BOARD_EVALS; sum = 0;
    // (Sum_k g_k * time_k) / (Sum_k time_k)
    //evals_with_time[++curr_t] = sum_with_time / total_time;
    //sum_with_time = 0;

    // checking overflow
    if (curr_n == NUM_BOARD_EVALS_N-1) { overflow = true; }
  }
}
