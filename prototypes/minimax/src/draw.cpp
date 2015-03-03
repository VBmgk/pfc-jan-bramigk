// think different
#ifdef __APPLE__
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
//#include <GL/glu.h>
#endif

#include <cmath>
#include <imgui.h>

#include "app.h"
#include "draw.h"

static const GLubyte BLACK[3] = {3, 3, 3};
static const GLubyte GREY[3] = {20, 20, 20};
static const GLubyte BLUE[3] = {50, 118, 177};
static const GLubyte BLUE2[3] = {40, 94, 142};
static const GLubyte DARK_GREEN[3] = {12, 60, 8};
static const GLubyte LIGHT_GREEN[3] = {37, 179, 23};
static const GLubyte FIELD_GREEN[3] = {25, 119, 15};
static const GLubyte ORANGE[3] = {255, 147, 31};
static const GLubyte ORANGE2[3] = {197, 122, 41};
static const GLubyte WHITE[3] = {239, 239, 239};
static const GLubyte YELLOW[3] = {237, 229, 40};
static const GLubyte RED[3] = {177, 84, 84};
static const GLubyte RED2[3] = {142, 67, 67};
static const int NSIDES = 64;

void raw_circle(float radius, int sides = NSIDES) {
  glVertex2f(0.0, 0.0);
  for (int i = 0; i <= sides; i++) {
    auto s = sin((2.0 * M_PI * i) / sides) * radius;
    auto c = cos((2.0 * M_PI * i) / sides) * radius;
    glVertex2f(s, c);
  }
}

void draw_circle(float radius) {
  glBegin(GL_TRIANGLE_FAN);
  raw_circle(radius);
  glEnd();
}

void draw_robot(const Robot &robot, const GLubyte *color) {
  glPushMatrix();
  auto pos = robot.pos();
  glTranslatef(pos[0], pos[1], 0.f);
  glColor3ubv(color);
  draw_circle(robot.radius());
  glPopMatrix();
}

void draw_shadow(const Board &board) {
  auto &ball = board.getBall();
  auto player_with_ball = board.getRobotWithBall().second;
  auto gx = board.enemyGoalPos(player_with_ball)[0];
  for (auto gap : board.getGoalGaps()) {
    glColor3ubv(LIGHT_GREEN);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glVertex3f(gx, gap.first, 0.0f);
    glVertex3f(gx, gap.second, 0.0f);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glEnd();
    glColor3ubv(WHITE);
    glBegin(GL_LINES);
    glVertex3f(gx, gap.first, 0.0f);
    glVertex3f(gx, gap.second, 0.0f);
    glEnd();
  }
}

void draw_ball(const Ball &ball) {
  glPushMatrix();
  auto pos = ball.pos();
  glTranslatef(pos[0], pos[1], 0.f);
  glColor3ubv(ORANGE);
  draw_circle(ball.radius());
  glPopMatrix();
}

void draw_goals(const Board &board) {
  if (board.isMaxLeft())
    glColor3ubv(BLUE);
  else
    glColor3ubv(YELLOW);

  glRectf(-board.goalX() - board.goalDepth(), board.goalWidth() / 2,
          -board.goalX(), -board.goalWidth() / 2);

  if (board.isMaxLeft())
    glColor3ubv(YELLOW);
  else
    glColor3ubv(BLUE);

  glRectf(board.goalX() + board.goalDepth(), board.goalWidth() / 2,
          board.goalX(), -board.goalWidth() / 2);
}

