#ifndef CONSTS_H
#define CONSTS_H

constexpr unsigned long N_ROBOTS = 6;

// Units are SI: m, m/s, s, ...

constexpr float LINE_WIDTH = 0.010;
constexpr float FIELD_WIDTH = 8.090;
constexpr float FIELD_HEIGHT = 6.050;
constexpr float GOAL_WIDTH = 1.000;
constexpr float GOAL_DEPTH = 0.180;
constexpr float GOAL_WALL_WIDTH = 0.020;
constexpr float CENTER_CIRCLE_RADIUS = 0.500;
constexpr float DEFENSE_RADIUS = 1.000;
constexpr float DEFENSE_STRETCH = 0.500;
constexpr float BOUNDARY_WIDTH = 0.500;
constexpr float REFEREE_WIDTH = 0.250;
constexpr float ROBOT_RADIUS = 0.180 / 2;
constexpr float BALL_RADIUS = 0.043 / 2;
constexpr float ROBOT_MAX_SPEED = 3.0;
constexpr float ROBOT_KICK_SPEED = 6.0;

constexpr const char *PROGRAM_NAME = "AI for RoboIME"; // TODO: better name maybe?
constexpr int GUI_DEFAULT_WIDTH = 944;
constexpr int GUI_DEFAULT_HEIGHT = 740;

#ifdef _CONST_IMPL
#define PARAM(TYPE, NAME, DEFAULT, ...) TYPE NAME = DEFAULT;
#else
#define PARAM(TYPE, NAME, DEFAULT, ...) extern TYPE NAME;
#endif

PARAM(bool, CONSTANT_RATE, true);
PARAM(bool, KICK_IF_NO_PASS, false);
PARAM(int, DECISION_RATE, 7);
PARAM(int, RAMIFICATION_NUMBER, 5000);
PARAM(int, FULL_CHANGE_PERCENTAGE, 100);
PARAM(int, MAX_DEPTH, 0);
PARAM(float, KICK_POS_VARIATION, 0.150);
PARAM(float, MIN_GAP_TO_KICK, 9.0);
PARAM(float, WEIGHT_MOVE_DIST_MAX, 0);
PARAM(float, WEIGHT_MOVE_DIST_TOTAL, 0);
PARAM(float, WEIGHT_MOVE_CHANGE, 2);
PARAM(float, WEIGHT_PASS_CHANGE, 2);
PARAM(float, WEIGHT_KICK_CHANGE, 2);
PARAM(float, TOTAL_MAX_GAP_RATIO, 0.5);
PARAM(float, WEIGHT_ATTACK, 1000);
PARAM(float, WEIGHT_SEE_ENEMY_GOAL, 10);
PARAM(float, WEIGHT_BLOCK_GOAL, 180);
PARAM(float, WEIGHT_BLOCK_ATTACKER, 5000);
PARAM(float, WEIGHT_RECEIVERS_NUM, 20);
PARAM(float, WEIGHT_ENEMY_RECEIVERS_NUM, 20);
PARAM(float, DIST_GOAL_PENAL, 2000);
PARAM(float, DIST_GOAL_TO_PENAL, 1.0);
PARAM(float, MOVE_RADIUS_0, 0.5);
PARAM(float, MOVE_RADIUS_1, 2.0);
PARAM(float, MOVE_RADIUS_2, 7.0);

#ifndef _CONST_IMPL
#undef PARAM
#endif

#endif
