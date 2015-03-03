#ifndef MINIMAX_DRAW_H
#define MINIMAX_DRAW_H

class Robot;
class Ball;
class Board;
class App;
typedef uint8_t GLubyte;

void init_graphics();
void display(int width, int height, float zoom);
void draw_circle(float radius);
void draw_robot(const Robot &robot, const GLubyte *color);
void draw_ball(const Ball &ball);
void draw_board(const Board &board);
void draw_teamaction(const TeamAction &t_action, const Board &board,
                     Player player);
void draw_app(App *, double fps, int width, int height);

#endif
