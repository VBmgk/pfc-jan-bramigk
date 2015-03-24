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

  // Creating random min
  for(int i=1; i<5 ;i++)
    min.addRobot(
      Robot(i, Vector::getURand(), Vector::getNRand(v))
    );

  // Creating random max
  for(int i=1; i<3 ;i++)
    max.addRobot(
      Robot(i, Vector::getURand(), Vector::getNRand(v))
    );

  // Creating different balls
  Ball ball_1, ball_2;
  ball_1.setV(Vector(2, 0));
  ball_2.setV(Vector(-2, 2));

  Board board(min,max);
  // boards ball position
  print(board.getBall());

  print(board, ball_1, ball_2);

  auto _r = board.getRobotWithBall().first;
  print(*_r);

  _r = board.getRobotWithVirtualBall(ball_1).first;
  print(*_r);

  _r = board.getRobotWithVirtualBall(ball_2).first;
  print(*_r);

  print(board);

  // TODO[bramigk]: testar com googletest
  // kickLineCrossRobot test
  Robot r_kicker(0, Vector(0, 0), Vector(0, 0));
  Robot r_blocker(1, Vector(1.5, 0), Vector(0, 0));

  Ball b(Vector(0.5, 0), Vector(0, 0));

  Team min2, max2;
  min2.addRobot(r_kicker);
  max2.addRobot(r_kicker);
  Board board2(min2,max2,b);

  print(*board2.getRobotWithBall().first);

  //std::cout << "kicklinecrossRobot: ";
  //if( board2.kickLineCrossRobot(150, r_blocker)) std::cout << "Ok" << std::endl;
  //else std::cout << "nao ok" << std::endl;

  // lineSegmentCrossCircle test
  if (Vector::lineSegmentCrossCircle(
          Vector(0, 0), Vector(2, 0),
          Vector(1, 0), Robot::radius()))
    std::cout << "basic funcion ok" << std::endl;
  else
    std::cout << "basic funcion errada" << std::endl;
  return 0;
}
