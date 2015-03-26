#include <stdio.h>

#include "action.h"
#include "state.h"
#include "discrete.pb.h"
#include "player.h"
#include "utils.h"
#include "vector.h"

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

Action gen_move_action(int robot, const State &state) {
  Vector pos;
  bool ok = true;
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
  pos = rand_vector_bounded(state.robots[robot], r_radius, FIELD_WIDTH / 2, FIELD_HEIGHT / 2);

  // XXX: do not allow __ANYONE__ (temporary) to enter the defense area
  if (norm(GOAL_POS(p) - pos) <= DEFENSE_RADIUS)
    goto again;

  // too close to the ball
  if (norm(state.ball - pos) <= ROBOT_RADIUS + BALL_RADIUS)
    goto again;

  FOR_EVERY_ROBOT(i) if (i != robot) {
    if (norm(state.robots[i] - pos) <= 2 * ROBOT_RADIUS)
      goto again;
  }

  // TODO: check agains move table maybe?

  return make_move_action(pos);
}

Action gen_kick_action(int robot, const State &state) {

  float ky, kx = GOAL_X(ENEMY_OF(robot));

  int gaps_count;
  Segment gaps[N_ROBOTS * 2]; // this should be enough
  discover_gaps_from_pos(state, state.ball, ENEMY_OF(robot), gaps, &gaps_count, robot);

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

Action gen_pass_action(int robot, const State &state); // TODO

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

void to_proto_action(Action &action, roboime::Action *ptb_action, int robot_id) {
  ptb_action->set_robot_id(robot_id);
  switch (action.type) {

  case MOVE: {
    ptb_action->set_type(roboime::Action::MOVE);
    auto *move = ptb_action->mutable_move();
    move->set_x(action.move_pos.x);
    move->set_y(action.move_pos.y);
  } break;

  case PASS: {
    ptb_action->set_type(roboime::Action::PASS);
    auto *pass = ptb_action->mutable_pass();
    pass->set_robot_id(action.pass_receiver);
  } break;

  case KICK: {
    ptb_action->set_type(roboime::Action::KICK);
    auto *kick = ptb_action->mutable_kick();
    kick->set_x(action.kick_pos.x);
    kick->set_y(action.kick_pos.y);
  } break;

  case NONE:
    fprintf(stderr, "WARNING: tried to dispatch NONE action, ignored");
  }
}