void draw_board(const Board &board) {
  glColor3ubv(FIELD_GREEN);
  glRectf(-board.fieldWidth() / 2, board.fieldHeight() / 2,
          board.fieldWidth() / 2, -board.fieldHeight() / 2);

  draw_goals(board);
  draw_shadow(board);

  // XXX: TEST
  auto with_ball = board.getRobotWithBall();
  auto robot_with_ball = with_ball.first;
  if (robot_with_ball != nullptr && false) {
    float step_time = board.timeToBall(*robot_with_ball);
    Board vrt_board = board.virtualStep(step_time);
    Ball vrt_ball = vrt_board.getBall();
    // vrt_ball.setV(Vector{5, 0});
    vrt_ball.setV((vrt_ball.pos() - Vector{0, 0}).unit() * Robot::kickV());

    float maxV2 = 4 * 4;
    float inc = 0.05;
    for (float x = -board.fieldWidth() / 2; x < inc + board.fieldWidth() / 2;
         x += inc) {
      for (float y = -board.fieldHeight() / 2;
           y < inc + board.fieldHeight() / 2; y += inc) {
        float t = board.timeToVirtualBall(Vector{x, y}, maxV2, vrt_ball);
        glColor3f(1 - t, 0, 0);
        // glTranslatef(x, y, 0.f);
        // glBegin(GL_TRIANGLE_FAN);
        glRectf(x - inc / 3, y - inc / 3, x + inc / 3, y + inc / 3);
        // raw_circle(0.1, 6);
        // glEnd();
      }
    }
  }

  for (auto robot : board.getMax().getRobots()) {
    draw_robot(robot, BLUE);
  }

  for (auto robot : board.getMin().getRobots()) {
    draw_robot(robot, YELLOW);
  }

  draw_ball(board.getBall());
}

void init_graphics() {}

void display(int width, int height, float zoom) {
  float ratio = width / (float)height;
  glViewport(0, 0, width, height);

  glClearColor(((float)DARK_GREEN[0]) / 255.0, ((float)DARK_GREEN[1]) / 255.0,
               ((float)DARK_GREEN[2]) / 255.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  // glPushMatrix();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float r = 1.0 / zoom;
  glOrtho(-ratio * r, ratio * r, -1.0 * r, 1.0 * r, 1.0, -1.0);
}

const Robot *get_robot(int id, const Board &board, Player player) {
  for (auto &robot : board.getTeam(player).getRobots()) {
    if (robot.getId() == id) {
      return &robot;
    }
  }
  return nullptr;
}

void draw_teamaction(const TeamAction &t_action, const Board &board,
                     Player player) {
  for (auto action : t_action) {
    auto robot = get_robot(action.robot_id, board, player);
    if (robot == nullptr)
      continue;
    switch (action.type) {
    case MOVE: {
      glColor3ubv(BLACK);
      glBegin(GL_LINES);
      auto pos_i = robot->pos();
      glVertex3f(pos_i[0], pos_i[1], 0.0f);
      auto pos_f = action.move_point;
      glVertex3f(pos_f[0], pos_f[1], 0.0f);
      glEnd();
      glPushMatrix();
      glTranslatef(pos_f[0], pos_f[1], 0.f);
      draw_circle(robot->radius() / 5);
      glPopMatrix();
    } break;
    case PASS: {
      auto rcv = get_robot(action.pass_id, board, player);
      if (rcv == nullptr)
        continue;
      glColor3ubv(GREY);
      // glColor3ubv(RED2);
      glBegin(GL_LINES);
      auto pos_i = robot->pos();
      glVertex3f(pos_i[0], pos_i[1], 0.0f);
      auto pos_b = board.getBall().pos();
      glVertex3f(pos_b[0], pos_b[1], 0.0f);
      glEnd();
      glPushAttrib(GL_ENABLE_BIT);
      // glLineStipple(1, 0xAAAA);
      glLineStipple(1, 0xF0F0);
      glEnable(GL_LINE_STIPPLE);
      glBegin(GL_LINES);
      glVertex3f(pos_b[0], pos_b[1], 0.0f);
      auto pos_f = rcv->pos();
      glVertex3f(pos_f[0], pos_f[1], 0.0f);
      glEnd();
      glPopAttrib();
    } break;
    case KICK: {
      glColor3ubv(RED);
      glBegin(GL_LINES);
      auto pos_i = robot->pos();
      glVertex3f(pos_i[0], pos_i[1], 0.0f);
      auto pos_b = board.getBall().pos();
      glVertex3f(pos_b[0], pos_b[1], 0.0f);
      auto pos_f = action.kick_point;
      glVertex3f(pos_f[0], pos_f[1], 0.0f);
      glEnd();
      glPushMatrix();
      glTranslatef(pos_f[0], pos_f[1], 0.f);
      draw_circle(robot->radius() / 5);
      glPopMatrix();
    } break;
    }
  }
}

void draw_app(App *app, double fps, int width, int height) {
  draw_board(app->command_board);
  draw_teamaction(app->command, app->command_board, MAX);
  draw_teamaction(app->enemy_command, app->command_board, MIN);
}
