#include "minimax.h"

int Minimax::RAMIFICATION_NUMBER = 500;
int Minimax::MAX_DEPTH = 0;

float Board::MIN_GAP_TO_KICK = 30;
float Board::WEIGHT_MOVE_DIST_TOTAL = 2;
float Board::WEIGHT_MOVE_DIST_MAX = 2;
float Board::WEIGHT_MOVE_CHANGE = 2;
float Board::TOTAL_MAX_GAP_RATIO = 0.5;
float Board::WEIGHT_ATTACK = 1000;
float Board::WEIGHT_SEE_ENEMY_GOAL = 100;
float Board::WEIGHT_BLOCK_GOAL = 10;
float Board::WEIGHT_BLOCK_ATTACKER = 2000;
float Board::WEIGHT_RECEIVERS_NUM = 1;
float Board::DIST_GOAL_PENAL = 30;
float Board::DIST_GOAL_TO_PENAL = 1.0;
float Board::KICK_POS_VARIATION = .0;

bool Board::KICK_IF_NO_PASS = true;
