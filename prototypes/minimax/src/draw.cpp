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
#include "imgui.h"

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

bool solve_Ax_b(float a11, float a12,
                float a21, float a22, std::pair<float, float>& x,
                float b1, float b2);

std::vector<std::pair<float, float>>
Board::getGoalGapsDraw(Player player, const Body &body) const {
  auto gx = goalPos(player)[0];

#if 0
  glPushMatrix();
  auto pos = ball.pos();
  glTranslatef(pos[0], pos[1], 0.f);
  glColor3ubv(RED);
  draw_circle(KICK_POS_VARIATION);
  glPopMatrix();
#endif

  // collect shadows
  std::vector<std::pair<double, double>> shadows;
  for (auto _robot : getRobotsMoving()) {
    if (_robot == &body)
      continue;
    auto &robot = *_robot;
    auto d = robot.pos() - ball.pos();
    auto k = d * d - Robot::radius() * Robot::radius();

    if (k <= 0)
      continue; // FIXME: should return maximum shadow or no gap
    if (d[0] * gx <= 0)
      continue;

    bool valid_solution = true; // XXX

    // --------------------------------------------------------------------------
    // Old way to compute the gap, consider the ball as a point light source
    //float tan_alpha = Robot::radius() / std::sqrt(k);
    //float tan_theta = d[1] / std::fabs(d[0]);
    //float tan_1 = (tan_theta + tan_alpha) / (1 - tan_theta * tan_alpha);
    //float tan_2 = (tan_theta - tan_alpha) / (1 + tan_theta * tan_alpha);
    float y_shadow_1; // = tan_1 * std::fabs(ball.pos()[0] - gx) + ball.pos()[1];
    float y_shadow_2; // = tan_2 * std::fabs(ball.pos()[0] - gx) + ball.pos()[1];
    // New
    // n: vector normal to the line that join
    //    ball and the robot
    // n = [ 0 1 ] 
    //     [-1 0 ].(b.pos() - r.pos())/|| b.pos() - r.pos() ||
    auto n = Vector(-(ball.pos().y() - robot.pos().y()),
                     (ball.pos().x() - robot.pos().x()));
    // normalization, to use radius later
    n = n * (1.0 / d.norm()) ;

    // nu -- upper line that toches the imaginary radius
    //       of the ball and the radius of the robot
    //       nu = Ru - Bu = radius_robot * n + robot.pos() -
    //                     (radius_body  * n + ball.pos())
    // nd -- lower line that toches the imaginary radius
    //       of the ball and the radius of the robot
    //       nd = Rd - Bd = -radius_robot * n + robot.pos() -
    //                     (-radius_body  * n + ball.pos())
    auto rd = n * -Robot::radius()    + robot.pos();
    auto ru = n *  Robot::radius()    + robot.pos();
    auto bu = n *  KICK_POS_VARIATION + ball.pos();
    auto bd = n * -KICK_POS_VARIATION + ball.pos();
    auto nu = ru - bu;
    auto nd = rd - bd;

    // solving sistem: [ nu_x  nd_x ] [ auxu ]
    //                 [ nu_y  nd_y ] [ auxd ] = Rd - Ru
    // to eliminate robots with no shadow
    std::pair<float, float> aux = std::make_pair(0, 0);
    if (solve_Ax_b(nu.x(), nd.x(),
                   nu.y(), nd.y(), aux,
                  (rd - ru).x(), (rd - ru).y())) {
      // system has solution
      // p = intersection of lu = { aux . nu + ru} and
      //                     ld = { aux . nd + rd}
      auto p = nu * aux.first + ru;

      // intersection lies between goal and robot
      // so there is no shadow
      //if (p.x() * gx > 0 && p.x() * gx <= gx * gx)
      //  valid_solution = false;//continue;
      if (aux.first > 0) {
        if (gx < 0 && p.x() > gx)
          valid_solution = false;//continue;
        if (gx > 0 && p.x() < gx)
          valid_solution = false;//continue;
      }
    }

    // if there is no interseption is parallel to this line.
    // [ gx   ]
    // [ yu/d ] = ku/d'. nu/d + ru/d
    //  
    //     [ nu/d_x   0 ] [ ku/d'] = [ gx - ru/d_x ]
    // --> [ nu/d_y  -1 ] [ yu/d ] = [    - ru/d_y ]
    if (solve_Ax_b(nu.x(),  0,
                   nu.y(), -1, aux,
                   gx - ru.x(), - ru.y())) {
      // system has solution: aux --> ku', yu
      y_shadow_1 = aux.second;
    } else {
      y_shadow_1 = (nu.y() > 0 ? std::numeric_limits<float>::infinity():
                                -std::numeric_limits<float>::infinity());
    }

    if (solve_Ax_b(nd.x(),  0,
                   nd.y(), -1, aux,
                   gx - rd.x(), - rd.y())) {
      // system has solution: aux --> kd', yd
      y_shadow_2 = aux.second;
    } else {
      y_shadow_2 = (nd.y() > 0 ? std::numeric_limits<float>::infinity():
                                -std::numeric_limits<float>::infinity());
    }

    // Old way code
    //if ((ball.pos()[0] - robot.pos()[0] + Robot::radius()) *
    //        (ball.pos()[0] - robot.pos()[0] - Robot::radius()) <
    //    0) {
    //  if (ball.pos()[1] > robot.pos()[1])
    //    y_shadow_2 = -std::numeric_limits<float>::infinity();
    //  else
    //    y_shadow_1 = std::numeric_limits<float>::infinity();
    //}

    // --------------------------------------------------------------------------
    float u_shadow = std::max(y_shadow_1, y_shadow_2);
    float d_shadow = std::min(y_shadow_1, y_shadow_2);

    if (!valid_solution)
      continue;

#if 0
    glColor3ubv(RED2);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(robot.pos()[0], robot.pos()[1], 0.0f);
    glVertex3f(ru.x(), ru.y(), 0.0f);
    glVertex3f(gx, y_shadow_1, 0.0f);
    glVertex3f(gx, y_shadow_2, 0.0f);
    glVertex3f(rd.x(), rd.y(), 0.0f);
    glEnd();

    glColor3ubv(RED);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(ball.pos()[0], ball.pos()[1], 0.0f);
    glVertex3f(bu.x(), bu.y(), 0.0f);
    glVertex3f(ru.x(), ru.y(), 0.0f);
    glVertex3f(rd.x(), rd.y(), 0.0f);
    glVertex3f(bd.x(), bd.y(), 0.0f);
    glEnd();
#endif

#if 0
    glColor3ubv(DARK_GREEN);
    glBegin(GL_LINES);
    glVertex3f(bu.x(), bu.y(), 0.0f);
    glVertex3f(gx, y_shadow_1, 0.0f);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(bd.x(), bd.y(), 0.0f);
    glVertex3f(gx, y_shadow_2, 0.0f);
    glEnd();
#endif

    if (u_shadow <= -goalWidth() / 2 || d_shadow >= goalWidth() / 2)
      continue;

    shadows.push_back(std::make_pair(u_shadow, d_shadow));
  }

  // sort shadows in descending order by the first parameter

  std::sort(shadows.begin(), shadows.end(),
            [](std::pair<float, float> s1,
               std::pair<float, float> s2) { return s1 > s2; });

  // merge shadows so no shadow overlap

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
      current_shadow = std::make_pair(current_shadow.first, shadow.second);
    } else {
      shadows_merged.push_back(current_shadow);
      current_shadow = shadow;
    }
  }
  if (has_first) {
    shadows_merged.push_back(current_shadow);
  }

  // gather gaps on the goal from merged shadows

  std::vector<std::pair<float, float>> gaps;
  std::pair<float, float> current_gap =
      std::make_pair(goalWidth() / 2, -goalWidth() / 2);
  bool has_last = true;
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

  return gaps;
}

void draw_test(const Board &board) {
  auto rwb = board.getRobotWithBall();
  if (std::get<0>(rwb) != nullptr)
    board.getGoalGapsDraw(std::get<1>(rwb)==MIN?MAX:MIN, *std::get<0>(rwb));
}

void draw_board(const Board &board) {
  glColor3ubv(FIELD_GREEN);
  glRectf(-board.fieldWidth() / 2, board.fieldHeight() / 2,
          board.fieldWidth() / 2, -board.fieldHeight() / 2);

  draw_goals(board);
  draw_shadow(board);
  draw_test(board);

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
    default: {}
    }
  }
}

void draw_app(App *app, double fps, int width, int height) {
  draw_board(app->command_board);
  draw_teamaction(app->command, app->command_board, MAX);
  draw_teamaction(app->enemy_command, app->command_board, MIN);
}
