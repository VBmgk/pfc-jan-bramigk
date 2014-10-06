#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <GLFW/glfw3.h>
#include <zmq.hpp>

#include "discrete.pb.h"
#include "update.pb.h"
#include "timer.h"

#include "base.h"
#include "body.h"
#include "action.h"
#include "minimax.h"

using namespace std;

static void error_callback(int error, const char* description) {
    cerr << description << endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

class MainWindow {
  GLFWwindow* window;

 public:
  MainWindow(int width=640, int height=480) {
    glfwSetErrorCallback(error_callback);
    window = glfwCreateWindow(width, height, "Minimax GUI", NULL, NULL);

    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
  }
  ~MainWindow() {
    glfwDestroyWindow(window);
  }

  void draw(const Board& board) {
  }

  void run() {
    Timer tmr;
    Board board;
    mutex board_mutex;
    bool recv(true);

    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    thread zmq_thread([&] () {
      zmq::context_t context (1);
      zmq::socket_t socket(context, ZMQ_REP);

      socket.bind("tcp://*:5555");
      zmq::message_t buffer(1024);
      string data;

      Board local_board;

      while (recv) {
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
      // draw the copied board, lock area could be reduced
      board_mutex.lock();
      draw(board);
      board_mutex.unlock();

      // poll events
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    recv = false;
    zmq_thread.join();
  }

  void demo() {
    while (!glfwWindowShouldClose(window)) {
      float ratio;
      int width, height;

      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;

      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);

      glLoadIdentity();
      glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);

      glBegin(GL_TRIANGLES);
      glColor3f(1.f, 0.f, 0.f);
      glVertex3f(-0.6f, -0.4f, 0.f);
      glColor3f(0.f, 1.f, 0.f);
      glVertex3f(0.6f, -0.4f, 0.f);
      glColor3f(0.f, 0.f, 1.f);
      glVertex3f(0.f, 0.6f, 0.f);
      glEnd();

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }
};

int main(int argc, char** argv) {
  if (!glfwInit())
    exit(EXIT_FAILURE);

  MainWindow app;
  app.run();

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
