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
  friend class Board;
  std::vector<Robot> robots;
  Player player;

public:
  Team(Player p) : player(p) {}
  Team(Player p, std::vector<Robot> n_robots) : player(p), robots(n_robots) {}

  const std::vector<Robot> &getRobots() const { return robots; }

  void addRobot(const Robot &robot) {
    robot.setPlayer(player);
    robots.push_back(robot);
  }
};

class Board {
  static constexpr int MIN_AREA_TO_MARK = 30; // TODO: set correct value
  static constexpr int NUM_SAMPLE_POINTS = 300; // TODO: set correct value
  static constexpr float WEIGHT_GOAL_OPEN_AREA         = 1.0; // TODO: set correct value
  static constexpr float WEIGHT_RECEIVERS_NUM    = 1.0; // TODO: set correct value
  static constexpr float WEIGHT_DISTANCE_TO_GOAL = 1.0; // TODO: set correct value

  Ball ball;
  Team max, min;

  Player player; // current player
  float actionsMaxTime;

public:
  Board()
      : min(Player::MIN), max(Player::MAX), ball(), actionsMaxTime(FLT_MAX) {}

  Board(Team &min, Team &max)
      : min(min), max(max), ball(), actionsMaxTime(FLT_MAX) {}

  Board(Team &min, Team &max, Ball &b)
      : min(min), max(max), ball(b), actionsMaxTime(FLT_MAX) {}

  Board(const Board &b)
      : max(Player::MAX, b.max.getRobots()),
        min(Player::MIN, b.min.getRobots()) {
    player = b.player;
    actionsMaxTime = b.actionsMaxTime;
  }

  const Team &getTeam(Player p) const {
    if (p == MIN)
      return min;
    else
      return max;
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

  const Ball &getBall() const { return ball; }
  const Team &getMax() const { return max; }
  const Team &getMin() const { return min; }

  bool isGameOver() const;
  Player currentPlayer() const;

  TeamAction genActions(bool) const;
  TeamAction genKickTeamAction() const;
  TeamAction genPassTeamAction() const;

  float openGoalArea() const;
  bool freeKickLine(int point_index) const;
  std::vector<Robot> getRobotsMoving() const;
  bool kickLineCrossRobot(const int point_index, const Robot &robot) const;
  float evaluate() const;

  Board virtualStep(float time) const;
  std::vector<Robot> canGetPass() const;
  float teamActionsTime(const std::vector<class Action> &) const;
  std::vector<Robot> getRobots2Move() const;

  Player playerWithBall() const;
  Player playerWithVirtualBall(const Ball &virt_ball, const Robot robot) const;
  const Robot &getRobotWithBall() const;
  const Robot &getRobotWithVirtualBall(const Ball &) const;
  const Robot &getRobotWithVirtualBall(const Ball &virt_ball,
                                       const Robot &r_rcv) const;

  float timeToBall(const Robot &robot) const;
  float timeToVirtualBall(const Robot &robot, const Ball &ball) const;
  Board applyTeamAction(const TeamAction &) const;

  static float fieldWidth() { return 8.090; }
  static float fieldHeight() { return 6.050; }

  // TODO; set correct values
  static float goalX() { return 6.050; }
  static float goalY() { return 6.050; }
  static float goalWidth() { return 4; }
};

class Minimax {
  static constexpr int RAMIFICATION_NUMBER = 10;

public:
  TeamAction decision(const Board &);
  std::pair<float, TeamAction> value(const Board &, TeamAction *);
};

struct App {
  Board board;
  std::mutex board_mutex;
  struct {
    int uptime;
    int pps;
  } display;
  std::mutex display_mutex;

  static void run(std::function<void(App &)>);
};

#endif
