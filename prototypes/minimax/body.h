class Body{
  Vector position;
  Vector speed;

public:
  Body(Vector p, Vector s):
    position(p), speed(s) {}

  Vector getPosition(){
    return position;
  }

  Vector getSpeed(){
    return speed;
  }

  void setPosition(Vector p){
    position = p;
  }

  void setSpeed(Vector s){
    speed = s;
  }
};
