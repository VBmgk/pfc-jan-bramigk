#include <vector>
#include <cfloat>
#include <tuple>
#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

using namespace std;

TeamAction Minimax::decision(const Board &board) {
  return value(board, nullptr).second;
}

pair<float, TeamAction> Minimax::value(const Board &board,
                                       TeamAction *maxAction) {
  pair<float, TeamAction> v, buffer;

  if (board.isGameOver()) { // TODO: implement isTerminaç
    v.first = board.evaluate();
    v.second = board.genKickTeamAction();
  }

  else if (board.currentPlayer() == MAX) {
    v.first = FLT_MIN;
    v.second = board.genPassTeamAction();

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      TeamAction team_action = board.genPassTeamAction();

      buffer = value(board, &team_action);
      buffer.second = team_action;

      if (v.first < buffer.first)
        v = buffer;
    }
  } else {
    v.first = FLT_MAX;
    v.second = board.genPassTeamAction();

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      TeamAction team_action = board.genPassTeamAction();

      buffer = value(board.applyTeamAction(team_action), nullptr);
      buffer.second = team_action;

      if (v.first > buffer.first)
        v = buffer;
    }
  }

  return v;
}
