#ifndef CONSTS_H
#define CONSTS_H

constexpr unsigned long N_ROBOTS = 6;

extern const float FIELD_WIDTH;
extern const float FIELD_HEIGHT;
extern const float GOAL_WIDTH;
extern const float GOAL_DEPTH;
extern const float DEFENSE_RADIUS;
extern const float ROBOT_RADIUS;
extern const float BALL_RADIUS;
extern const float ROBOT_MAX_SPEED;
extern const float ROBOT_KICK_SPEED;

extern const char *PROGRAM_NAME;
extern const int GUI_DEFAULT_WIDTH;
extern const int GUI_DEFAULT_HEIGHT;

#ifdef _CONST_IMPL
#define DEF_CONST(TYPE, NAME, DEFAULT, ...) TYPE NAME = DEFAULT;
#else
#define DEF_CONST(TYPE, NAME, DEFAULT, ...) extern TYPE NAME;
#endif

DEF_CONST(int, RAMIFICATION_NUMBER, 500);
DEF_CONST(int, MAX_DEPTH, 0);
DEF_CONST(float, MIN_GAP_TO_KICK, 30);
DEF_CONST(float, WEIGHT_MOVE_DIST_TOTAL, 2);
DEF_CONST(float, WEIGHT_MOVE_DIST_MAX, 2);
DEF_CONST(float, WEIGHT_MOVE_CHANGE, 2);
DEF_CONST(float, WEIGHT_PASS_CHANGE, 2);
DEF_CONST(float, WEIGHT_KICK_CHANGE, 2);
DEF_CONST(float, TOTAL_MAX_GAP_RATIO, 0.5);
DEF_CONST(float, WEIGHT_ATTACK, 1000);
DEF_CONST(float, WEIGHT_SEE_ENEMY_GOAL, 100);
DEF_CONST(float, WEIGHT_BLOCK_GOAL, 20);
DEF_CONST(float, WEIGHT_BLOCK_ATTACKER, 2000);
DEF_CONST(float, WEIGHT_RECEIVERS_NUM, 20);
DEF_CONST(float, DIST_GOAL_PENAL, 30);
DEF_CONST(float, DIST_GOAL_TO_PENAL, 1.0);
DEF_CONST(float, MOVE_RADIUS_0, 1.0);
DEF_CONST(float, MOVE_RADIUS_1, 2.0);
DEF_CONST(float, MOVE_RADIUS_2, 6.0);
DEF_CONST(bool, KICK_IF_NO_PASS, false);

#ifndef _CONST_IMPL
#undef DEF_CONST
#endif

#endif
