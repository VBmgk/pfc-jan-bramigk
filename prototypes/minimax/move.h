class Move: public Action{
  Vector nextPosition;

public:
  Move(class Robot robot){
    Vector position = robot.lastPlanedPosition();
    srand(time(NULL));
    if(rand()%2 == 1){
      // Move with uniforme distribution
      // TODO
    } else{
      // Move with normal distribution
      // TODO
    }
  }

  float getTime(){
    // TODO
  }
};
