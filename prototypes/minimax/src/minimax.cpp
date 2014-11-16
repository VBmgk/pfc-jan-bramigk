#include <vector>
#include <cfloat>
#include <tuple>
#include "minimax.h"

TeamAction Minimax::decision(const Board &board) {
  return std::get<1>(decision_value(board));
}

std::tuple<float, TeamAction> Minimax::decision_value(const Board &board) {
  // special case for it not to crash when no robots on a team
  if (board.getMin().size() == 0 || board.getMax().size() == 0)
    return std::make_pair(0.0, TeamAction(0));

  return value(board, MAX, nullptr, 0);
}

std::tuple<float, TeamAction> Minimax::value(const Board &board, Player player,
                                             TeamAction *max_action,
                                             int depth) {
  if (depth % 2 == 0 && board.isGameOver()) {
    return std::make_pair(board.evaluate(), board.genKickTeamAction(player));
  }

  if (depth >= MAX_DEPTH * 2) {
    return std::make_pair(board.evaluate(), TeamAction(0));
  }

  if (player == MAX) {
    auto v = std::make_pair(-std::numeric_limits<float>::infinity(),
                            board.genPassTeamAction(MAX));

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      auto max_action = board.genPassTeamAction(MAX);

      // recurse
      float val;
      std::tie(val, std::ignore) = value(board, MIN, &max_action, depth + 1);

      // minimize loss
      if (v.first < val) {
        v.first = val;
        v.second = max_action;
      }
    }

    return v;

  } else {
    auto v = std::make_pair(std::numeric_limits<float>::infinity(),
                            board.genPassTeamAction(MIN));

    for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
      auto min_action = board.genPassTeamAction(MIN);
      auto next_board = board.applyTeamAction(*max_action, min_action);

      // recurse
      float val;
      std::tie(val, std::ignore) = value(next_board, MAX, nullptr, depth + 1);

      // minimize loss
      if (v.first > val) {
        v.first = val;
        v.second = min_action;
      }
    }

    return v;
  }
}

std::tuple<float, TeamAction, TeamAction>
Minimax::decision_experimental(const Board &board) {
  // special case for it not to crash when no robots on a team
  if (board.getMin().size() == 0 || board.getMax().size() == 0)
    return std::make_tuple(0.0, TeamAction(0), TeamAction(0));

// if (board.isGameOver()) {
//  return std::make_pair(board.evaluate(), board.genKickTeamAction(player));
//}

#define N RAMIFICATION_NUMBER

  float payoff_matrix[N][N];
  TeamAction max_choices[N], min_choices[N];

  // generate payoff matrix
  // std::cout << "--" << std::endl;
  for (int i = 0; i < N; i++) {
    max_choices[i] = board.genPassTeamAction(MAX);

    for (int j = 0; j < N; j++) {
      min_choices[j] = board.genPassTeamAction(MIN);

      auto next_board = board.applyTeamAction(max_choices[i], min_choices[j]);
      // TODO: maybe check if game is over or recurse
      payoff_matrix[i][j] = next_board.evaluate();
      // std::cout << payoff_matrix[i][j] << ",";
    }
    // std::cout << std::endl;
  }

  int best_max = 0, best_min = 0;
  float best_max_value = -std::numeric_limits<float>::infinity(),
        best_min_value = std::numeric_limits<float>::infinity();

  // TODO: consider mixed strategy maybe

  // chose max with minimax decision
  for (int i = 0; i < N; i++) {
    float min_value = std::numeric_limits<float>::infinity();
    for (int j = 0; j < N; j++)
      min_value = std::min(min_value, payoff_matrix[i][j]);
    if (min_value > best_max_value) {
      best_max_value = min_value;
      best_max = i;
    }
  }

  // chose min with minimax decision
  for (int j = 0; j < N; j++) {
    float max_value = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < N; i++)
      max_value = std::max(max_value, payoff_matrix[i][j]);
    if (max_value < best_min_value) {
      best_min_value = max_value;
      best_min = j;
    }
  }

  return std::make_tuple(best_max_value, max_choices[best_max],
                         min_choices[best_min]);
}
