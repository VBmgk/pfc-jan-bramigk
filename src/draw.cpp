// think different ( ¬ ¬)
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <cmath>
#include <imgui.h>

#include "draw.h"
#include "colors.h"
#include "vector.h"
#include "state.h"
#include "utils.h"
#include "decision.h"
#include "action.h"

constexpr int NSIDES = 64;

void screen_zoom(int width, int height, float zoom) {
  float ratio = width / (float)height;
  glViewport(0, 0, width, height);

  glClearColor(((float)DARK_GREEN[0]) / 255.0, ((float)DARK_GREEN[1]) / 255.0, ((float)DARK_GREEN[2]) / 255.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  // glPushMatrix();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float r = 1.0 / zoom;
  glOrtho(-ratio * r, ratio * r, -1.0 * r, 1.0 * r, 1.0, -1.0);
}

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

void draw_robot(Vector pos, const GLubyte *color) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, 0.0);
  glColor3ubv(color);
  draw_circle(ROBOT_RADIUS);
  glPopMatrix();
}

#if 0
void draw_shadow(const Board &board) {
  auto &ball = board.getBall();
  auto player_with_ball = board.getRobotWithBall().second;
  auto gx = board.enemyGoalPos(player_with_ball)[0];
  for (auto gap : board.getGoalGaps()) {
    glColor3ubv(LIGHT_GREEN);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0);
    glVertex3f(gx, gap.first, 0.0);
    glVertex3f(gx, gap.second, 0.0);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0);
    glEnd();
    glColor3ubv(WHITE);
    glBegin(GL_LINES);
    glVertex3f(gx, gap.first, 0.0);
    glVertex3f(gx, gap.second, 0.0);
    glEnd();
  }
}
#else
void draw_shadow(const State &state) {}
#endif

void draw_ball(Vector pos) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, 0.0);
  glColor3ubv(ORANGE);
  draw_circle(BALL_RADIUS);
  glPopMatrix();
}

#if 0
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
#else
void draw_goals() {
  glColor3ubv(BLUE);
  glRectf(-FIELD_WIDTH / 2 - GOAL_DEPTH, GOAL_WIDTH / 2, -FIELD_WIDTH / 2, -GOAL_WIDTH / 2);

  glColor3ubv(YELLOW);
  glRectf(FIELD_WIDTH / 2 + GOAL_DEPTH, GOAL_WIDTH / 2, FIELD_WIDTH / 2, -GOAL_WIDTH / 2);
}
#endif

void draw_state(const State &state) {
  glColor3ubv(FIELD_GREEN);
  glRectf(-FIELD_WIDTH / 2, FIELD_HEIGHT / 2, FIELD_WIDTH / 2, -FIELD_HEIGHT / 2);

  draw_goals();
  draw_shadow(state);

// XXX: TEST
#if 0
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
#endif

  FOR_EVERY_ROBOT(i) { draw_robot(state.robots[i], PLAYER_FOR(i) == MAX ? BLUE : YELLOW); }

  draw_ball(state.ball);
}

void draw_decision(const struct Decision &decision, const struct State &state, Player player) {
  FOR_EVERY_ROBOT(i) {
    auto action = decision.action[i % N_ROBOTS];
    switch (action.type) {
    case MOVE: {
      glColor3ubv(BLACK);
      glBegin(GL_LINES);
      auto pos_i = state.robots[i];
      glVertex3f(pos_i.x, pos_i.y, 0.0);
      auto pos_f = action.move_pos;
      glVertex3f(pos_f.x, pos_f.y, 0.0);
      glEnd();
      glPushMatrix();
      glTranslatef(pos_f.x, pos_f.y, 0.f);
      draw_circle(ROBOT_RADIUS / 5);
      glPopMatrix();
    } break;
    case PASS: {
      glColor3ubv(GREY);
      // glColor3ubv(RED2);
      glBegin(GL_LINES);
      auto pos_i = state.robots[i];
      glVertex3f(pos_i.x, pos_i.y, 0.0);
      auto pos_b = state.ball;
      glVertex3f(pos_b.x, pos_b.y, 0.0);
      glEnd();
      glPushAttrib(GL_ENABLE_BIT);
      // glLineStipple(1, 0xAAAA);
      glLineStipple(1, 0xF0F0);
      glEnable(GL_LINE_STIPPLE);
      glBegin(GL_LINES);
      glVertex3f(pos_b.x, pos_b.y, 0.0);
      auto pos_f = state.robots[action.pass_receiver];
      glVertex3f(pos_f.x, pos_f.y, 0.0);
      glEnd();
      glPopAttrib();
    } break;
    case KICK: {
      glColor3ubv(RED);
      glBegin(GL_LINES);
      auto pos_i = state.robots[i];
      glVertex3f(pos_i.x, pos_i.y, 0.0);
      auto pos_b = state.ball;
      glVertex3f(pos_b.x, pos_b.y, 0.0);
      auto pos_f = action.kick_pos;
      glVertex3f(pos_f.x, pos_f.y, 0.0);
      glEnd();
      glPushMatrix();
      glTranslatef(pos_f.x, pos_f.y, 0.f);
      draw_circle(ROBOT_RADIUS / 5);
      glPopMatrix();
    } break;
    default: {}
    }
  }
}

#if 0
void draw_app(App *app, double fps, int width, int height) {
  draw_board(app->command_board);
  draw_teamaction(app->command, app->command_board, MAX);
  draw_teamaction(app->enemy_command, app->command_board, MIN);
}
#endif
