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
  static Action genMove(const Robot &, Player, const Board &, const MoveTable &);
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

/*
   class Move : public Action {
   Vector next_position;
   static constexpr float MAX_SPEED = 4; // meters per second
   float time;

   public:
   Move(const Robot &robot);

   float getTime() const;

   void apply(Player, Board &) const;

   ActionType type() const { return MOVE; }

   Vector pos() { return next_position; }

   void discreteAction(roboime::Action *a) const {
   Vector ux(1, 0), uy(0, 1);

   a->set_robot_id(getId());
   a->set_type(roboime::Action::MOVE);

   roboime::Action::Move *move = a->mutable_move();
   move->set_x(ux * next_position);
   move->set_y(uy * next_position);
   }
   };

   class Pass : public Action {
   int rcv_id;

   public:
   Pass(const Robot &r_b, const Robot &r_rcv)
   : Action(r_b.getId()), rcv_id(r_rcv.getId()) {}

   void apply(Player, Board &) const;

   ActionType type() const { return PASS; }

   int getRcvId() const { return rcv_id; }

   void discreteAction(roboime::Action *a) const {
   a->set_robot_id(Action::getId());
   a->set_type(roboime::Action::PASS);

   roboime::Action::Pass *pass = a->mutable_pass();
   pass->set_robot_id(rcv_id);
   }
   };

   class Kick : public Action {
   float speed, angle;
   static constexpr float DEFAULT_SPEED = 10; // meters per second

   public:
   Kick(const Robot &robot) : Action(robot.getId()), speed(DEFAULT_SPEED) {}

   void apply(Player, Board &) const;

   ActionType type() const { return KICK; }

   Vector point2Kick() const {
   Vector v;
// TODO: get some kick point
return v;
}

void discreteAction(roboime::Action *a) const {
Vector ux(1, 0), uy(0, 1);

a->set_robot_id(getId());
a->set_type(roboime::Action::KICK);

roboime::Action::Kick *kick = a->mutable_kick();
Vector point_to_kick = point2Kick();
kick->set_x(ux * point_to_kick);
kick->set_y(uy * point_to_kick);
}
};
*/

// typedef std::vector<std::shared_ptr<Action>> TeamAction;
// typedef std::map<int, std::shared_ptr<Move>> MoveTable;

#endif
