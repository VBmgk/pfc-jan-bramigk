#include <vector>
#include <cfloat>
#include <tuple>
#include "minimax.h"

TeamAction Minimax::decision(const Board &board) {
  return std::get<1>(decision_value(board));
}

std::tuple<float, TeamAction, TeamAction>
Minimax::decision_value(const Board &board) {
  // special case for it not to crash when no robots on a team
  if (board.getMin().size() == 0 || board.getMax().size() == 0)
    return std::make_tuple(0.0, TeamAction(0), TeamAction(0));

  float value;
  TeamAction action_max, action_min;

  if (MAX_DEPTH > 0) {
    std::tie(value, action_max, action_min) = value_max(board, 0);

    // build min move table
    move_table_min.clear();
    for (auto a : action_min) {
      if (a->type() == Action::MOVE) {
        auto move_action = std::dynamic_pointer_cast<Move>(a);
        int robot_id = move_action->getId();
        move_table_min[robot_id] = move_action;
      }
    }
  } else {
    std::tie(value, action_max) = value_max_only(board);
    ;
  }

  // build max move table
  move_table_max.clear();
  for (auto a : action_max) {
    if (a->type() == Action::MOVE) {
      auto move_action = std::dynamic_pointer_cast<Move>(a);
      int robot_id = move_action->getId();
      move_table_max[robot_id] = move_action;
    }
  }

  move_count++;

  // we're done, go ahead and return
  return std::make_tuple(value, action_max, action_min);
}

std::tuple<float, TeamAction, TeamAction> Minimax::value_max(const Board board,
                                                             int depth) {
  if (depth >= MAX_DEPTH) {
    return std::make_tuple(board.evaluate(), TeamAction(0), TeamAction(0));
  }

  auto mtable = move_table_max;
  int MTABLE_COUNT = RAMIFICATION_NUMBER - 2;

  auto robots = board.getTeam(MAX).getRobots();
  int move_id = robots[move_count % robots.size()].getId();

  if (board.isGameOver(MAX)) {
    return std::make_tuple(board.evaluate(),
                           board.genKickTeamAction(MAX, mtable), TeamAction(0));
  }

  auto v = std::make_tuple(-std::numeric_limits<float>::infinity(),
                           board.genPassTeamAction(MAX, mtable), TeamAction(0));

  for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
    auto max_action = i == 0
                          ? board.genPassTeamAction(MAX, mtable)
                          : i < MTABLE_COUNT
                                ? board.genPassTeamAction(MAX, mtable, move_id)
                                : board.genPassTeamAction(MAX);

    // recurse
    float val;
    TeamAction min_action;
    std::tie(val, min_action) = value_min(board, max_action, depth);

    // cost of moves
    for (auto move : max_action) {
      if (move->type() != Action::ActionType::MOVE)
        continue;

      for (auto robot : robots)
        if (robot.getId() == move->getId())
          val += Board::WEIGHT_MOVE * (move->pos() - robot.pos()).norm();
    }

    // minimize loss for max
    if (std::get<0>(v) < val) {
      std::get<0>(v) = val;
      std::get<1>(v) = max_action;
      std::get<2>(v) = min_action;
    }
  }

  return v;
}

std::tuple<float, TeamAction>
Minimax::value_min(const Board board, TeamAction max_action, int depth) {
  auto mtable = move_table_min;
  int MTABLE_COUNT = RAMIFICATION_NUMBER - 2;

  auto robots = board.getTeam(MIN).getRobots();
  int move_id = robots[move_count % robots.size()].getId();

  if (board.isGameOver(MIN)) {
    auto min_action = board.genKickTeamAction(MIN, mtable);
    auto next_board = board.applyTeamAction(max_action, min_action);
    return std::make_tuple(board.evaluate(), min_action);
  }

  auto v = std::make_pair(std::numeric_limits<float>::infinity(),
                          board.genPassTeamAction(MIN, mtable));

  for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
    auto min_action = i == 0
                          ? board.genPassTeamAction(MIN, mtable)
                          : i < MTABLE_COUNT
                                ? board.genPassTeamAction(MIN, mtable, move_id)
                                : board.genPassTeamAction(MIN);
    auto next_board = board.applyTeamAction(max_action, min_action);

    // recurse
    float val;
    std::tie(val, std::ignore, std::ignore) = value_max(next_board, depth + 1);

    // cost of moves
    for (auto move : max_action) {
      if (move->type() != Action::ActionType::MOVE)
        continue;

      for (auto robot : robots)
        if (robot.getId() == move->getId())
          val -= Board::WEIGHT_MOVE * (move->pos() - robot.pos()).norm();
    }

    // minimize loss for min
    if (std::get<0>(v) > val) {
      std::get<0>(v) = val;
      std::get<1>(v) = min_action;
    }
  }

  return v;
}

std::tuple<float, TeamAction> Minimax::value_max_only(const Board board) {
  auto mtable = move_table_max;
  int MTABLE_COUNT = RAMIFICATION_NUMBER - 2;

  auto robots = board.getTeam(MAX).getRobots();
  int move_id = robots[move_count % robots.size()].getId();

  if (board.isGameOver(MAX)) {
    auto max_action = board.genKickTeamAction(MAX, mtable);
    auto next_board = board.applyTeamAction(max_action, TeamAction(0));
    return std::make_tuple(next_board.evaluate(), max_action);
  }

  auto v = std::make_tuple(-std::numeric_limits<float>::infinity(),
                           board.genPassTeamAction(MAX, mtable));

  for (int i = 0; i < RAMIFICATION_NUMBER; i++) {
    auto max_action = i == 0
                          ? board.genPassTeamAction(MAX, mtable)
                          : i < MTABLE_COUNT
                                ? board.genPassTeamAction(MAX, mtable, move_id)
                                : board.genPassTeamAction(MAX);

    auto next_board = board.applyTeamAction(max_action, TeamAction(0));
    float val = next_board.evaluate();

    // minimize loss for max
    if (std::get<0>(v) < val) {
      std::get<0>(v) = val;
      std::get<1>(v) = max_action;
    }
  }

  return v;
}

std::tuple<float, TeamAction, TeamAction>
Minimax::decision_experimental(const Board &board) {
  // special case for it not to crash when no robots on a team
  if (board.getMin().size() == 0 || board.getMax().size() == 0)
    return std::make_tuple(0.0, TeamAction(0), TeamAction(0));

// if (board.isGameOver()) {
//  return std::make_pair(board.evaluate(), board.genKickTeamAction(player));
//}

#define N 1000
  int n = std::min(RAMIFICATION_NUMBER, N);

  float payoff_matrix[N][N];
  TeamAction max_choices[N], min_choices[N];

  // generate payoff matrix
  // std::cout << "--" << std::endl;
  for (int i = 0; i < n; i++) {
    max_choices[i] = board.genPassTeamAction(MAX);

    for (int j = 0; j < n; j++) {
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
  for (int i = 0; i < n; i++) {
    float min_value = std::numeric_limits<float>::infinity();
    for (int j = 0; j < n; j++)
      min_value = std::min(min_value, payoff_matrix[i][j]);
    if (min_value > best_max_value) {
      best_max_value = min_value;
      best_max = i;
    }
  }

  // chose min with minimax decision
  for (int j = 0; j < n; j++) {
    float max_value = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < n; i++)
      max_value = std::max(max_value, payoff_matrix[i][j]);
    if (max_value < best_min_value) {
      best_min_value = max_value;
      best_min = j;
    }
  }

  return std::make_tuple(best_max_value, max_choices[best_max],
                         min_choices[best_min]);
}
