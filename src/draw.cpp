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
#include "decision_table.h"
#include "suggestion_table.h"
#include "action.h"
#include "segment.h"
#include "array.h"
#include "app.h"

constexpr int NSIDES = 64;

bool DRAW_DECISON = true;
bool DRAW_GAP = true;
bool DRAW_POSSIBLE_RECEIVERS = true;
bool DRAW_BALL_OWNER = true;
int DRAW_BALL_SHADOW =
    1; // 0: none, 1: time to ball, 2: time when kick to selected robot
bool DRAW_SELECTED_ROBOT = true;

void draw_options_window(void) {
  if (!ImGui::Begin("Draw options")) {
    ImGui::End();
    return;
  }

  ImGui::Checkbox("draw decisions", &DRAW_DECISON);
  ImGui::Checkbox("draw gap", &DRAW_GAP);
  ImGui::RadioButton("no time to ball", &DRAW_BALL_SHADOW, 0);
  ImGui::RadioButton("draw time to ball", &DRAW_BALL_SHADOW, 1);
  ImGui::RadioButton("draw time to ball after pass", &DRAW_BALL_SHADOW, 2);
  ImGui::Checkbox("draw possible receivers", &DRAW_POSSIBLE_RECEIVERS);
  ImGui::Checkbox("draw ball owner", &DRAW_BALL_OWNER);
  ImGui::Checkbox("draw selected robot", &DRAW_SELECTED_ROBOT);

  ImGui::End();
}

