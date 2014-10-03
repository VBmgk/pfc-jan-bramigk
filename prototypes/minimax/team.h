typedef arma::vec Vector;

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

class Robot: public Body{
  int id;
  Player player;

public:
  int getId(){
    return id;
  }

  void setPlayer(Player p){
    player = p;
  }

  Player getPlayer(){
    return player;
  }

  void setPositonN(){

  }
};

class Ball: public Body{
};
