#ifndef UTILS_H
#define UTILS_H

#include <cmath>

#include "consts.h"
#include "player.h"
#include "vector.h"

#define FOR_RANGE(I, F, T) for (int I = (F); I < (T); I++)
#define FOR_N(I, N) FOR_RANGE(I, 0, N)
#define FOR_N_IN(I, N, F) FOR_RANGE(I, 0, N) if (!F[I])
#define FOR_EVERY_ROBOT(I) FOR_RANGE(I, 0, 2 * N_ROBOTS)
#define FOR_TEAM_ROBOT(I, T) FOR_RANGE(I, T *N_ROBOTS, (1 + T) * N_ROBOTS)
#define FOR_EVERY_ROBOT_IN(I, F) FOR_EVERY_ROBOT(I) if (!F[I])
#define FOR_TEAM_ROBOT_IN(I, T, F) FOR_TEAM_ROBOT(I, T) if (!F[I])

constexpr int ROBOT_WITH_PLAYER(int R, Player P) {
  return P * N_ROBOTS + R % N_ROBOTS;
}

constexpr Player ENEMY_FOR(Player P) { return P == MIN ? MAX : MIN; }
constexpr Player PLAYER_OF(int R) { return R / N_ROBOTS == MIN ? MIN : MAX; }
constexpr Player ENEMY_OF(int R) { return ENEMY_FOR(PLAYER_OF(R)); }

// 1 for MIN -1 for MAX
constexpr int PLAYER_SIGN(Player P) { return P == MAX ? 1 : -1; }

// XXX: MAX to the left, not taking into account that it may be
// otherwise
constexpr float GOAL_Y(Player) { return 0; }
constexpr float GOAL_X(Player P) { return -PLAYER_SIGN(P) * FIELD_WIDTH / 2; }
constexpr Vector GOAL_POS(Player P) { return {GOAL_X(P), GOAL_Y(P)}; }

// angle mesures conversions radians <-> degrees
template <typename T> constexpr T RADIANS(T DEGREES) {
  return M_PI * DEGREES / 180.;
}
template <typename T> constexpr T DEGREES(T RADIANS) {
  return 180. * RADIANS / M_PI;
}

// a number squared
template <typename T> constexpr T SQ(T X) { return X * X; }

#endif