void screen_zoom(int width, int height, double zoom, double center_x,
                 double center_y) {
  float ratio = width / (float)height;
  glViewport(0, 0, width, height);

  glClearColor(((float)DARK_GREEN[0]) / 255.0, ((float)DARK_GREEN[1]) / 255.0,
               ((float)DARK_GREEN[2]) / 255.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  // glPushMatrix();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float r = 1.0 / zoom;
  float ox = -center_x / width * 2;
  float oy = center_y / height * 2;
  glOrtho((ox - 1.0) * ratio * r, (ox + 1.0) * ratio * r, (oy - 1.0) * r,
          (oy + 1.0) * r, 1.0, -1.0);
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

void draw_robot(Vector pos, const GLubyte *color, float extra_radius = 0.0) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, 0.0);
  glColor3ubv(color);
  draw_circle(ROBOT_RADIUS + extra_radius);
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

void draw_shadow(const State &state) {
  int gaps_count;
  Segment gaps[N_ROBOTS * 2];

  int robot = robot_with_ball(state);
  discover_gaps_from_pos(state, state.ball, ENEMY_OF(robot), gaps, &gaps_count,
                         robot);
  float gx = GOAL_X(ENEMY_OF(robot));

  auto b = state.ball;

  FOR_N(i, gaps_count) {
    glColor3ubv(LIGHT_GREEN);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(b.x, b.y, 0.0);
    glVertex3f(gx, gaps[i].u, 0.0);
    glVertex3f(gx, gaps[i].d, 0.0);
    glEnd();
    // glColor3ubv(WHITE);
    glColor3ubv(PINK);
    glBegin(GL_LINES);
    glVertex3f(gx, gaps[i].u, 0.0);
    glVertex3f(gx, gaps[i].d, 0.0);
    glEnd();
  }
}
#endif

void draw_field(void) {
  glColor3ubv(FIELD_GREEN);
  glRectf(-FIELD_WIDTH / 2 - BOUNDARY_WIDTH - REFEREE_WIDTH,
          -FIELD_HEIGHT / 2 - BOUNDARY_WIDTH - REFEREE_WIDTH,
          FIELD_WIDTH / 2 + BOUNDARY_WIDTH + REFEREE_WIDTH,
          FIELD_HEIGHT / 2 + BOUNDARY_WIDTH + REFEREE_WIDTH);

  glColor3ubv(WHITE);

  // mid line
  glRectf(-LINE_WIDTH / 2, -FIELD_HEIGHT / 2, LINE_WIDTH / 2, FIELD_HEIGHT / 2);

  // BACK LINES
  glRectf(-FIELD_WIDTH / 2, -FIELD_HEIGHT / 2, -FIELD_WIDTH / 2 + LINE_WIDTH,
          FIELD_HEIGHT / 2);
  glRectf(FIELD_WIDTH / 2, -FIELD_HEIGHT / 2, FIELD_WIDTH / 2 - LINE_WIDTH,
          FIELD_HEIGHT / 2);

  // SIDE LINES
  glRectf(-FIELD_WIDTH / 2, -FIELD_HEIGHT / 2, FIELD_WIDTH / 2,
          -FIELD_HEIGHT / 2 + LINE_WIDTH);
  glRectf(-FIELD_WIDTH / 2, FIELD_HEIGHT / 2, FIELD_WIDTH / 2,
          FIELD_HEIGHT / 2 - LINE_WIDTH);

  // center dot
  constexpr int PSIDES{20};
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < PSIDES; i++) {
    float a = RADIANS(i * 360.0 / PSIDES);
    float r = 1.5 * LINE_WIDTH;
    glVertex2f(r * cos(a), r * sin(a));
  }
  glEnd();

  // center circle
  constexpr int CSIDES{60};
  glBegin(GL_TRIANGLE_STRIP);
  for (int i = 0; i <= CSIDES; i++) {
    float a = RADIANS(i * 360.0 / CSIDES);
    float r1 = CENTER_CIRCLE_RADIUS - LINE_WIDTH;
    float r2 = CENTER_CIRCLE_RADIUS;
    glVertex2f(r1 * cos(a), r1 * sin(a));
    glVertex2f(r2 * cos(a), r2 * sin(a));
  }
  glEnd();

  // defense areas
  constexpr int DSIDES{15};
  glBegin(GL_TRIANGLE_STRIP);
  for (int i = -DSIDES; i <= 0; i++) {
    float a = RADIANS(i * 90.0 / DSIDES);
    float r1 = DEFENSE_RADIUS - LINE_WIDTH;
    float r2 = DEFENSE_RADIUS;
    float yoff = -DEFENSE_STRETCH / 2;
    glVertex2f(-FIELD_WIDTH / 2 + r1 * cos(a), yoff + r1 * sin(a));
    glVertex2f(-FIELD_WIDTH / 2 + r2 * cos(a), yoff + r2 * sin(a));
  }
  for (int i = 0; i <= DSIDES; i++) {
    float a = RADIANS(i * 90.0 / DSIDES);
    float r1 = DEFENSE_RADIUS - LINE_WIDTH;
    float r2 = DEFENSE_RADIUS;
    float yoff = DEFENSE_STRETCH / 2;
    glVertex2f(-FIELD_WIDTH / 2 + r1 * cos(a), yoff + r1 * sin(a));
    glVertex2f(-FIELD_WIDTH / 2 + r2 * cos(a), yoff + r2 * sin(a));
  }
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  for (int i = -DSIDES; i <= 0; i++) {
    float a = RADIANS(i * 90.0 / DSIDES);
    float r1 = DEFENSE_RADIUS - LINE_WIDTH;
    float r2 = DEFENSE_RADIUS;
    float yoff = -DEFENSE_STRETCH / 2;
    glVertex2f(FIELD_WIDTH / 2 - r1 * cos(a), yoff + r1 * sin(a));
    glVertex2f(FIELD_WIDTH / 2 - r2 * cos(a), yoff + r2 * sin(a));
  }
  for (int i = 0; i <= DSIDES; i++) {
    float a = RADIANS(i * 90.0 / DSIDES);
    float r1 = DEFENSE_RADIUS - LINE_WIDTH;
    float r2 = DEFENSE_RADIUS;
    float yoff = DEFENSE_STRETCH / 2;
    glVertex2f(FIELD_WIDTH / 2 - r1 * cos(a), yoff + r1 * sin(a));
    glVertex2f(FIELD_WIDTH / 2 - r2 * cos(a), yoff + r2 * sin(a));
  }
  glEnd();

  // penalty dots
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < PSIDES; i++) {
    float a = RADIANS(i * 360.0 / PSIDES);
    float r = 1.5 * LINE_WIDTH;
    float x = -FIELD_WIDTH / 2 + DEFENSE_RADIUS - LINE_WIDTH / 2;
    glVertex2f(x + r * cos(a), r * sin(a));
  }
  glEnd();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < PSIDES; i++) {
    float a = RADIANS(i * 360.0 / PSIDES);
    float r = 1.5 * LINE_WIDTH;
    float x = FIELD_WIDTH / 2 - DEFENSE_RADIUS + LINE_WIDTH / 2;
    glVertex2f(x + r * cos(a), r * sin(a));
  }
  glEnd();

  // the goals
  glColor3ubv(GREY);
  glRectf(-FIELD_WIDTH / 2 - GOAL_DEPTH - GOAL_WALL_WIDTH,
          -GOAL_WIDTH / 2 - GOAL_WALL_WIDTH, -FIELD_WIDTH / 2, -GOAL_WIDTH / 2);
  glRectf(-FIELD_WIDTH / 2 - GOAL_DEPTH - GOAL_WALL_WIDTH,
          GOAL_WIDTH / 2 + GOAL_WALL_WIDTH, -FIELD_WIDTH / 2, GOAL_WIDTH / 2);
  glRectf(-FIELD_WIDTH / 2 - GOAL_DEPTH - GOAL_WALL_WIDTH,
          -GOAL_WIDTH / 2 - GOAL_WALL_WIDTH, -FIELD_WIDTH / 2 - GOAL_DEPTH,
          GOAL_WIDTH / 2 + GOAL_WALL_WIDTH);
  glRectf(FIELD_WIDTH / 2 + GOAL_DEPTH + GOAL_WALL_WIDTH,
          -GOAL_WIDTH / 2 - GOAL_WALL_WIDTH, FIELD_WIDTH / 2, -GOAL_WIDTH / 2);
  glRectf(FIELD_WIDTH / 2 + GOAL_DEPTH + GOAL_WALL_WIDTH,
          GOAL_WIDTH / 2 + GOAL_WALL_WIDTH, FIELD_WIDTH / 2, GOAL_WIDTH / 2);
  glRectf(FIELD_WIDTH / 2 + GOAL_DEPTH + GOAL_WALL_WIDTH,
          -GOAL_WIDTH / 2 - GOAL_WALL_WIDTH, FIELD_WIDTH / 2 + GOAL_DEPTH,
          GOAL_WIDTH / 2 + GOAL_WALL_WIDTH);
}

