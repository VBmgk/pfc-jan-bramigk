#include <stdio.h>

#include "discrete.pb.h"
#include "decision.h"
#include "decision_table.h"
#include "state.h"
#include "action.h"
#include "consts.h"
#include "utils.h"
#include "id_table.h"
#include "segment.h"

void apply_to_state(const Decision decision, Player player,
                    struct State *state) {
  // apply all moves first
  FOR_TEAM_ROBOT(i, player) {
    if (decision.action[i].type == MOVE)
      apply_to_state(decision.action[i], i, state);
  }
  // and then all others
  FOR_TEAM_ROBOT(i, player) {
    if (decision.action[i].type != MOVE)
      apply_to_state(decision.action[i], i, state);
  }
}

Decision gen_decision(bool kick, const State &state, Player player,
                      DecisionTable &table, int robot_to_move) {
  Decision decision;

  int rwb = robot_with_ball(state);
  int rcv = -1;

  // copy the move table
  DecisionTable next_table = table;

  // push a Move action for every other robot
  FOR_TEAM_ROBOT(i, player) if (i != rwb) {
    // if (i == rcv)
    //  // decision.action[i] = make_move_action(state.robots[i]);
    //  decision.action[i] = table.move[i];
    // else
    decision.action[i] = (robot_to_move == i || robot_to_move == -1)
                             ? gen_move_action(i, state, table)
                             : table.move[i];

    next_table.move[i] = decision.action[i];
  }

  // push an action for the robot with ball, if it's us
  if (player == PLAYER_OF(rwb)) {
    auto action = decision.action[rwb] =
        gen_primary_action(rwb, state, next_table, kick);

    if (action.type == PASS) {
      rcv = action.pass_receiver;
    }
  }

  // if (rcv >= 0) {
  //  decision.action[rcv] = table.move[rcv];
  //}

  return decision;
}

Decision from_decision_table(DecisionTable &table, const State &state,
                             Player player, bool kick) {
  Decision decision;
  int rwb = robot_with_ball(state);
  if (PLAYER_OF(rwb) != player)
    rwb = -1;

  FOR_N(i, N_ROBOTS) { decision.action[i] = table.move[i]; }

  if (kick) {
    if (table.kick_robot >= 0 && table.kick_robot == rwb) {
      decision.action[rwb] = table.kick;
    } else {
      decision.action[rwb] = gen_kick_action(rwb, state, table);
    }
  } else {
#if 1
    decision.action[rwb] = gen_pass_action(rwb, state, table);
#else
    if (table.pass_robot >= 0 && table.pass_robot == rwb) {
      decision.action[table.pass_robot] = table.pass;
    } else {
      decision.action[rwb] = gen_pass_action(rwb, state, table);
    }
#endif
  }

  return decision;
}

void to_proto_action(Action &action, CommandMessage::Action *ptb_action,
                     int robot_id) {
  ptb_action->set_robot_id(robot_id);
  switch (action.type) {

  case MOVE: {
    ptb_action->set_type(CommandMessage::Action::MOVE);
    auto *move = ptb_action->mutable_move();
    move->set_x(action.move_pos.x);
    move->set_y(action.move_pos.y);
  } break;

  case PASS: {
    ptb_action->set_type(CommandMessage::Action::PASS);
    auto *pass = ptb_action->mutable_pass();
    pass->set_robot_id(action.pass_receiver % N_ROBOTS);
  } break;

  case KICK: {
    ptb_action->set_type(CommandMessage::Action::KICK);
    auto *kick = ptb_action->mutable_kick();
    kick->set_x(action.kick_pos.x);
    kick->set_y(action.kick_pos.y);
  } break;

  case NONE:
    fprintf(stderr, "WARNING: tried to dispatch NONE action for robot "
                    "%i, ignored\n",
            robot_id);
  }
}

void to_proto_command(const Decision &decision, Player player,
                      CommandMessage &ptb_command, const IdTable &table) {
  FOR_TEAM_ROBOT(i, player) {
    Action &&action = decision.action[i];
    if (action.type != NONE) {
      CommandMessage::Action *ptb_action = ptb_command.add_action();
      to_proto_action(action, ptb_action, table.id[i]);
    }
  }
}
