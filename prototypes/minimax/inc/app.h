#ifndef MINIMAX_APP_H
#define MINIMAX_APP_H

#include "minimax.h"

struct App {
  Board board;
  std::mutex board_mutex;
  TeamAction command;
  Board command_board;
  std::mutex command_mutex;
  struct {
    int uptime;
    int minimax_count;
    int pps;
    int mps;
    float val;
    bool has_val;
    float minimax_val = 0.0;
  } display;
  std::mutex display_mutex;
  bool play_minimax = false;
  bool play_minimax_once = false;
  bool eval_board_once = false;

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
    TeamAction dummy;
    board = board.applyTeamAction(command, dummy);
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
};

#endif