void draw_ball(Vector pos) {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, 0.0);
  glColor3ubv(ORANGE);
  draw_circle(BALL_RADIUS);
  glPopMatrix();
}

void draw_goals() {
  glColor3ubv(BLUE);
  glRectf(-FIELD_WIDTH / 2 - GOAL_DEPTH, GOAL_WIDTH / 2, -FIELD_WIDTH / 2,
          -GOAL_WIDTH / 2);

  glColor3ubv(YELLOW);
  glRectf(FIELD_WIDTH / 2 + GOAL_DEPTH, GOAL_WIDTH / 2, FIELD_WIDTH / 2,
          -GOAL_WIDTH / 2);
}

void draw_state(const State &state) {
  // glColor3ubv(FIELD_GREEN);
  // glRectf(-FIELD_WIDTH / 2, FIELD_HEIGHT / 2, FIELD_WIDTH / 2,
  // -FIELD_HEIGHT
  // / 2);
  draw_field();

  draw_goals();
  if (DRAW_GAP) {
    draw_shadow(state);
  }

  if (DRAW_BALL_SHADOW == 1) {
    constexpr float I = 0.10;
    constexpr float S = I / 2;
    glPushMatrix();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (float x = -FIELD_WIDTH / 2 + std::fmod(FIELD_WIDTH, I) / 2;
         x < S + FIELD_WIDTH / 2; x += I) {
      for (float y = -FIELD_HEIGHT / 2 + std::fmod(FIELD_HEIGHT, I) / 2;
           y < S + FIELD_HEIGHT / 2; y += I) {
        float t = time_to_pos({x, y}, {}, state.ball, state.ball_v);
        float c = (1 - std::sqrt(t)) / 2 - 0.1;
        glColor4f(1, 0, 1,
                  c); // this works ok because t=1 means 1 second, which is ok
        // glRectf(x, y, x + S, y + S);
        glPushMatrix();
        glTranslatef(x, y, 0.0);
        glBegin(GL_TRIANGLE_FAN);
        raw_circle(S / 2, 8);
        glEnd();
        glPopMatrix();
      }
    }
    glPopMatrix();
  }

  if (DRAW_BALL_SHADOW == 2 && *app_selected_robot != -1) {
    constexpr float I = 0.10;
    constexpr float S = I / 2;
    auto target = app_decision_table->move[*app_selected_robot].move_pos;
    auto kick_v = unit(target - state.ball) * ROBOT_KICK_SPEED;
    glPushMatrix();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (float x = -FIELD_WIDTH / 2 + std::fmod(FIELD_WIDTH, I) / 2;
         x < S + FIELD_WIDTH / 2; x += I) {
      for (float y = -FIELD_HEIGHT / 2 + std::fmod(FIELD_HEIGHT, I) / 2;
           y < S + FIELD_HEIGHT / 2; y += I) {
        float t = time_to_pos({x, y}, {}, state.ball, kick_v, ROBOT_MAX_SPEED);
        float c = (1 - std::sqrt(t)) / 2 - 0.1;
        glColor4f(1, 0, 1,
                  c); // this works ok because t=1 means 1 second, which is ok
        // glRectf(x, y, x + S, y + S);
        glPushMatrix();
        glTranslatef(x, y, 0.0);
        glBegin(GL_TRIANGLE_FAN);
        raw_circle(S / 2, 8);
        glEnd();
        glPopMatrix();
      }
    }
    glPopMatrix();
  }

  if (DRAW_POSSIBLE_RECEIVERS) {
    // shadow for possible receivers (from owner)
    // DRAW_RECEIVERS(MIN);
    // DRAW_RECEIVERS(MAX);
    TeamFilter receivers;
    int rwb = robot_with_ball(state);
    discover_possible_receivers(state, app_decision_table, MAX, receivers,
                                PLAYER_OF(rwb) == MAX ? rwb : -1);
    FOR_TEAM_ROBOT_IN(i, MAX, receivers) {
      draw_robot(state.robots[i], PINK2, 2 * BALL_RADIUS);
    }
  }

  if (DRAW_BALL_OWNER) {
    int rwb = robot_with_ball(state);
    // shadow for ball owner
    draw_robot(state.robots[rwb], PINK, 2 * BALL_RADIUS);
  }

  // DRAW ALL THE ROBOTS!!!!
  FOR_EVERY_ROBOT(i) {
    draw_robot(state.robots[i], PLAYER_OF(i) == MAX ? BLUE : YELLOW);
  }

  if (DRAW_SELECTED_ROBOT) {
    // shadow for selected robot
    draw_robot(state.robots[*app_selected_robot], RED, -ROBOT_RADIUS / 2);
  }

  // and the ball
  draw_ball(state.ball);
}

void draw_decision(const struct Decision &decision, const struct State &state,
                   Player player) {
  FOR_TEAM_ROBOT(i, player) {
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

void draw_suggestion(const struct SuggestionTable &table) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4ub(PINK[0], PINK[1], PINK[2], 192);
  FOR_N(i, table.spots_count) {
    glPushMatrix();
    auto spot = table.spots[i];
    glTranslatef(spot.x, spot.y, 0.f);
    draw_circle(ROBOT_RADIUS);
    glPopMatrix();
  }
  glDisable(GL_BLEND);
}

#if 0
void draw_app(App *app, double fps, int width, int height) {
  draw_board(app->command_board);
  draw_teamaction(app->command, app->command_board, MAX);
  draw_teamaction(app->enemy_command, app->command_board, MIN);
}
#endif

// void draw_app_status(void) is implemented on app.cpp
