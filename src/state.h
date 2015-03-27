#ifndef STATE_H
#define STATE_H

#include "consts.h"
#include "vector.h"
#include "array.h"
#include "player.h"
#include "segment.h"

struct State {
  Vector ball, ball_v;
  GameArray<Vector> robots, robots_v;
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

namespace roboime {
class Update;
}
void update_from_proto(State &state, roboime::Update &ptb_update, struct IdTable &table);

#endif
