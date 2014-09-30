class Team{
  vector<Robots> robots;

public:
  vector<Robots>* getRobots(){
    return &robots;
  }

  void addRobot(const Robot& robot){
    robots.push_back(robot);
  }
};
