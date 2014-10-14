#include <vector>
#include <cfloat>
#include <tuple>
#include "minimax.h"

TeamAction Minimax::decision(const Board &board) {
  return value(board, MAX, nullptr, 1).second;
}

std::pair<float, TeamAction> Minimax::value(const Board &board, Player player,
                                            TeamAction *max_action, int depth) {

  if (board.isGameOver() || depth >= MAX_DEPTH) {
    // TODO: implement isGameOver
    return std::make_pair(board.evaluate(), board.genKickTeamAction(player));
  }

  if (player == MAX) {
    auto v = std::make_pair(FLT_MIN, board.genPassTeamAction(MAX));

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      auto max_action = board.genPassTeamAction(MAX);

      // recurse
      auto buffer = value(board, MIN, &max_action, depth + 1);

      // minimize loss
      if (v.first < buffer.first)
        v = buffer;

      buffer.second = max_action;
    }

    return v;

  } else {
    auto v = std::make_pair(FLT_MAX, board.genPassTeamAction(MIN));

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      auto min_action = board.genPassTeamAction(MIN);
      auto next_board = board.applyTeamAction(*max_action, min_action);

      // recurse
      auto buffer = value(next_board, MAX, nullptr, depth + 1);

      // minimize loss
      if (v.first > buffer.first)
        v = buffer;

      buffer.second = min_action;
    }

    return v;
  }
}
