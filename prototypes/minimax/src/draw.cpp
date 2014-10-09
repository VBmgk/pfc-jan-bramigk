#ifdef __APPLE__
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
//#include <GL/glu.h>
#endif

#include "minimax.h"
#include "draw.h"

#include <cmath>

static const GLubyte BLACK[3]       = {3, 3, 3};
static const GLubyte BLUE[3]        = {50, 118, 177};
static const GLubyte BLUE2[3]       = {40, 94, 142};
static const GLubyte DARK_GREEN[3]  = {12, 60, 8};
static const GLubyte FIELD_GREEN[3] = {25, 119, 15};
static const GLubyte ORANGE[3]      = {255, 147, 31};
static const GLubyte ORANGE2[3]     = {197, 122, 41};
static const GLubyte WHITE[3]       = {239, 239, 239};
static const GLubyte YELLOW[3]      = {237, 229, 40};
static const int NSIDES = 64;

void draw_circle(float radius) {
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0.0, 0.0);
  for(int i = 0; i <= NSIDES; i++) {
    auto s = sin((2.0 * M_PI * i) / NSIDES) * radius;
    auto c = cos((2.0 * M_PI * i) / NSIDES) * radius;
    glVertex2f(s, c);
  }
  glEnd();
}

void draw_robot(const Robot& robot, const GLubyte* color) {
  glPushMatrix();
  auto pos = robot.pos();
  glTranslatef(pos[0], pos[1], 0.f);
  glColor3ubv(color);
  draw_circle(robot.radius());
  glPopMatrix();
}

void draw_ball(const Ball& ball) {
  glPushMatrix();
  auto pos = ball.pos();
  glTranslatef(pos[0], pos[1], 0.f);
  glColor3ubv(ORANGE);
  draw_circle(ball.radius());
  glPopMatrix();
}

void draw_board(const Board& board) {
  glColor3ubv(FIELD_GREEN);
  glRectf(-board.fieldWidth() / 2,  board.fieldHeight() / 2,
           board.fieldWidth() / 2, -board.fieldHeight() / 2);

  for (auto robot : board.getMax().getRobots()) {
    draw_robot(robot, BLUE);
  }

  for (auto robot : board.getMin().getRobots()) {
    draw_robot(robot, YELLOW);
  }

  draw_ball(board.getBall());
}

void clear_for_drawing(int width, int height, float zoom) {
  float ratio = width / (float) height;
  glViewport(0, 0, width, height);

  glClearColor(
    ((float) DARK_GREEN[0]) / 255.0,
    ((float) DARK_GREEN[1]) / 255.0,
    ((float) DARK_GREEN[2]) / 255.0,
    1.0
  );
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float r = 1.0 / zoom;
  glOrtho(
     -ratio * r,
      ratio * r,
     -1.0   * r,
      1.0   * r,
      1.0,
     -1.0
  );
  //glMatrixMode(GL_MODELVIEW);
}
