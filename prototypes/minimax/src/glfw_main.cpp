#include <stdio.h>
#include <iostream>
#include <GLFW/glfw3.h>

#include "minimax.h"
#include "draw.h"

//
// STATIC DATA
//

static Board *board;
static std::mutex *board_mutex;

//
// CALLBACKS
//

static void error_callback(int error, const char *description) {
  std::cerr << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    return glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_R: {
      std::lock_guard<std::mutex> _(*board_mutex);
      *board = Board::randomBoard();
    } break;
    default:
      break;
    }
  }
}

static void redraw(GLFWwindow *window, int width, int height);
static void resize_callback(GLFWwindow *window, int width, int height) {
  redraw(window, width, height);
}

static double zoom;
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  static constexpr double zoom_speed = 0.01;
  static constexpr double zoom_min = 0.15;
  static constexpr double zoom_max = 5.50;
  double offset2 = xoffset * abs(xoffset) + yoffset * abs(yoffset);
  // nonsqrt'd
  // zoom += offset2 * zoom_speed;
  // sqrt'd
  zoom += copysign(sqrt(abs(offset2)), offset2) * zoom_speed;
  // restrict zoom in [zoom_min, zoom_max] interval
  zoom = (zoom > zoom_max) ? zoom_max : (zoom < zoom_min) ? zoom_min : zoom;
  // std::cout << zoom << std::endl;
}

static bool is_drag;
void drag_callback(GLFWwindow *window, double xpos, double ypos) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  // TODO: dragging
  // std::cout << xpos - width / 2 << ' ' << ypos - height / 2 << std::endl;
}

void cursorpos_callback(GLFWwindow *window, double xpos, double ypos) {
  if (is_drag)
    drag_callback(window, xpos, ypos);
}

void mousebutton_callback(GLFWwindow *window, int button, int action,
                          int mods) {
  static constexpr int drag_button = GLFW_MOUSE_BUTTON_LEFT;
  if (button == drag_button) {
    if (action == GLFW_PRESS)
      is_drag = true;
    else if (action == GLFW_RELEASE)
      is_drag = false;
  }
}

void init_callbacks(GLFWwindow *window) {
  zoom = 0.28;
  is_drag = false;
  glfwSetWindowSizeCallback(window, resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorPosCallback(window, cursorpos_callback);
  glfwSetMouseButtonCallback(window, mousebutton_callback);
  glfwSetKeyCallback(window, key_callback);
}

//
// MAIN LOGIC
//

void redraw(GLFWwindow *window, int width, int height) {
  display(width, height, zoom);

  // draw the copied board, lock area could be reduced maybe
  {
    std::lock_guard<std::mutex> _(*board_mutex);
    draw_board(*board);
  }

  draw_test(width, height);

  // poll events
  glfwSwapBuffers(window);
  glfwPollEvents();
}

int main(int argc, char **argv) {
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow *window = glfwCreateWindow(944, 740, "Minimax GUI", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  init_callbacks(window);
  init_graphics();
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  Minimax::run_minimax([&window](Board &board_, std::mutex &board_mutex_) {
    board = &board_;
    board_mutex = &board_mutex_;
    while (!glfwWindowShouldClose(window)) {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      redraw(window, width, height);
    }
    std::cout << "\rGoodbye!" << std::endl;
  });

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
