#include <stdio.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "gui.h"
#include "app.h"
#include "consts.h"
#include "colors.h"
#include "draw.h"
#include "app.h"

static GLFWwindow *window;
static bool mousePressed[2] = {false, false};

static void error_callback(int error, const char *description) { fputs(description, stderr); }

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    return glfwSetWindowShouldClose(window, GL_TRUE);

  if (action == GLFW_PRESS) {
    switch (key) {

    case GLFW_KEY_R: {
      app_random();
    } break;

    case GLFW_KEY_S: {
      app_decide_once();
    } break;

    case GLFW_KEY_P: {
      app_decide_toggle();
    } break;

    case GLFW_KEY_E: {
      app_eval_once();
    } break;

    case GLFW_KEY_V: {
      app_eval_toggle();
    } break;

    case GLFW_KEY_A: {
      app_apply();
    } break;

    case GLFW_KEY_Q: {
      app_toggle_experimental();
    } break;

#define SELECT_SLOT(I)                                                                                                 \
  case GLFW_KEY_##I: {                                                                                                 \
    app_select_save_slot(I);                                                                                           \
  } break;
      SELECT_SLOT(1)
      SELECT_SLOT(2)
      SELECT_SLOT(3)
      SELECT_SLOT(4)
      SELECT_SLOT(5)
      SELECT_SLOT(6)
      SELECT_SLOT(7)
      SELECT_SLOT(8)
      SELECT_SLOT(9)
      SELECT_SLOT(0)
#undef LOAD_SLOT
    case GLFW_KEY_MINUS: {
      app_save_state();
    } break;

    case GLFW_KEY_EQUAL: {
      app_load_state();
    } break;

    // move a robot
    case GLFW_KEY_M: {
      app_toggle_selected_player();
    } break;
    case GLFW_KEY_N: {
      app_select_next_robot();
    } break;
    case GLFW_KEY_UP: {
      app_move_up();
    } break;
    case GLFW_KEY_DOWN: {
      app_move_down();
    } break;
    case GLFW_KEY_LEFT: {
      app_move_left();
    } break;
    case GLFW_KEY_RIGHT: {
      app_move_right();
    } break;

    default:
      break;
    }
  }
}

static void resize_callback(GLFWwindow *window, int width, int height) {
  gui_update();
  gui_render();
}

static double zoom;
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  static constexpr double zoom_speed = 0.01;
  static constexpr double zoom_min = 0.15;
  static constexpr double zoom_max = 5.50;
  double offset2 = xoffset * std::abs(xoffset) + yoffset * std::abs(yoffset);
  // nonsqrt'd
  // zoom += offset2 * zoom_speed;
  // sqrt'd
  zoom += copysign(sqrt(std::abs(offset2)), offset2) * zoom_speed;
  // restrict zoom in [zoom_min, zoom_max] interval
  zoom = (zoom > zoom_max) ? zoom_max : (zoom < zoom_min) ? zoom_min : zoom;
  // std::cout << zoom << std::endl;
}

static bool is_drag;
void drag_callback(GLFWwindow *window, double xpos, double ypos) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  // TODO: dragging#
  // std::cout << xpos - width / 2 << ' ' << ypos - height / 2 << std::endl;
}

void cursorpos_callback(GLFWwindow *window, double xpos, double ypos) {
  if (is_drag)
    drag_callback(window, xpos, ypos);
}

void mousebutton_callback(GLFWwindow *window, int button, int action, int mods) {
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
  // render(window, width, height);
  glfwSwapBuffers(window);
}

static bool is_active = false;
void focus_callback(GLFWwindow *window, int focus) { is_active = focus == GL_TRUE; }

void gui_sync(void) {
  if (!is_active) // if running in background idle avoid high cpu usage,
    // empirical parameter
    // std::this_thread::sleep_for(std::chrono::milliseconds(96));
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

static void chars_mod_callback(GLFWwindow *window, unsigned int codepoint, int mods) {
  if (codepoint > 0 && codepoint < 0x10000)
    ImGui::GetIO().AddInputCharacter((unsigned short)codepoint);
}

void gui_init_glfw(void) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW.");
    exit(EXIT_FAILURE);
  }

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window = glfwCreateWindow(GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT, PROGRAM_NAME, NULL, NULL);
  if (!window) {
    glfwTerminate();
    fprintf(stderr, "Failed to create window.");
    exit(EXIT_FAILURE);
  }

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
  glfwMakeContextCurrent(window);
  glfwSwapInterval(2);
}

// This is the main rendering function that you have to implement and provide to
// ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void imgui_renderdrawlists(struct ImDrawList **const cmd_lists, int cmd_lists_count) {
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
    const unsigned char *vtx_buffer = (const unsigned char *)&cmd_list->vtx_buffer.front();

#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void *)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void *)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void *)(vtx_buffer + OFFSETOF(ImDrawVert, col)));
#undef OFFSETOF

    int vtx_offset = 0;
    for (size_t cmd_i = 0; cmd_i < cmd_list->commands.size(); cmd_i++) {
      const ImDrawCmd *pcmd = &cmd_list->commands[cmd_i];
      glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t) pcmd->texture_id);
      glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x),
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
static const char *imgui_getclipboardtextfn() { return glfwGetClipboardString(window); }

