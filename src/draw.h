#ifndef DRAW_H
#define DRAW_H

#include "player.h"

void screen_zoom(int width, int height, float zoom);
void draw_state(const struct State &state);
void draw_decision(const struct Decision &decision, const struct State &state, Player player);

#endif
