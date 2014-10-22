#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>

#include "minimax.h"
#include "draw.h"

//
// STATIC DATA
//

static App *app;

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
      std::lock_guard<std::mutex> _(app->board_mutex);
      app->board = Board::randomBoard();
    } break;
    default:
      break;
    }
  }
}

static void render(GLFWwindow *window, int width, int height);

// static void resize_callback(GLFWwindow *window, int width, int height) {
//  render(window, width, height);
//}

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

void refresh_callback(GLFWwindow *window) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  render(window, width, height);
}

static bool is_active = false;
void focus_callback(GLFWwindow *window, int focus) {
  is_active = focus == GL_TRUE;
}

void init_callbacks(GLFWwindow *window) {
  zoom = 0.28;
  is_drag = false;
  // glfwSetWindowSizeCallback(window, resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorPosCallback(window, cursorpos_callback);
  glfwSetMouseButtonCallback(window, mousebutton_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetWindowRefreshCallback(window, refresh_callback);
  glfwSetWindowFocusCallback(window, focus_callback);
}

//
// MAIN LOGIC
//

static int n_frames = 0;
static double last_time = 0.0;
static double fps = 0;

void render(GLFWwindow *window, int width, int height) {
  display(width, height, zoom);

  // calculate current FPS
  double current_time = glfwGetTime();
  n_frames++;
  if (current_time - last_time >= 1.0) { // If last cout was more than 1 sec ago
    fps = (double)n_frames;
    // glfwSetWindowTitle(window, title);
    n_frames = 0;
    last_time += 1.0;
  }

  // draw the copied board, lock area could be reduced maybe

  // draw the text buffer
  {
    std::lock_guard<std::mutex> _(app->display_mutex);
    draw_board(app->command_board);
    draw_teamaction(app->command, app->command_board);
    draw_display(app, fps, width, height);
  }

  // poll events
  glfwSwapBuffers(window);
}

int main(int argc, char **argv) {
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwSetErrorCallback(error_callback);

  App::run([&](App &app_) {
    app = &app_;

    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(944, 740, "Minimax GUI", NULL, NULL);
    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    init_callbacks(window);
    init_graphics();
    glfwMakeContextCurrent(window);
    glfwSwapInterval(2);

    double last_time = 0.0;
    while (!glfwWindowShouldClose(window)) {

      refresh_callback(window);
      if (!is_active) // if running in background idle avoid high cpu usage,
                      // empirical parameter
        std::this_thread::sleep_for(std::chrono::milliseconds(96));

      glfwPollEvents();
    }

    glfwDestroyWindow(window);
    std::cout << "\rGoodbye!" << std::endl;
  });

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
