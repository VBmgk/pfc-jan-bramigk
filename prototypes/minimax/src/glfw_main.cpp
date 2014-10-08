#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <GLFW/glfw3.h>
//#ifdef __APPLE__
//#include <OpenGL/glu.h>
//#else
//#include <GL/glu.h>
//#endif
#include <zmq.hpp>

#include "discrete.pb.h"
#include "update.pb.h"
#include "timer.h"

#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

using namespace std;

//
// STATIC DATA
//

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

static Board board;
static mutex board_mutex;

//
// CALLBACKS
//

static void error_callback(int error, const char* description) {
    cerr << description << endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    return glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_R:
        board = Board::randomBoard();
        break;
      default:
        break;
    }
  }
}

static void redraw(GLFWwindow* window, int width, int height);
static void resize_callback(GLFWwindow* window, int width, int height) {
  redraw(window, width, height);
}

static double zoom;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  static constexpr double zoom_speed = 0.01;
  static constexpr double zoom_min = 0.15;
  static constexpr double zoom_max = 5.50;
  double offset2 = xoffset * abs(xoffset) + yoffset * abs(yoffset);
  // nonsqrt'd
  //zoom += offset2 * zoom_speed;
  // sqrt'd
  zoom += copysign(sqrt(abs(offset2)), offset2) * zoom_speed;
  // restrict zoom in [zoom_min, zoom_max] interval
  zoom = (zoom > zoom_max) ? zoom_max : (zoom < zoom_min) ? zoom_min : zoom;
  //cout << zoom << endl;
}

static bool is_drag;
void drag_callback(GLFWwindow* window, double xpos, double ypos) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  // TODO: dragging
  //cout << xpos - width / 2 << ' ' << ypos - height / 2 << endl;
}

void cursorpos_callback(GLFWwindow* window, double xpos, double ypos) {
  if (is_drag)
    drag_callback(window, xpos, ypos);
}

void mousebutton_callback(GLFWwindow* window, int button, int action, int mods) {
  static constexpr int drag_button = GLFW_MOUSE_BUTTON_LEFT;
  if (button == drag_button) {
    if (action == GLFW_PRESS)
      is_drag = true;
    else if (action == GLFW_RELEASE)
      is_drag = false;
  }
}

void init_callbacks(GLFWwindow* window) {
  zoom = 0.28;
  is_drag = false;
  glfwSetWindowSizeCallback(window, resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorPosCallback(window, cursorpos_callback);
  glfwSetMouseButtonCallback(window, mousebutton_callback);
  glfwSetKeyCallback(window, key_callback);
}

//
// DRAWING FUNCTIONS
//

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

void redraw(GLFWwindow* window, int width, int height) {
  glfwGetFramebufferSize(window, &width, &height);
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

  // draw the copied board, lock area could be reduced maybe
  board_mutex.lock();
  draw_board(board);
  board_mutex.unlock();

  // poll events
  glfwSwapBuffers(window);
  glfwPollEvents();
}

//
// MAIN LOGIC
//

void run(GLFWwindow* window) {
  //Timer tmr;
  bool should_recv(true);

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  thread zmq_thread([&] () {
    zmq::context_t context (1);
    zmq::socket_t socket(context, ZMQ_REP);

    static const char * addr = "tcp://*:5555";
    socket.bind(addr);
    zmq::message_t buffer(1024);
    string data;

    cout << "listening on " << addr << endl;

    Board local_board;

    while (should_recv) {
      try {
        if (socket.recv(&buffer, ZMQ_RCVTIMEO)) {
          string buffer_str((char*)buffer.data(), buffer.size());
          roboime::Update u;
          u.ParseFromString(buffer_str);
          cout << u.ball().x() << endl;
          // TODO: update local_board with u

          board_mutex.lock();
          board = local_board;
          board_mutex.unlock();

          zmq::message_t command_message(data.length());
          memcpy((void *) command_message.data(), data.c_str(), data.length());
          socket.send(command_message);
        }
      } catch(zmq::error_t e) {
        cerr << "error" << endl;
      }
    }
  });

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    redraw(window, width, height);
  }

  should_recv = false;
  zmq_thread.join();
}

int main(int argc, char** argv) {
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow* window = glfwCreateWindow(944, 740, "Minimax GUI", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  init_callbacks(window);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  run(window);

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
