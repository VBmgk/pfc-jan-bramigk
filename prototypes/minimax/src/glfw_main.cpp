#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "app.h"
#include "draw.h"


//
// STATIC DATA
//

static App *app;
static GLFWwindow *window;
static bool mousePressed[2] = {false, false};

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
      app->random();
    } break;

    case GLFW_KEY_S: {
      app->minimax_once();
    } break;

    case GLFW_KEY_P: {
      app->minimax_toggle();
    } break;

    case GLFW_KEY_E: {
      app->eval_once();
    } break;

    case GLFW_KEY_A: {
      app->apply();
    } break;

    case GLFW_KEY_Q: {
      app->toggle_experimental();
    } break;

#define LOAD_SLOT(I)                                                           \
  case GLFW_KEY_##I: {                                                         \
    app->load_saved_slot(I);                                                   \
  } break;
      LOAD_SLOT(1)
      LOAD_SLOT(2)
      LOAD_SLOT(3)
      LOAD_SLOT(4)
      LOAD_SLOT(5)
      LOAD_SLOT(6)
      LOAD_SLOT(7)
      LOAD_SLOT(8)
      LOAD_SLOT(9)
      LOAD_SLOT(0)
#undef LOAD_SLOT
    case GLFW_KEY_EQUAL: {
      app->save_board();
    } break;

    // move a robot
    case GLFW_KEY_M: {
      app->switch_select_team();
    } break;
    case GLFW_KEY_N: {
      app->next_robot();
    } break;
    case GLFW_KEY_UP: {
      app->move_up();
    } break;
    case GLFW_KEY_DOWN: {
      app->move_down();
    } break;
    case GLFW_KEY_LEFT: {
      app->move_left();
    } break;
    case GLFW_KEY_RIGHT: {
      app->move_right();
    } break;

    default:
      break;
    }
  }
}

static void render(GLFWwindow *window, int width, int height);

static void UpdateImGui();
static void resize_callback(GLFWwindow *window, int width, int height) {
  UpdateImGui();
  render(window, width, height);
  glfwSwapBuffers(window);
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

void refresh_callback(GLFWwindow *window) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  render(window, width, height);
  glfwSwapBuffers(window);
}

static bool is_active = false;
void focus_callback(GLFWwindow *window, int focus) {
  is_active = focus == GL_TRUE;
}

static void chars_mod_callback(GLFWwindow *window, unsigned int codepoint, int mods) {
  if (codepoint > 0 && codepoint < 0x10000)
    ImGui::GetIO().AddInputCharacter((unsigned short)codepoint);
}

void init_callbacks(GLFWwindow *window) {
  zoom = 0.28;
  is_drag = false;
  glfwSetWindowSizeCallback(window, resize_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCharModsCallback(window, chars_mod_callback);
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
    draw_teamaction(app->command, app->command_board, MAX);
    draw_teamaction(app->enemy_command, app->command_board, MIN);
    //draw_display(app, fps, width, height);
    ImGui::Text("uptime: %is", app->display.uptime);
    ImGui::Text("minimax: #%i", app->display.minimax_count);
    ImGui::Text("%i packets/s", app->display.pps);
    ImGui::Text("%i minimax/s", app->display.mps);
    ImGui::Text("minimax: %f", app->display.minimax_val);
    if (app->display.has_val)
      ImGui::Text("value: %f", app->display.val);
  }

  ImGui::Render();
}

// This is the main rendering function that you have to implement and provide to
// ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
static void ImImpl_RenderDrawLists(ImDrawList **const cmd_lists,
                                   int cmd_lists_count) {
  if (cmd_lists_count == 0)
    return;

  // We are using the OpenGL fixed pipeline to make the example code simpler to
  // read!
  // A probable faster way to render would be to collate all vertices from all
  // cmd_lists into a single vertex buffer.
  // Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, vertex/texcoord/color pointers.
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);

  // Setup orthographic projection matrix
  const float width = ImGui::GetIO().DisplaySize.x;
  const float height = ImGui::GetIO().DisplaySize.y;
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Render command lists
  for (int n = 0; n < cmd_lists_count; n++) {
    const ImDrawList *cmd_list = cmd_lists[n];
    const unsigned char *vtx_buffer =
        (const unsigned char *)&cmd_list->vtx_buffer.front();

#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                    (void *)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                      (void *)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
                   (void *)(vtx_buffer + OFFSETOF(ImDrawVert, col)));
