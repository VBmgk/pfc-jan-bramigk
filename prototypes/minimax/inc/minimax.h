#ifndef MINIMAX_MINIMAX_H
#define MINIMAX_MINIMAX_H

#include "base.h"
#include "body.h"

#include <vector>
#include <functional>
#include <mutex>

class Minimax {
 public:
  std::vector<class Action>* decision(class Board);
  float getValue(class Board);
  std::vector<class Board> getSuccessors(class Board);

  static void run_minimax(std::function<void(Board&, std::mutex&)>);
};

class Team {
  std::vector<Robot> robots;
  Player player;

 public:
  Team(Player p) : player(p) {}

  std::vector<Robot>& getRobots() { return robots; }
  const std::vector<Robot>& getRobots() const { return robots; }

  void addRobot(const Robot& robot) {
    robot.setPlayer(player);
    robots.push_back(robot);
  }
};

class Board {
  static constexpr int RAMIFICATION_NUMBER = 10;
  static constexpr int MIN_AREA_TO_MARK = 30; // TODO: set correct value

  Ball ball;
  Team max, min;

  Player player;  // current player

public:
  Board() : min(Player::MIN), max(Player::MAX), ball() {}
  Board(Team &min, Team &max) : min(min), max(max), ball() {}
  Board(Team &min, Team &max, Ball &b) : min(min), max(max), ball(b) {}

  Team& GetTeam(Player p) {
    if(p == MIN) return min;
    else return max;
  }

  static Board randomBoard() {
    Board b;
    for (int i = 0; i < 5; i++) {
      Robot r(i);
      r.setPos(Vector::getURand(fieldWidth(), fieldHeight()));
      b.min.addRobot(r);
      r.setPos(Vector::getURand(fieldWidth(), fieldHeight()));
      b.max.addRobot(r);
    }
    b.ball.setPos(Vector::getURand(fieldWidth(), fieldHeight()));
    return b;
  }

  const Ball& getBall() const { return ball; }
  const Team& getMax() const { return max; }
  const Team& getMin() const { return min; }

  bool isGameOver();
  Player currentPlayer();
  std::vector<Action> getActions();
  std::vector<std::vector<class Action> > getRobotsActions();
  float evaluate();
  float openGoalArea();
  Player playerWithBall();
  std::vector<Robot> canGetPass();
  Robot& getRobotWithBall();
  float getRobotsActionsTime(const std::vector<class Action> &);
  std::vector<Robot>& getRobots2Move();
  float getTimeToBall(const Robot& robot);
  float getTimeToVirtualBall(const Robot& robot, const Ball ball);
  Board applyRobotsActions(const std::vector<class Action> &);

  static float fieldWidth()  { return 8.090; }
  static float fieldHeight() { return 6.050; }
};

#endif
