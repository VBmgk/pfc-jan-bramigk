#include <vector>
#include <cfloat>
#include <tuple>
#include "minimax.h"

TeamAction Minimax::decision(const Board &board) {
  // special case for it not to crash when no robots on a team
  if (board.getMin().size() == 0 || board.getMax().size() == 0)
    return TeamAction(0);

  return value(board, MAX, nullptr, 0).second;
}

std::pair<float, TeamAction> Minimax::value(const Board &board, Player player,
                                            TeamAction *max_action, int depth) {
  // XXX: temporary!!!!!!
  // FIXME: don't verify this on odd depths
  //if (board.isGameOver()) {
  //  // TODO: implement isGameOver
  //  return std::make_pair(board.evaluate(), board.genKickTeamAction(player));
  //}

  if (depth >= MAX_DEPTH * 2) {
    return std::make_pair(board.evaluate(), TeamAction(0));
  }

  if (player == MAX) {
    auto v = std::make_pair(FLT_MIN, board.genPassTeamAction(MAX));

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      auto max_action = board.genPassTeamAction(MAX);

      // recurse
      auto buffer = value(board, MIN, &max_action, depth + 1);

      // minimize loss
      if (v.first < buffer.first) {
        v = buffer;
        v.second = max_action;
      }
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
      if (v.first > buffer.first) {
        v = buffer;
        v.second = min_action;
      }
    }

    return v;
  }
}
