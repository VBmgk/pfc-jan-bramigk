#ifndef DRAW_H
#define DRAW_H

#include "player.h"

void screen_zoom(int width, int height, double zoom, double center_x,
                 double center_y);
void draw_state(const struct State &state);
void draw_decision(const struct Decision &decision, const struct State &state,
                   Player player);
void draw_suggestion(const struct SuggestionTable &table);
void draw_app_status(void);
void draw_options_window(void);

#endif
