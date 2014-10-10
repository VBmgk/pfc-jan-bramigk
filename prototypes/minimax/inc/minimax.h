#ifndef MINIMAX_MINIMAX_H
#define MINIMAX_MINIMAX_H

#include "base.h"
#include "body.h"
#include "action.h"

#include <cfloat>
#include <vector>
#include <functional>
#include <mutex>

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
  float actionsMaxTime;

public:
  Board() : min(Player::MIN), max(Player::MAX), ball(){
    actionsMaxTime = FLT_MAX;
  }

  Board(Team &min, Team &max) : min(min), max(max), ball(){
    actionsMaxTime = FLT_MAX;
  }

  Board(Team &min, Team &max, Ball &b) : min(min), max(max), ball(b){
    actionsMaxTime = FLT_MAX;
  }

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

  bool isGameOver() const;
  Player currentPlayer() const;
  std::vector<Action> getActions() const;
  std::vector<std::vector<class Action> > getRobotsActions() const;
  float evaluate() const;
  float openGoalArea() const;
  Player playerWithBall() const;
  std::vector<Robot> canGetPass() const;
  Robot getRobotWithBall() const;
  float getRobotsActionsTime(const std::vector<class Action> &) const;
  std::vector<Robot> getRobots2Move() const;
  float getTimeToBall(const Robot& robot) const;
  float getTimeToVirtualBall(const Robot& robot, const Ball ball) const;
  Board applyRobotsActions(const std::vector<class Action> &) const;

  static float fieldWidth()  { return 8.090; }
  static float fieldHeight() { return 6.050; }
};

class Minimax {
 public:
  std::vector<class Action>* decision(const Board&);
  float getValue(const Board&);
  std::vector<class Board>& getSuccessors(const Board&);

  static void run_minimax(std::function<void(Board&, std::mutex&)>);
};

#endif
