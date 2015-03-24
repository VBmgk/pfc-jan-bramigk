#ifndef APP_H
#define APP_H

#include <functional>

void app_run(std::function<void(void)> loop_func);
void app_random();
void app_decide_once();
void app_decide_toggle();
void app_eval_once();
void app_apply();
void app_toggle_experimental();
void app_select_save_slot(int slot);
void app_load_state();
void app_save_state();
void app_toggle_selected_player();
void app_select_next_robot();
void app_move_up();
void app_move_down();
void app_move_left();
void app_move_right();

extern const struct State *app_state;

#endif
