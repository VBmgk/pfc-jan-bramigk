#include <vector>
#include <armadillo>
#include "action.h"
#include "pass.h"
#include "kick.h"
#include "board.h"
#include "body.h"
#include "ball.h"
#include "robot.h"
#include "move.h"
#include "team.h"

using namespace std;

bool Board::isGameOver(){
  if(openGoalArea() > MIN_AREA_TO_MARK)
    return true;

  return false;
}

Player Board::currentPlayer(){
  return player;
}

vector<Action> Board::getActions(){
  vector<Action> actions;

  if(playerWithBall() == player){
    // Pass
    Robot robotWithBall = getRobotWithBall();

    for(auto robot: canGetPass()){
      Pass *pass = new Pass(robotWithBall, robot);
      actions.push_back(*pass);
    }

    // Kick
    actions.push_back(*new Kick(robotWithBall));
  }

  // Move
  for(auto robot: getRobots2Move()){
    actions.push_back(*new Move(robot));
  }

  return actions;
}

vector<vector<Action> > Board::getRobotsActions(){
  vector<vector<Action> > robotsActions;

  for(int i=0 ; i<RAMIFICATION_NUMBER ; i++){
    robotsActions.push_back(getActions());
  }

  return robotsActions;
}

Robot Board::getRobotWithBall(){
  // TODO: don't concatenate vectors
  vector<Robots> robots;

  // preallocate memory
  robots.reserve(min.getRobots()->size() + max.getRobots->size());
  robots.insert(robots.end(), min.getRobots()->begin(), min.getRobots()->end());
  robots.insert(robots.end(), max.getRobots()->begin(), max.getRobots()->end());

  float min_time = FLT_MAX;
  Robot *robotWithBall = nullptr;

  float time;
  for(auto& robot: robots){
    time = getTimeToBall(robot);

    if(time < min_time){
      robotWithBall = &robot;
      min_time = time;
    }
  }

  return *robotWithBall;
}

float Board::getTimeToBall(const Robot& robot){
  // TODO
  // vb.t + pb = vr.t + pr, t_min? vr?
  // => 0 = (vr^2 - vb^2)t^2 - |pr - pb|^2 - 2.vb.(pb - pr).t
  // => delta = 4.[ (vb.(pb - pr))^2 + (vr^2 - vb^2).|pr - pb|^2]
  // t = -b +- sqrt(delta)
  //    -----------------
  //           2.a
}

float Board::getTimeToVirtualBall(const Robot& robot, const Ball ball){
  // TODO
  // vb.t + pb = vr.t + pr, t_min? vr?
  // => 0 = (vr^2 - vb^2)t^2 - |pr - pb|^2 - 2.vb.(pb - pr).t
  // => delta = 4.[ (vb.(pb - pr))^2 + (vr^2 - vb^2).|pr - pb|^2]
  // t = -b +- sqrt(delta)
  //    -----------------
  //           2.a
}

Player Board::playerWithBall(){
  return getRobotWithBall().getPlayer();
}

vector<Robot> Board::canGetPass(){
  // TODO
  // create ball with after kick
  // float Board::getTimeToBall(const Robot& robot, const Ball ball){
}

float Board::openGoalArea(){
  // TODO
}

float Board::evaluate(){
  // TODO
}

Board Board::applyRobotsActions(const vector<class Action>& actions){
  // TODO
}

float Board::getRobotsActionsTime(const vector<class Action>& actions){
  // TODO: get maximum time
}

vector<class Robot> Board::getRobots2Move(){
  // TODO
}
