#ifndef UTILS_H
#define UTILS_H

#include "consts.h"
#include "player.h"

#define FOR_EVERY_ROBOT(I) for (int I = 0; I < 2 * N_ROBOTS; I++)
#define FOR_TEAM_ROBOT(I, T) for (int I = T * N_ROBOTS; I < (1 + T) * N_ROBOTS; I++)
#define FOR_EVERY_ROBOT_FILTERED(I, F) FOR_EVERY_ROBOT(I) if (F[I])
#define FOR_TEAM_ROBOT_FILTERED(I, T, F) FOR_TEAM_ROBOT(I, T) if (F[I])

inline Player PLAYER_FOR(int ROBOT) { return ROBOT / N_ROBOTS == MIN ? MIN : MAX; }

#endif
