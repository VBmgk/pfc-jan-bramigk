#include <cfloat>
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
 gen_var_index(void) {
  if (wait_change == false) {
    // gen random variable index
    var_index = rand() % VAR_NUM;
  }
}

void AdaptativeControl::AdaptativeControlEval::
 change_var(float delta) {
  // save current variables and modify the desired variable
  if (wait_change == false) {
    PREV_VALS[var_index] = *CONST_ADDRS[var_index];
    *CONST_ADDRS[var_index] += delta;
    change_curr_n = curr_n; wait_change = true;
  }
}

void AdaptativeControl::AdaptativeControlEval::
 go_back_var(float delta) {
  // go back if some new evaluation is worst than before
  if (wait_change && change_curr_n != curr_n){
    if (evals_n[curr_n] < evals_n[change_curr_n]) {
      // go back if it is worst
      *CONST_ADDRS[var_index] -= delta;
    }

    // enable new variable change
    wait_change = false;
  }
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

void AdaptativeControl::AdaptativeControlEval::
 save_addrs(void) {
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

void load(void) {
  for(int i = 0; i<VAR_NUM ; i++) {
    *CONST_ADDRS[i] = PREV_VALS[i];
  }
}

void save(void) {
  for(int i = 0; i<VAR_NUM ; i++) {
    PREV_VALS[i] = *CONST_ADDRS[i];
  }
}
