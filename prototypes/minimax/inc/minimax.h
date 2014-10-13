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
  Team(Player p, std::vector<Robot> n_robots) : player(p), robots(n_robots) {}

  const std::vector<Robot> &getRobots() const { return robots; }

  void addRobot(const Robot &robot) {
    robot.setPlayer(player);
    robots.push_back(robot);
  }
};

class Board {
  static constexpr int MIN_AREA_TO_MARK = 30; // TODO: set correct value

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

  Team &GetTeam(Player p) {
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
  float evaluate() const;

  Board virtualStep(float time);
  std::vector<Robot> canGetPass() const;
  float teamActionsTime(const std::vector<class Action> &) const;
  std::vector<Robot> getRobots2Move() const;

  Player playerWithBall() const;
  Robot getRobotWithBall() const;
  Robot getRobotWithVirtualBall(const Ball &) const;

  float timeToBall(const Robot &robot) const;
  float timeToVirtualBall(const Robot &robot, const Ball &ball) const;
  Board applyTeamAction(const TeamAction &) const;

  static float fieldWidth() { return 8.090; }
  static float fieldHeight() { return 6.050; }
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
  char *text;
  std::mutex text_mutex;

  App() : text(new char[1024]) { text[0] = '\0'; }
  ~App() { delete text; }

  static void run(std::function<void(App &)>);
};

#endif
