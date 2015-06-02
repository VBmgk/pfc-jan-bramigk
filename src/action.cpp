#include <stdio.h>
#include <random>

#include "discrete.pb.h"
#include "action.h"
#include "state.h"
#include "player.h"
#include "utils.h"
#include "vector.h"
#include "decision_table.h"
#include "decision.h"

static void update(Action *a, const Action *b) {
  switch (b->type) {
  case MOVE:
    a->move_pos = b->move_pos;
    break;
  case KICK:
    a->kick_pos = b->kick_pos;
    break;
  case PASS:
    a->pass_receiver = b->pass_receiver;
    break;
  case NONE:
    break;
  }
}

Action::Action(const Action &a) : type(a.type) { update(this, &a); }
Action &Action::operator=(const Action &a) {
  type = a.type;
  update(this, &a);
  return *this;
}

Action make_move_action(Vector move_pos) {
  Action a(MOVE);
  a.move_pos = move_pos;
  return a;
}

Action make_kick_action(Vector kick_pos) {
  Action a(KICK);
  a.kick_pos = kick_pos;
  return a;
}

Action make_pass_action(int pass_receiver) {
  Action a(PASS);
  a.pass_receiver = pass_receiver;
  return a;
}

Action gen_move_action(int robot, const State &state,
                       struct DecisionTable &table) {
  Vector pos;
  float r_radius;
  Player p = PLAYER_OF(robot);

again:
  r_radius = 0;
  switch (rand() % 3) {
  case 0:
    r_radius = MOVE_RADIUS_0;
    break;
  case 1:
    r_radius = MOVE_RADIUS_1;
    break;
  case 2:
    r_radius = MOVE_RADIUS_2;
    break;
  }
  pos = rand_vector_bounded(state.robots[robot], r_radius, FIELD_WIDTH / 2,
                            FIELD_HEIGHT / 2);

  // XXX: do not allow __ANYONE__ (temporary) to enter the defense area
  if (robot % N_ROBOTS != 0 && norm(GOAL_POS(p) - pos) <= DEFENSE_RADIUS)
    // if (norm(GOAL_POS(p) - pos) <= DEFENSE_RADIUS)
    goto again;

  // too close to the ball
  if (norm2(state.ball - pos) <= SQ(ROBOT_RADIUS + BALL_RADIUS))
    goto again;

  FOR_EVERY_ROBOT(i) if (i != robot) {
    if (norm2(state.robots[i] - pos) <= SQ(2 * ROBOT_RADIUS))
      goto again;
    if (norm2(table.move[i].move_pos - pos) <= SQ(2 * ROBOT_RADIUS))
      goto again;
  }

  // TODO: check agains move table maybe?

  return make_move_action(pos);
}

// XXX: how should the table me used here??
// Action gen_kick_action(int robot, const State &state, struct
// DecisionTable
// &table);
Action gen_kick_action(int robot, const State &state, struct DecisionTable &) {
  // XXX: can table help in any way? avoid maybe?

  float ky, kx = GOAL_X(ENEMY_OF(robot));

  int gaps_count;
  Segment gaps[N_ROBOTS * 2]; // this should be enough
  discover_gaps_from_pos(state, state.ball, ENEMY_OF(robot), gaps, &gaps_count,
                         robot);

  float max_len = 0.0;
  FOR_N(i, gaps_count) {
    float len = gaps[i].u - gaps[i].d;
    if (len > max_len) {
      max_len = len;
      ky = (gaps[i].u + gaps[i].d) / 2;
    }
  }

  return make_kick_action({kx, ky});
}

Action gen_pass_action(int robot, const State &state, DecisionTable &table) {
  Player player = PLAYER_OF(robot);
  TeamFilter receivers;
  filter_out(receivers, robot);
  discover_possible_receivers(state, &table, player, receivers, robot);
  if (receivers.count > 0) {

    // select a random receiver
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dis(0, receivers.count - 1);
    int sel = dis(gen);
    int rcv = -1;
    FOR_TEAM_ROBOT_IN(i, player, receivers) {
      if (sel-- == 0) {
        rcv = i;
        break;
      }
    }
    return make_pass_action(rcv);

  } else {
    // in case there isn't any possible pass
    // for the robot with ball, we'll make it move or kick
    if (KICK_IF_NO_PASS)
      return gen_kick_action(robot, state, table);
    else
      return gen_move_action(robot, state, table);
  }
}

Action gen_primary_action(int robot, const State &state, DecisionTable &table,
                          bool kick) {
  return kick ? gen_kick_action(robot, state, table)
              : gen_pass_action(robot, state, table);
}

void apply_to_state(Action action, int robot, State *state) {
  switch (action.type) {

  case MOVE: {
    state->robots[robot] = action.move_pos;
  } break;

  case KICK: {
    state->robots[robot] = state->ball;
    state->ball = action.kick_pos;
  } break;

  case PASS: {
    auto ball = state->ball;
    auto recv = state->robots[action.pass_receiver];
    state->robots[robot] = ball;
    state->ball = recv - unit(recv - ball) * (ROBOT_RADIUS + BALL_RADIUS);
  } break;

  case NONE:
    break;
  }
}
