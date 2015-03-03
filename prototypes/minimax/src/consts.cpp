#include "minimax.h"

int Minimax::RAMIFICATION_NUMBER = 1000;
int Minimax::MAX_DEPTH = 0;

float Board::MIN_GAP_TO_WIN = 0.1;
float Board::WEIGHT_TOTAL_GAP = 100.0;
float Board::WEIGHT_TOTAL_GAP_TEAM = 3.0;
float Board::WEIGHT_MAX_GAP = 2.0;
float Board::WEIGHT_RECEIVERS_NUM = 1.0;
float Board::WEIGHT_DISTANCE_TO_GOAL = 1.0;
float Board::WEIGHT_MOVE = 1.0;
