#ifndef MINIMAX_APP_H
#define MINIMAX_APP_H

#include "minimax.h"
#include "adaptative_control.h"

struct App {
  Board board;
  std::mutex board_mutex;
  TeamAction command, enemy_command;
  Board command_board;
  std::mutex command_mutex;
  struct AdaptativeControl::AdaptativeControlEval
    // A - attack
    // D - defence
    // EF - ball on enemy field
    // OF - ball on our field
    adptv_cntrl_eval_a_of, adptv_cntrl_eval_a_ef, 
    adptv_cntrl_eval_d_of, adptv_cntrl_eval_d_ef,
    * adptv_cntrl_eval = NULL;

  struct {
    int uptime = 0;
    int minimax_count = 0;
    int pps = 0;
    int mps = 0;
    float val = 0.0;
    bool has_val = false;
    float minimax_val = 0.0;
  } display;
  std::mutex display_mutex;
  bool play_minimax = false;
  bool play_minimax_once = false;
  bool eval_board_once = false;
  bool use_experimental = false;

  void toggle_experimental() { use_experimental = !use_experimental; }

  static void run(std::function<void(App &)>);

  void random() {
    std::lock_guard<std::mutex> _(board_mutex);
    board = Board::randomBoard();
    display.has_val = false;
  }

  void minimax_once() { play_minimax_once = true; }
  void minimax_toggle() { play_minimax = !play_minimax; }
  void eval_once() { eval_board_once = true; }

  void apply() {
    std::lock_guard<std::mutex> _(board_mutex);
    board = board.applyTeamAction(command, enemy_command);
  }

  Player select_team = MIN;
  void switch_select_team() {
    if (select_team == MIN)
      select_team = MAX;
    else
      select_team = MIN;
    select_pos--;
    next_robot();
  }
  Robot *selected_robot = nullptr;
  int select_pos = 0;
  void next_robot() {
    auto &robots = board.getTeam(select_team).getRobots();
    if (++select_pos >= robots.size())
      select_pos = 0;
    selected_robot = &robots[select_pos];
  }

  // TODO: maybe persist these
  static constexpr int save_slots = 10;
  Board saved_boards[save_slots];
  int save_slot = 0;
  void save_board() { saved_boards[save_slot] = board; }
  void load_saved_slot(int i) {
    std::lock_guard<std::mutex> _(board_mutex);
    save_slot = i % save_slots;
    board = saved_boards[save_slot];
  }

  static constexpr float move_step = 0.05;
#define MOVE(D, V)                                                             \
  void move_##D() {                                                            \
    if (selected_robot == nullptr)                                             \
      return;                                                                  \
    selected_robot->setPos(selected_robot->pos() + V);                         \
  }
  MOVE(up, Vector(0, move_step))
  MOVE(down, Vector(0, -move_step))
  MOVE(right, Vector(move_step, 0))
  MOVE(left, Vector(-move_step, 0))
#undef MOVE
  float adpt_eval_brd(void)   { return adptv_cntrl_eval->eval(board); }
  float adpt_eval_n_brd(void) { return adptv_cntrl_eval->get_eval_n(); }
  int adpt_num(void) { return adptv_cntrl_eval->curr_n; }
  float * get_eval_n(void) { return adptv_cntrl_eval->evals_n; }
  void select_table(void) {
    bool attack = false, ball_on_enemy_field = false;
    std::pair<const Robot *, Player>
      robot_with_ball = board.getRobotWithBall();

    attack = (robot_with_ball.second == MAX);
    ball_on_enemy_field = board.isMaxLeft() ? 
      board.getBall().pos().x() < 0:
      board.getBall().pos().x() > 0;

    if (attack && ball_on_enemy_field) {
      adptv_cntrl_eval = &adptv_cntrl_eval_a_ef; 
    }
    if (attack && !ball_on_enemy_field) {
      adptv_cntrl_eval = &adptv_cntrl_eval_a_of;
    }
    if (!attack && ball_on_enemy_field) {
      adptv_cntrl_eval = &adptv_cntrl_eval_d_ef;
    }
    if (!attack && !ball_on_enemy_field) {
      adptv_cntrl_eval = &adptv_cntrl_eval_d_ef;
    }
  }

  void change_vars(void) {
    // select tables
    select_table();

    // load table values
    adptv_cntrl_eval->load();

    // get corect list of variables
    adptv_cntrl_eval->gen_var_index();
    adptv_cntrl_eval->change_var();
    adptv_cntrl_eval->go_back_var();

    // save table values
    adptv_cntrl_eval->load();
  }
};

#endif
