#ifndef STATE_H
#define STATE_H

#include "consts.h"
#include "vector.h"
#include "array.h"
#include "player.h"
#include "segment.h"
#include "filter.h"

struct State {
  Vector ball, ball_v;
  GameArray<Vector> robots, robots_v;
};

struct Decision;
struct DecisionTable;

State uniform_rand_state();

bool can_kick_directly(const State state, Player player);

int robot_with_ball(const State state);

float total_gap_len_from_pos(const State state, Vector pos, Player player, int ignore_robot = -1);

float max_gap_len_from_pos(const State state, Vector pos, Player player, int ignore_robot = -1);

float time_to_pos(Vector robot_p, Vector robot_v, Vector pos, Vector pos_v, float max_speed = ROBOT_MAX_SPEED);

void discover_gaps_from_pos(const State state, Vector pos, Player player, Segment *gaps, int *gaps_count,
                            int ignore_robot = -1);

float evaluate_with_decision(Player player, const State &state, const Decision &decision, const DecisionTable &table);

void discover_possible_receivers(const State state, const DecisionTable *table, Player player, TeamFilter &result);

namespace roboime {
class Update;
}
void update_from_proto(State &state, roboime::Update &ptb_update, struct IdTable &table);

#endif
