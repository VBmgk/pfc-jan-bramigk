#define MINIMAX_DRAW_H
#ifdef MINIMAX_DRAW_H

class Robot;
class Ball;
class Board;
typedef uint8_t GLubyte;

void clear_for_drawing(int width, int height, float zoom);
void draw_circle(float radius);
void draw_robot(const Robot& robot, const GLubyte* color);
void draw_ball(const Ball& ball);
void draw_board(const Board& board);

#endif
