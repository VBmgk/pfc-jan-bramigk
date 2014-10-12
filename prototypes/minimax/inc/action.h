#ifndef MINIMAX_ACTION_H
#define MINIMAX_ACTION_H

// dimensions in meters
#define FIELD_WIDTH 4.0
#define FIELD_HIGHT 6.0

class Action{
  Player player;
  int robot_id;

public:
  Action(Player p, int r_id) :
    player(p), robot_id(r_id) {}

  virtual float getTime(){
    return 0;
  }

  virtual void apply(Board &board) const {};
};


class Move: public Action{
  Vector nextPosition;
  static constexpr float MAX_SPEED = 4;// meters per second
  float time;

public:
  Move(const class Robot &robot) :
    Action(robot.getPlayer(), robot.getId()) {
    Vector position = robot.getLastPlanedPos();
    //srand(time(NULL));//XXX: ????
    if(rand()%2 == 1){
      // Move with uniforme distribution
      nextPosition = robot.getURandPos();
    } else{
      // Move with normal distribution
      nextPosition = robot.getNRandPos();
    }
    // Compute minimum time
    time = robot.getDist(nextPosition) / MAX_SPEED;
  }

  float getTime(){
    return time;
  }

  void apply(Board &b) const {
  }
};


class Pass: public Action{
  int rcv_id;

public:
  Pass(Robot r_b,Robot r_rcv) :
    Action(r_b.getPlayer(), r_b.getId()), rcv_id(r_rcv.getId()) {}

  void apply(Board &b) const {
  }
};


class Kick: public Action{
  float speed, angle;
  static constexpr float DEFAULT_SPEED = 10;// meters per second

public:
  Kick(Robot robot) :
    Action(robot.getPlayer(), robot.getId()), speed(DEFAULT_SPEED) {}

  void apply(Board &b) const {
  }
};

#endif
