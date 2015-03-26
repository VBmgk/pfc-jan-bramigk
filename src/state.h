#ifndef STATE_H
#define STATE_H

#include "consts.h"
#include "vector.h"
#include "player.h"
#include "segment.h"

struct State {
  Vector ball;
  Vector ball_v;
  Vector robots[2 * N_ROBOTS];
  Vector robots_v[2 * N_ROBOTS];
};

State uniform_rand_state();

bool can_kick_directly(const State state, Player player);

int robot_with_ball(const State state);

float total_gap_len_from_pos(const State state, Vector pos, Player player, int ignore_robot = -1);

float max_gap_len_from_pos(const State state, Vector pos, Player player, int ignore_robot = -1);

float time_to_pos(Vector robot_p, Vector robot_v, Vector pos, Vector pos_v);

void discover_gaps_from_pos(const State state, Vector pos, Player player, Segment *gaps, int *gaps_count,
                            int ignore_robot = -1);

float evaluate_with_decision(Player player, const State &state, const struct Decision &decision,
                             const struct DecisionTable &table);

#endif
