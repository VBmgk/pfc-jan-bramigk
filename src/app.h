#ifndef APP_H
#define APP_H

#include <functional>

void app_run(std::function<void(void)> loop_func, bool play_as_max = true);
void app_random();
void app_decide_once();
void app_decide_toggle();
void app_eval_once();
void app_eval_toggle();
void app_apply();
void app_toggle_experimental();
void app_select_save_slot(int slot);
void app_load_state();
void app_save_state();
void app_toggle_selected_player();
void app_select_next_robot();
void app_select_ball();
void app_move_up();
void app_move_down();
void app_move_left();
void app_move_right();
void app_save_params(const char *filename);
void app_load_params(const char *filename);

extern const struct State *app_state;
extern const struct Decision *app_decision_max;
extern const struct Decision *app_decision_min;
extern const struct DecisionTable *app_decision_table;
extern const int *app_selected_robot;
extern struct Suggestions *app_suggestions;
extern int app_selected_suggestion;

#include "decision_source.h"
extern DecisionSource *app_decision_source;

#endif
