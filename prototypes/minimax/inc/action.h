#ifndef MINIMAX_ACTION_H
#define MINIMAX_ACTION_H

#include <memory>
#include <map>

enum Player { MIN, MAX };

enum ActionType { NONE, MOVE, PASS, KICK };

namespace roboime {
class Action;
}

struct Action {
  const ActionType type;
  int robot_id;

  union {
    Vector move_point;
    Vector kick_point;
    int pass_id;
  };

private:
  inline Action(int r, Vector p) : type(MOVE), robot_id(r), move_point(p) {}
  inline Action(int r, Vector p, int)
      : type(KICK), robot_id(r), kick_point(p) {}
  inline Action(int r, int p) : type(PASS), robot_id(r), pass_id(p) {}
  inline void update(const Action &a) {
    switch (type) {
    case MOVE:
      move_point = a.move_point;
      break;
    case KICK:
      kick_point = a.kick_point;
      break;
    case PASS:
      pass_id = a.pass_id;
      break;
    default:
      break;
    }
  }

public:
  Action() : type(NONE) {}
  Action(const Action &a) : type(a.type), robot_id(a.robot_id) { update(a); }
  Action &operator=(const Action &a) {
    // cast away the constness of type so we can change it
    *const_cast<ActionType *>(&type) = a.type;
    robot_id = a.robot_id;
    update(a);
    return *this;
  }

  static Action makeMove(int r, Vector p) { return Action(r, p); }
  static Action makeKick(int r, Vector p) { return Action(r, p, 0); }
  static Action makePass(int r, int p) { return Action(r, p); }

  typedef std::vector<Action> TeamAction;
  typedef std::map<int, Action> MoveTable;
  static Action genMove(const Robot &, Player, const Board &,
                        const MoveTable &);
  static Action genKick(const Robot &, Player, const Board &);

  /*
   * This method will dump the action into a
   * a protobuf packet that is used to communicate
   * with the core module
   */
  void discreteAction(roboime::Action *);
};

typedef Action::TeamAction TeamAction;
typedef Action::MoveTable MoveTable;

void applyAction(const Action &, Player, Board &);

#endif
