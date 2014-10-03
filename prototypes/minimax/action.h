#include "robot.h"

// dimensions in meters
#define FIELD_WIDTH 4.0
#define FIELD_HIGHT 6.0

class Action{
public:
  float getTime(){
    return 0;
  }
};

class Move: public Action{
  Vector nextPosition;
  static unsigned float MAX_SPEED = 4;// meters per second
  unsigned float time;

public:
  Move(const class Robot &robot){
    Vector position = robot.lastPlanedPosition();
    srand(time(NULL));
    if(rand()%2 == 1){
      // Move with uniforme distribution
      nextPosition = robot.setURandPos();
    } else{
      // Move with normal distribution
      nextPosition = robot.setNRandPos();
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
    Pass(class Robot,class Robot);
};


class Kick: public Action{
  float speed, angle;
  static unsigned float DEFAUL_SPEED = 10;// meters per second

public:
  Kick(class Robot robot){
    speed = DEFAUL_SPEED;
    // TODO: get goal greatest area
  }
};