static void imgui_setclipboardtextfn(const char *text) { glfwSetClipboardString(window, text); }

void load_fonts_texture() {
  ImGuiIO &io = ImGui::GetIO();
  // ImFont* my_font1 = io.Fonts->AddFontDefault();
  // ImFont* my_font2 = io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf", 15.0f);
  // ImFont* my_font3 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f);
  // my_font3->DisplayOffset.y += 1;
  // ImFont* my_font4 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f);
  // my_font4->DisplayOffset.y += 1;
  // ImFont* my_font5 = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 20.0f,
  // io.Fonts->GetGlyphRangesJapanese());

  // ImFont* my_font2 = io.Fonts->AddFontFromFileTTF("../vendor/fonts/anonymous-pro.ttf", 12.0f);

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  GLuint tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t) tex_id;
}

void gui_init_imgui(void) {
  auto &io = ImGui::GetIO();
  io.IniFilename = "minimax_gui.ini";
  io.LogFilename = "minimax_gui.log";
  io.DeltaTime = 1.0f / 60.0f;            // Time elapsed since last frame, in seconds (in
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

  io.RenderDrawListsFn = imgui_renderdrawlists;
  io.SetClipboardTextFn = imgui_setclipboardtextfn;
  io.GetClipboardTextFn = imgui_getclipboardtextfn;

  load_fonts_texture();

  // Styling
  auto &style = ImGui::GetStyle();
  style.Alpha = 0.6;
  style.WindowRounding = 6.0;
  style.WindowPadding = {3.0, 3.0};
  style.FrameRounding = 2.0;
  style.FramePadding = {1.0, 1.0};
}

void gui_update(void) {
  mousePressed[0] = mousePressed[1] = false;
  glfwPollEvents();
  ImGuiIO &io = ImGui::GetIO();

  // Setup resolution (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(window, &w, &h);
  glfwGetFramebufferSize(window, &display_w, &display_h);
  io.DisplaySize =
      ImVec2((float)display_w, (float)display_h); // Display size, in pixels. For clamping windows positions.

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
  io.MouseDown[0] =
      mousePressed[0] ||
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != 0; // If a mouse press event came, always pass it as
                                                               // "mouse held this frame", so we don't miss
                                                               // click-release events that are shorter than 1
                                                               // frame.
  io.MouseDown[1] = mousePressed[1] || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

  // Start the frame
  ImGui::NewFrame();
}

void gui_init(void) {
  gui_init_glfw();
  gui_init_imgui();
}

extern bool DRAW_DECISON;

void gui_render(void) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  screen_zoom(width, height, zoom);

  draw_state(*app_state);
  if (DRAW_DECISON) {
    draw_decision(*app_decision_max, *app_state, MAX);
    draw_decision(*app_decision_min, *app_state, MIN);
  }

  draw_options_window();

  ImGui::Begin("App status");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate);
  draw_app_status();
  ImGui::End();

  ImGui::Begin("Calibration");
  ImGui::Checkbox("KICK_IF_NO_PASS", &KICK_IF_NO_PASS);
  ImGui::SliderInt("RAMIFICATION_NUMBER", &RAMIFICATION_NUMBER, 10, 20000);
  ImGui::SliderInt("FULL_CHANGE_PERCENTAGE", &FULL_CHANGE_PERCENTAGE, 0, 100);
  ImGui::SliderInt("MAX_DEPTH", &MAX_DEPTH, 0, 3);

#define SLIDER(V, A, B) ImGui::SliderFloat(#V, &V, A, B);
  SLIDER(KICK_POS_VARIATION, 0, 1);
  SLIDER(MIN_GAP_TO_KICK, 0, 180)
  SLIDER(WEIGHT_MOVE_DIST_TOTAL, 0, 10);
  SLIDER(WEIGHT_MOVE_DIST_MAX, 0, 10);
  SLIDER(WEIGHT_MOVE_CHANGE, 0, 5000);
  SLIDER(WEIGHT_PASS_CHANGE, 0, 5000);
  SLIDER(WEIGHT_KICK_CHANGE, 0, 5000);
  SLIDER(TOTAL_MAX_GAP_RATIO, 0, 1)
  SLIDER(WEIGHT_ATTACK, 0, 5000)
  SLIDER(WEIGHT_SEE_ENEMY_GOAL, 0, 5000)
  SLIDER(WEIGHT_BLOCK_GOAL, 0, 5000)
  SLIDER(WEIGHT_BLOCK_ATTACKER, 0, 5000)
  SLIDER(WEIGHT_RECEIVERS_NUM, 0, 5000)
  SLIDER(DIST_GOAL_PENAL, 0, 5000);
  SLIDER(DIST_GOAL_TO_PENAL, 0, 6)
  SLIDER(MOVE_RADIUS_0, 0, 10)
  SLIDER(MOVE_RADIUS_1, 0, 10)
  SLIDER(MOVE_RADIUS_2, 0, 10)
#undef SLIDER
  ImGui::End();

  ImGui::Render();
  glfwSwapBuffers(window);
}

bool gui_should_close(void) { return glfwWindowShouldClose(window); }

void gui_destroy(void) {
  glfwDestroyWindow(window);
  glfwTerminate();
}
