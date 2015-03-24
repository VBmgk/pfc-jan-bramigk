#include "optimization.h"
#include "utils.h"
#include "consts.h"
#include "vector.h"

ValuedDecision decide(Optimization &opt, State state, Player player) {
  ValuedDecision vd;

  // TODO

  // XXX: stub:
  vd.value = 666;
  FOR_TEAM_ROBOT(robot, player) {
    if (robot % N_ROBOTS == 0) {
      vd.decision.action[robot] = make_move_action(Vector(-FIELD_WIDTH / 2 + 0.5, 0));
    }
    vd.decision.action[robot] = make_move_action(Vector(-1.0, FIELD_HEIGHT * 0.9 * (robot % N_ROBOTS - 3)));
  }

  return vd;
}
