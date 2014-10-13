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

  for (auto &robotsActions : board.genTeamActions()) {
    float v = value(board.applyTeamActions(robotsActions));

    if (v > max_value) {
      max_value = v;
      max_action = &robotsActions;
    }
  }

  return max_action;
}

float Minimax::value(const Board &board) {
  if (board.isGameOver())
    return board.evaluate();

  else if (board.currentPlayer() == MAX) {
    float v = FLT_MIN;

    for (auto &state : genSuccessors(board)) {
      float buffer = value(state);

      if (v < buffer)
        v = buffer;
    }

    return v;
  } else {
    float v = FLT_MAX;

    for (auto state : genSuccessors(board)) {
      float buffer = value(state);

      if (v > buffer)
        v = buffer;
    }

    return v;
  }
}

vector<Board> Minimax::genSuccessors(const Board &board) {
  vector<Board> successors;

  for (auto &actions : board.genTeamActions()) {
    successors.push_back(board.applyTeamActions(actions));
  }

  return successors;
}
