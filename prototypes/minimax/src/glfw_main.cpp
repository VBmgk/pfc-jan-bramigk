#include <iostream>
#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>

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

  void run() {
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
