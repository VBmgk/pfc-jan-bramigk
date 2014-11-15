// think different
#ifdef __APPLE__
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glut.h>
#endif

//#include <ft2build.h>
//#include FT_FREETYPE_H

#include "minimax.h"
#include "draw.h"

#include <cmath>

static const GLubyte BLACK[3] = {3, 3, 3};
static const GLubyte GREY[3] = {20, 20, 20};
static const GLubyte BLUE[3] = {50, 118, 177};
static const GLubyte BLUE2[3] = {40, 94, 142};
static const GLubyte DARK_GREEN[3] = {12, 60, 8};
static const GLubyte FIELD_GREEN[3] = {25, 119, 15};
static const GLubyte ORANGE[3] = {255, 147, 31};
static const GLubyte ORANGE2[3] = {197, 122, 41};
static const GLubyte WHITE[3] = {239, 239, 239};
static const GLubyte YELLOW[3] = {237, 229, 40};
static const GLubyte RED[3] = {177, 84, 84};
static const GLubyte RED2[3] = {142, 67, 67};
static const int NSIDES = 64;

void raw_circle(float radius) {
  glVertex2f(0.0, 0.0);
  for (int i = 0; i <= NSIDES; i++) {
    auto s = sin((2.0 * M_PI * i) / NSIDES) * radius;
    auto c = cos((2.0 * M_PI * i) / NSIDES) * radius;
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

void draw_shadow(const Board& board){
  auto& ball = board.getBall();
  auto player_with_ball =
    board.getRobotWithBall().second;

  auto gx = board.enemyGoalPos(player_with_ball)[0];

  // collect shadows

  std::vector<std::pair<float, float>> shadows;
  for (auto _robot : board.getRobotsMoving()) {
    auto& robot = *_robot;
    auto d = robot.pos() - ball.pos();
    auto k = d * d - Robot::radius() * Robot::radius();

    if (k <= 0) continue;
    if (d[0] * gx <= 0) continue;

    // XXX: it's likely there is an issue calculating tangents
    //      with near zero value, which causes, near horizontal
    //      lines to be wrong, check it out on the gui.

    float tan_alpha = Robot::radius() / std::sqrt(k);
    float tan_theta = std::fabs(d[1] / d[0]);
    printf("%.010g, %.010g\n", tan_alpha, tan_theta);
    float tan_1 = (tan_theta + tan_alpha) /
      (1 - tan_theta * tan_alpha);

    float tan_2 = (tan_theta - tan_alpha) /
      (1 + tan_theta * tan_alpha);

#define Y_SHADOW(var, tan) \
    float var = std::copysign( \
        tan * std::fabs (ball.pos()[0] - gx), \
        d[1]) + ball.pos()[1]
    Y_SHADOW(y_shadow_1, tan_1);
    Y_SHADOW(y_shadow_2, tan_2);
#undef Y_SHADOW

    float u_shadow = std::max(y_shadow_1, y_shadow_2);
    float d_shadow = std::min(y_shadow_1, y_shadow_2);

    if (u_shadow <= - board.goalWidth() /2 ||
        d_shadow >= board.goalWidth() /2) continue;

    shadows.push_back(std::make_pair(u_shadow, d_shadow));
  }

  // sort shadows

  std::sort(shadows.begin(), shadows.end(),
    [](std::pair<float, float> s1, std::pair<float, float> s2) {
      return s1 > s2; });

  // merge shadows

  std::vector<std::pair<float, float>> shadows_merged;
  std::pair<float, float> current_shadow;
  bool has_first = false;
  for (auto shadow : shadows) {
    if (!has_first) {
      current_shadow = shadow;
      has_first = true;
      continue;
    }

    if (shadow.second >= current_shadow.second) {
      continue;
    }

    if (shadow.first >= current_shadow.second) {
      current_shadow =
        std::make_pair(current_shadow.first, shadow.second);
    } else {
      shadows_merged.push_back(current_shadow);
      current_shadow = shadow;
    }
  }
  if (has_first) {
    shadows_merged.push_back(current_shadow);
  }

  // gather gaps from merged shadows

  std::vector<std::pair<float, float>> gaps;
  std::pair<float, float> current_gap =
    std::make_pair(board.goalWidth() / 2, -board.goalWidth() / 2);
  bool has_last = true;
  //std::cout << "--" << std::endl;
  for (auto shadow : shadows_merged) {

    if (shadow.first >= current_gap.first) {
      if (shadow.second <= current_gap.second) {
        has_last = false;
        break;
      }
      current_gap.first = shadow.second;
      continue;
    }

    gaps.push_back(std::make_pair(current_gap.first, shadow.first));
    if (shadow.second <= current_gap.second) {
      has_last = false;
      break;
    }
    current_gap.first = shadow.second;
  }
  if (has_last) {
    gaps.push_back(current_gap);
  }

  for (auto gap : gaps) {
    //std::cout << gap.first << ' ' << gap.second << std::endl;
    glColor3ubv(WHITE);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glVertex3f(gx, gap.first, 0.0f);
    glVertex3f(gx, gap.second, 0.0f);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glEnd();
    glColor3ubv(BLACK);
    glBegin(GL_LINES);
    glVertex3f(gx, gap.first, 0.0f);
    glVertex3f(gx, gap.second, 0.0f);
    glEnd();
  }
  for (auto shadow : shadows) {
    glColor3ubv(DARK_GREEN);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glVertex3f(gx, shadow.first, 0.0f);
    glVertex3f(gx, shadow.second, 0.0f);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glEnd();
    //glBegin(GL_LINES);
    //glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    //glVertex3f(gx, gap.first, 0.0f);
    //glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    //glVertex3f(gx, gap.second, 0.0f);
    //glEnd();
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
  if (board.isMaxLeft()) glColor3ubv(BLUE);
  else glColor3ubv(YELLOW);

  glRectf(-board.goalX() -board.goalDepth(), board.goalWidth() / 2,
          -board.goalX(), -board.goalWidth() / 2);

  if (board.isMaxLeft()) glColor3ubv(YELLOW);
  else glColor3ubv(BLUE);

  glRectf(board.goalX() +board.goalDepth(), board.goalWidth() / 2,
          board.goalX(), -board.goalWidth() / 2);
  draw_shadow(board);
}

void draw_board(const Board &board) {
  glColor3ubv(FIELD_GREEN);
  glRectf(-board.fieldWidth() / 2, board.fieldHeight() / 2,
          board.fieldWidth() / 2, -board.fieldHeight() / 2);

  draw_goals(board);

  for (auto robot : board.getMax().getRobots()) {
    draw_robot(robot, BLUE);
  }

  for (auto robot : board.getMin().getRobots()) {
    draw_robot(robot, YELLOW);
  }

  draw_ball(board.getBall());
}

// largely adapted from
// http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
// const char *typeface = "anonymous-pro.ttf";
// FT_Library ft;
// FT_Face face;
// FT_GlyphSlot g;
// GLuint tex;

void init_graphics() {
#ifndef __APPLE__
  char name[] = "minimax";
  char *argv[] = {name};
  int argc = 1;

  glutInit(&argc, argv);
#endif
  // if (FT_Init_FreeType(&ft)) {
  //  std::cerr << "Could not init freetype library" << std::endl;
  //  exit(EXIT_FAILURE);
  //}
  // if (FT_New_Face(ft, typeface, 0, &face)) {
  //  std::cerr << "Could not open font: " << typeface << std::endl;
  //  exit(EXIT_FAILURE);
  //}
  // FT_Set_Pixel_Sizes(face, 0, 48);
  // g = face->glyph;

  //// initialize texture for text
  // glActiveTexture(GL_TEXTURE0);
  // glGenTextures(1, &tex);
  // glBindTexture(GL_TEXTURE_2D, tex);
  //// glUniform1i(uniform_tex, 0);

  //// clamp texture at edges
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //// set proper alignment, we're using 1b grayscale
  // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // set up vertex buffer
  // GLuint vbo;
  // glGenBuffers(1, &vbo);
  // glEnableVertexAttribArray(attribute_coord);
  // glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

// void render_text(const char *text, float x, float y, float sx, float sy) {
//  const char *p;
//
//  for (p = text; *p; p++) {
//    if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
//      continue;
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows,
//    0,
//                 GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
//
//    float x2 = x + g->bitmap_left * sx;
//    float y2 = -y - g->bitmap_top * sy;
//    float w = g->bitmap.width * sx;
//    float h = g->bitmap.rows * sy;
//
//    GLfloat box[4][4] = {
//        {x2, -y2, 0, 0},
//        {x2 + w, -y2, 1, 0},
//        {x2, -y2 - h, 0, 1},
//        {x2 + w, -y2 - h, 1, 1},
//    };
//
//    glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
//    x += (g->advance.x >> 6) * sx;
//    y += (g->advance.y >> 6) * sy;
//  }
//}

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

void output(const char *text) {
  float x = 3.0, y = 15.0;
  glRasterPos2f(x, y);
  for (const char *p = text; *p; p++) {
    switch (*p) {
    case '\n':
      y += 15.0;
    case '\r':
      glRasterPos2f(x, y);
      break;
    default:
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
    }
  }
}

void draw_text(const char *text, int width, int height) {
  glColor3ubv(WHITE);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();   // save
  glLoadIdentity(); // and clear
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // glMatrixMode(GL_MODELVIEW);
  glOrtho(0, width, height, 0, 1.0, -1.0);
  output(text);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix(); // revert back to the matrix I had before.
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void draw_test(int width, int height) {
  int sx = width;
  int sy = height;

  // draw_text("The Quick Brown Fox\nJumps Over The Lazy Dog.", width, height);
}

const Robot & get_robot(int id, const Board &board) {
  for (auto &robot : board.getMax().getRobots()) {
    if (robot.getId() == id) {
      return robot;
    }
  }
  throw "robot not found!!!";
}

void draw_teamaction(const TeamAction &t_action, const Board &board) {
  for (auto action : t_action) {
    auto robot = get_robot(action->getId(), board);
    switch(action->type()) {
      case Action::MOVE: {
          auto move = std::dynamic_pointer_cast<Move>(action);
          glColor3ubv(BLACK);
          glBegin(GL_LINES);
          auto pos_i = robot.pos();
          glVertex3f(pos_i[0], pos_i[1], 0.0f);
          auto pos_f = move->pos();
          glVertex3f(pos_f[0], pos_f[1], 0.0f);
          glEnd();
          glPushMatrix();
          glTranslatef(pos_f[0], pos_f[1], 0.f);
          draw_circle(robot.radius() / 3);
          glPopMatrix();
        } break;
      case Action::PASS: {
          auto pass = std::dynamic_pointer_cast<Pass>(action);
          auto rcv = get_robot(pass->getRcvId(), board);
          glColor3ubv(GREY);
          //glColor3ubv(RED2);
          glBegin(GL_LINES);
          auto pos_i = robot.pos();
          glVertex3f(pos_i[0], pos_i[1], 0.0f);
          auto pos_b = board.getBall().pos();
          glVertex3f(pos_b[0], pos_b[1], 0.0f);
          glEnd();
          glPushAttrib(GL_ENABLE_BIT);
          //glLineStipple(1, 0xAAAA);
          glLineStipple(1, 0xF0F0);
          glEnable(GL_LINE_STIPPLE);
          glBegin(GL_LINES);
          glVertex3f(pos_b[0], pos_b[1], 0.0f);
          auto pos_f = rcv.pos();
          glVertex3f(pos_f[0], pos_f[1], 0.0f);
          glEnd();
          glPopAttrib();
        } break;
      case Action::KICK: {
          auto kick = std::dynamic_pointer_cast<Kick>(action);
          glColor3ubv(RED);
          glBegin(GL_LINES);
          auto pos_i = robot.pos();
          glVertex3f(pos_i[0], pos_i[1], 0.0f);
          auto pos_b = board.getBall().pos();
          glVertex3f(pos_b[0], pos_b[1], 0.0f);
          //auto pos_f = rcv.pos();
          //glVertex3f(pos_f[0], pos_f[1], 0.0f);
          glEnd();
        } break;
    }
  }
}

void draw_display(App *app, double fps, int width, int height) {
  char text[256];
  snprintf(text, 256, "%2.0ffps\n"
                      "uptime: %is\n"
                      "minimax: #%i\n"
                      "%i packets/s\n"
                      "%i minimax/s\n",
           fps, app->display.uptime, app->display.minimax_count,
           app->display.pps, app->display.mps);
  if (app->display.has_val) {
    char text2[256];
    snprintf(text2, 256, "value: %f", app->display.val);
    strcat(text, text2);
  }
  draw_text(text, width, height);
}

void draw_app(App *app, double fps, int width, int height) {
  draw_board(app->command_board);
  draw_teamaction(app->command, app->command_board);
  draw_display(app, fps, width, height);
}
