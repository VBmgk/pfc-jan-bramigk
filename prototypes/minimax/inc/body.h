class Body{
  Vector position;
  Vector speed;

public:
  Body(){}

  Body(Vector p, Vector s):
    position(p), speed(s) {}

  Vector pos() const {
    return position;
  }

  Vector v() const {
    return speed;
  }

  void setPos(Vector p){
    position = p;
  }

  void setV(Vector s){
    speed = s;
  }

  float getDist(Vector target) const{
    return (position - target).norm();
  }
};

typedef enum {MIN, MAX} Player;

class Robot: public Body{
  int id;
  Player player;
  Vector last_planed_pos;
  static const float MAX_SPEED_SQUARE = 16;// 4 m/s * 4 m/s

public:
  Robot(int id):
    id(id){
    player = MIN;
    Vector zero;
  }

  Robot(int id, Vector pos, Vector speed):
    Body(pos, speed), id(id){
    player = MIN;
    Vector zero;
  }

  int getId() const {
    return id;
  }

  void setPlayer(Player p){
    player = p;
  }

  Player getPlayer(){
    return player;
  }

  Vector getNRandPos() const {
    return Vector::getNRand(last_planed_pos);
  }

  Vector getURandPos() const {
    return Vector::getURand();
  }

  Vector getLastPlanedPos() const {
    return last_planed_pos;
  }

  float maxV2() const {
    return MAX_SPEED_SQUARE;
  }
};

class Ball: public Body{
};
