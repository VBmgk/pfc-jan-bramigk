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
};
