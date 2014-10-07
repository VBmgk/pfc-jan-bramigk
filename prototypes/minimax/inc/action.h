#ifndef MINIMAX_ACTION_H
#define MINIMAX_ACTION_H

// dimensions in meters
#define FIELD_WIDTH 4.0
#define FIELD_HIGHT 6.0

class Action{
public:
  virtual float getTime(){
    return 0;
  }
};

class Move: public Action{
  Vector nextPosition;
  static constexpr float MAX_SPEED = 4;// meters per second
  float time;

public:
  Move(const class Robot &robot) {
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
};


class Pass: public Action{
  public:
    Pass(class Robot,class Robot) {}
};


class Kick: public Action{
  float speed, angle;
  static constexpr float DEFAULT_SPEED = 10;// meters per second

public:
  Kick(class Robot robot) : speed(DEFAULT_SPEED) {}

};

#endif
