class Minimax{
public:
  vector<class Action>* decision(class Board);
  float getValue(class Board);
  vector<class Board> getSuccessors(class Board);
};

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

#define MIN_AREA_TO_MARK 30 // TODO: set correct value

typedef enum {MIN, MAX} Player;

class Board{
  Ball ball;
  Team max, min;

  Player player;// current player

public:
  bool isGameOver();
  Player currentPlayer();
  vector<Action> getActions();
  vector<vector<class Action> > getRobotsActions();
  float evaluate();
  float openGoalArea();
  Player playerWithBall();
  vector<Robot> canGetPass();
  Robot getRobotWithBall();
  Board applyRobotsActions(vector<class Action>);
  float getRobotsActionsTime(vector<class Action>);
  vector<class Robot> getRobots2Move();
};