#undef OFFSETOF

    int vtx_offset = 0;
    for (size_t cmd_i = 0; cmd_i < cmd_list->commands.size(); cmd_i++) {
      const ImDrawCmd *pcmd = &cmd_list->commands[cmd_i];
      glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t) pcmd->texture_id);
      glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w),
                (int)(pcmd->clip_rect.z - pcmd->clip_rect.x),
                (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
      glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
      vtx_offset += pcmd->vtx_count;
    }
  }

  // Restore modified state
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

// NB: ImGui already provide OS clipboard support for Windows so this isn't
// needed if you are using Windows only.
static const char *ImImpl_GetClipboardTextFn() {
  return glfwGetClipboardString(window);
}

static void ImImpl_SetClipboardTextFn(const char *text) {
  glfwSetClipboardString(window, text);
}

void LoadFontsTexture() {
  ImGuiIO &io = ImGui::GetIO();
  // ImFont* my_font1 = io.Fonts->AddFontDefault();
  // ImFont* my_font2 =
  // io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf", 15.0f);
  // ImFont* my_font3 =
  // io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f);
  // my_font3->DisplayOffset.y += 1;
  // ImFont* my_font4 =
  // io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f);
  // my_font4->DisplayOffset.y += 1;
  // ImFont* my_font5 =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 20.0f,
  // io.Fonts->GetGlyphRangesJapanese());

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  GLuint tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
               GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t) tex_id;
}

void InitImGui() {
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = "minimax_gui.ini";
  io.LogFilename = "minimax_gui.log";
  io.DeltaTime = 1.0f / 60.0f; // Time elapsed since last frame, in seconds (in
                               // this sample app we'll override this every
                               // frame because our time step is variable)
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB; // Keyboard mapping. ImGui will use
                                          // those indices to peek into the
                                          // io.KeyDown[] array.
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

  io.RenderDrawListsFn = ImImpl_RenderDrawLists;
  io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
  io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;

  LoadFontsTexture();
}

static void UpdateImGui() {
  ImGuiIO &io = ImGui::GetIO();

  // Setup resolution (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(window, &w, &h);
  glfwGetFramebufferSize(window, &display_w, &display_h);
  io.DisplaySize = ImVec2(
      (float)display_w, (float)
      display_h); // Display size, in pixels. For clamping windows positions.

  // Setup time step
  static double time = 0.0f;
  const double current_time = glfwGetTime();
  io.DeltaTime = (float)(current_time - time);
  time = current_time;

  // Setup inputs
  // (we already got mouse wheel, keyboard keys & characters from glfw callbacks
  // polled in glfwPollEvents())
  double mouse_x, mouse_y;
  glfwGetCursorPos(window, &mouse_x, &mouse_y);
  mouse_x *= (float)display_w / w; // Convert mouse coordinates to pixels
  mouse_y *= (float)display_h / h;
  io.MousePos = ImVec2((float)mouse_x, (float)mouse_y); // Mouse position, in
                                                        // pixels (set to -1,-1
                                                        // if no mouse / on
                                                        // another screen, etc.)
  io.MouseDown[0] = mousePressed[0] ||
                    glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) !=
                        0; // If a mouse press event came, always pass it as
                           // "mouse held this frame", so we don't miss
                           // click-release events that are shorter than 1
                           // frame.
  io.MouseDown[1] = mousePressed[1] ||
                    glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

  // Start the frame
  ImGui::NewFrame();
}

int main(int argc, char **argv) {
  if (!glfwInit()) {
    return EXIT_SUCCESS;
  }

  glfwSetErrorCallback(error_callback);

  App::run([&](App &app_) {
    app = &app_;

    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(944, 740, "Minimax GUI", NULL, NULL);
    if (!window) {
      glfwTerminate();
      return;
    }

    init_callbacks(window);
    init_graphics();
    glfwMakeContextCurrent(window);
    glfwSwapInterval(2);

    InitImGui();

    double last_time = 0.0;
    while (!glfwWindowShouldClose(window)) {
      mousePressed[0] = mousePressed[1] = false;
      glfwPollEvents();
      UpdateImGui();

      if (!is_active) // if running in background idle avoid high cpu usage,
                      // empirical parameter
        std::this_thread::sleep_for(std::chrono::milliseconds(96));

      //ImGui::ShowTestWindow();
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      render(window, width, height);
      glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    std::cout << "\rGoodbye!" << std::endl;
  });

  glfwTerminate();
  return EXIT_SUCCESS;
}
