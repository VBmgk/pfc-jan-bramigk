#include <vector>
#include <cfloat>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

using namespace std;

vector<Action> *Minimax::decision(const Board &board) {
  float max_value = FLT_MIN;
  vector<Action> *max_action = nullptr;

  for (auto &robotsActions : board.getRobotsActions()) {
    float value = getValue(board.applyRobotsActions(robotsActions));

    if (value > max_value) {
      max_value = value;
      max_action = &robotsActions;
    }
  }

  return max_action;
}

float Minimax::getValue(const Board &board) {
  if (board.isGameOver())
    return board.evaluate();

  else if (board.currentPlayer() == MAX) {
    float value = FLT_MIN;

    for (auto &state : getSuccessors(board)) {
      float buffer = getValue(state);

      if (value < buffer)
        value = buffer;
    }

    return value;
  } else {
    float value = FLT_MAX;

    for (auto state : getSuccessors(board)) {
      float buffer = getValue(state);

      if (value > buffer)
        value = buffer;
    }

    return value;
  }
}

vector<Board> Minimax::getSuccessors(const Board &board) {
  vector<Board> successors;

  for (auto &actions : board.getRobotsActions()) {
    successors.push_back(board.applyRobotsActions(actions));
  }

  return successors;
}
