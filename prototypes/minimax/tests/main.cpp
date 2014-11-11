#include <armadillo>
#include <iostream>
#include "base.h"
#include "body.h"
#include "minimax.h"

void print(const Robot& r){
  std::cout << "robot:\n"
      << "id: " << r.getId() << std::endl
      << "pos: " << r.pos()  << std::endl;
}

void print(const Ball &b){
  std::cout << "Ball: " << std::endl
            << b.v() << std::endl
            << b.pos() << std::endl;
}

void print(const Board& board, const Ball &b1, const Ball &b2){
  for(auto &_r: board.getRobotsMoving()){
    print(*_r);
      std::cout << "time to ball: " << board.timeToBall(*_r) << std::endl;
      std::cout << "time to ball 1: " << board.timeToVirtualBall(*_r, b1) << std::endl;
      std::cout << "time to ball 2: " << board.timeToVirtualBall(*_r, b2) << std::endl;
  }
}

void print(const Board& board){
  std::cout << "Team with ball: ";
      if( board.getRobotWithBall().second == MIN) std::cout << " min" << std::endl;
      else std::cout << " max" << std::endl;

  std::cout << "CanGetPass list: ";

  std::cout << "min:";
  for(auto& r: board.canGetPass(MIN)) print(*r);

  std::cout << "max:";
  for(auto& r: board.canGetPass(MAX)) print(*r);

}


int main(void){
  Vector v = Vector::getURand();
  Vector null;
  Team min, max;

  for(int i=1; i<5 ;i++)
    min.addRobot(
      Robot(i, Vector::getURand(), Vector::getNRand(v))
    );

  for(int i=1; i<3 ;i++)
    max.addRobot(
      Robot(i, Vector::getURand(), Vector::getNRand(v))
    );

  Ball ball_1, ball_2;
  ball_1.setV(Vector(arma::vec("2 0")));
  ball_2.setV(Vector(arma::vec("-2 2")));

  Board board(min,max);
  print(board.getBall());

  print(board, ball_1, ball_2);

  auto _r = board.getRobotWithBall().first;
  print(*_r);

  _r = board.getRobotWithVirtualBall(ball_1).first;
  print(*_r);

  _r = board.getRobotWithVirtualBall(ball_2).first;
  print(*_r);

  print(board);

  // TODO[bramigk]: testar esta função, se possível com googletest
  //board.kickLineCrossRobot();
  return 0;
}
