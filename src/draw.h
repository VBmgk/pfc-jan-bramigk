#ifndef DRAW_H
#define DRAW_H

#include "player.h"

void screen_zoom(int width, int height, float zoom);
void draw_state(const struct State &state);
void draw_decision(const struct Decision &decision, const struct State &state, Player player);
void draw_app_status(void);

extern bool DRAW_DECISON;
extern bool DRAW_GAP;
extern bool DRAW_TIME_TO_BALL;
extern bool DRAW_POSSIBLE_RECEIVERS;
extern bool DRAW_BALL_OWNER;

void draw_options_window(void);

#endif
