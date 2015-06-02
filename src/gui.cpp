#include <stdio.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <imgui.h>

#include <GLFW/glfw3.h>
#ifdef _MSC_VER
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

#include "gui.h"
#include "app.h"
#include "consts.h"
#include "colors.h"
#include "draw.h"
#include "app.h"
#include "utils.h"
#include "suggestions.h"

static GLFWwindow *window;
static bool mouse_pressed[3] = {false, false, false};
static float mouse_wheel = 0.0f;
static GLuint font_texture = 0;

static void error_callback(int /*error*/, const char *description) {
  fputs(description, stderr);
}

static void key_callback(GLFWwindow *window, int key, int, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    return glfwSetWindowShouldClose(window, GL_TRUE);

  ImGuiIO &io = ImGui::GetIO();
  if (action == GLFW_PRESS)
    io.KeysDown[key] = true;
  if (action == GLFW_RELEASE)
    io.KeysDown[key] = false;

  (void)mods; // Modifiers are not reliable across systems
  io.KeyCtrl =
      io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift =
      io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];

  if (!io.WantCaptureKeyboard &&
      (action == GLFW_PRESS || action == GLFW_REPEAT)) {
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

#define SELECT_SLOT(I)                                                         \
  case GLFW_KEY_##I: {                                                         \
    app_select_save_slot(I);                                                   \
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
    case GLFW_KEY_B: {
      app_select_ball();
    } break;

    default:
      break;
    }
  }
}

static void resize_callback(GLFWwindow *, int /*width*/, int /*height*/) {
  gui_new_frame();
  gui_render();
}

static bool is_drag = false;
static bool is_down = false;
static double zoom = 1.0;
static double drag_x = 0.0;
static double drag_y = 0.0;
static double start_drag_x = 0.0;
static double start_drag_y = 0.0;
static double current_drag_x = 0.0;
static double current_drag_y = 0.0;
static double last_drag_x = 0.0;
static double last_drag_y = 0.0;
static double screen_xpos = 0.0;
static double screen_ypos = 0.0;

static void scroll_callback(GLFWwindow *, double xoffset, double yoffset) {
  static constexpr double zoom_speed = 0.01;
  static constexpr double zoom_min = 0.15;
  static constexpr double zoom_max = 5.50;

  if (!ImGui::IsMouseHoveringAnyWindow()) {
    double offset2 = xoffset * std::abs(xoffset) + yoffset * std::abs(yoffset);
    // nonsqrt'd
    // zoom += offset2 * zoom_speed;
    // sqrt'd
    zoom += copysign(sqrt(std::abs(offset2)), offset2) * zoom_speed;
    // restrict zoom in [zoom_min, zoom_max] interval
    zoom = (zoom > zoom_max) ? zoom_max : (zoom < zoom_min) ? zoom_min : zoom;

    // TODO: update drag_x and drag_y accordingly
  }

  mouse_wheel +=
      (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

static void drag_callback(GLFWwindow *window, double xpos, double ypos) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  double w = (double)width;
  double h = (double)height;
  current_drag_x = xpos - w / 2;
  current_drag_y = ypos - h / 2;

  double drag_x_min = -w * 0.9;
  double drag_x_max = +w * 0.9;
  double drag_y_min = -h * 0.9;
  double drag_y_max = +h * 0.9;

  drag_x = last_drag_x + current_drag_x - start_drag_x;
  drag_y = last_drag_y + current_drag_y - start_drag_y;

  drag_x = fmin(drag_x_max, fmax(drag_x_min, drag_x));
  drag_y = fmin(drag_y_max, fmax(drag_y_min, drag_y));
}

static void no_drag_click_callback(GLFWwindow *window, double xpos,
                                   double ypos) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  double w = (double)width;
  double h = (double)height;
  float field_x = (xpos - w / 2 - drag_x) / w / zoom * 2 * w / h;
  float field_y = (h / 2 - ypos + drag_y) / h / zoom * 2;
  if (app_selected_suggestion >= 0 &&
      app_selected_suggestion < app_suggestions->tables_count) {
    auto &table = app_suggestions->tables[app_selected_suggestion];
    int s = add_spot(table);
    if (s >= 0) {
      table.spots[s - 1] = {field_x, field_y};
    }
  }
}

static void cursorpos_callback(GLFWwindow *window, double xpos, double ypos) {
  if (is_down && !is_drag) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    double w = (double)width;
    double h = (double)height;
    start_drag_x = xpos - w / 2;
    start_drag_y = ypos - h / 2;
  }

  if (!ImGui::IsMouseHoveringAnyWindow()) {

    if (!ImGui::IsMouseDragging())
      is_drag = is_down;

    if (is_drag)
      drag_callback(window, xpos, ypos);
  }

  screen_xpos = xpos;
  screen_ypos = ypos;
}

static void mousebutton_callback(GLFWwindow *, int button, int action,
                                 int /*mods*/) {
  static constexpr int drag_button = GLFW_MOUSE_BUTTON_LEFT;

  if (action == GLFW_PRESS && button >= 0 && button < 3)
    mouse_pressed[button] = true;

  if (button == drag_button) {
    if (action == GLFW_PRESS) {
      is_down = true;
    } else if (action == GLFW_RELEASE) {
      is_down = false;
      if (is_drag) {
        is_drag = false;
        last_drag_x = drag_x;
        last_drag_y = drag_y;
      } else if (!ImGui::IsMouseHoveringAnyWindow()) {
        no_drag_click_callback(window, screen_xpos, screen_ypos);
      }
    }
  }
}

static void refresh_callback(GLFWwindow *window) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  // render(window, width, height);
  glfwSwapBuffers(window);
}

static bool is_active = false;
static void focus_callback(GLFWwindow *, int focus) {
  is_active = focus == GL_TRUE;
}

void gui_sync(void) {
  if (!is_active) // if running in background idle avoid high cpu usage,
    // empirical parameter
    // std::this_thread::sleep_for(std::chrono::milliseconds(96));
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

// void chars_mod_callback(GLFWwindow *window, unsigned int codepoint,
// int
// mods);
static void chars_mod_callback(GLFWwindow *, unsigned int codepoint, int) {
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

  window = glfwCreateWindow(GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT, PROGRAM_NAME,
                            NULL, NULL);
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

// This is the main rendering function that you have to implement and
// provide to
// ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void imgui_renderdrawlists(struct ImDrawList **const cmd_lists,
                           int cmd_lists_count) {
  if (cmd_lists_count == 0)
    return;

  // We are using the OpenGL fixed pipeline to make the example code
  // simpler to
  // read!
  // A probable faster way to render would be to collate all vertices
  // from all
  // cmd_lists into a single vertex buffer.
  // Setup render state: alpha-blending enabled, no face culling, no
  // depth
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
#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
  for (int n = 0; n < cmd_lists_count; n++) {
    const ImDrawList *cmd_list = cmd_lists[n];
    const unsigned char *vtx_buffer =
        (const unsigned char *)&cmd_list->vtx_buffer.front();
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                    (void *)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                      (void *)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
                   (void *)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

    int vtx_offset = 0;
    for (size_t cmd_i = 0; cmd_i < cmd_list->commands.size(); cmd_i++) {
      const ImDrawCmd *pcmd = &cmd_list->commands[cmd_i];
      if (pcmd->user_callback) {
        pcmd->user_callback(cmd_list, pcmd);
      } else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t) pcmd->texture_id);
        glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w),
                  (int)(pcmd->clip_rect.z - pcmd->clip_rect.x),
                  (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
        glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
      }
      vtx_offset += pcmd->vtx_count;
    }
  }
#undef OFFSETOF

  // Restore modified state
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindTexture(GL_TEXTURE_2D, 0);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

// NB: ImGui already provide OS clipboard support for Windows so this
// isn't
// needed if you are using Windows only.
static const char *imgui_getclipboardtextfn() {
  return glfwGetClipboardString(window);
}

static void imgui_setclipboardtextfn(const char *text) {
  glfwSetClipboardString(window, text);
}

void load_font_texture(void) {
  ImGuiIO &io = ImGui::GetIO();
  // ImFont* my_font1 = io.Fonts->AddFontDefault();
  // ImFont* my_font2 =
  // io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf",
  // 15.0f);
  // ImFont* my_font3 =
  // io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f);
  // my_font3->DisplayOffset.y += 1;
  // ImFont* my_font4 =
  // io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f);
  // my_font4->DisplayOffset.y += 1;
  // ImFont* my_font5 =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf",
  // 20.0f,
  // io.Fonts->GetGlyphRangesJapanese());

  // ImFont* my_font2 =
  // io.Fonts->AddFontFromFileTTF("../vendor/fonts/anonymous-pro.ttf",
  // 12.0f);

  // Build texture
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  // Create texture
  glGenTextures(1, &font_texture);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
               GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t) font_texture;
}

void unload_font_texture(void) {
  if (font_texture) {
    glDeleteTextures(1, &font_texture);
    ImGui::GetIO().Fonts->TexID = 0;
    font_texture = 0;
  }
}

void gui_init_imgui(void) {
  auto &io = ImGui::GetIO();
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

  io.RenderDrawListsFn = imgui_renderdrawlists;
  io.SetClipboardTextFn = imgui_setclipboardtextfn;
  io.GetClipboardTextFn = imgui_getclipboardtextfn;
#ifdef _MSC_VER
  io.ImeWindowHandle = glfwGetWin32Window(window);
#endif

  // Styling
  auto &style = ImGui::GetStyle();
  style.Alpha = 0.8;
  style.WindowRounding = 6.0;
  style.WindowPadding = {3.0, 3.0};
  style.FrameRounding = 2.0;
  style.FramePadding = {1.0, 1.0};
}

void gui_new_frame(void) {
  if (!font_texture)
    load_font_texture();

  mouse_pressed[0] = mouse_pressed[1] = false;
  glfwPollEvents();
  ImGuiIO &io = ImGui::GetIO();

  // Setup resolution (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(window, &w, &h);
  glfwGetFramebufferSize(window, &display_w, &display_h);
  // Display size, in pixels. For clamping windows positions.
  io.DisplaySize = ImVec2((float)display_w, (float)display_h);

  // Setup time step
  static double time = 0.0f;
  const double current_time = glfwGetTime();
  io.DeltaTime =
      time > 0.0 ? (float)(current_time - time) : (float)(1.0f / 60.0f);
  time = current_time;

  // Setup inputs
  // (we already got mouse wheel, keyboard keys & characters from glfw
  // callbacks
  // polled in glfwPollEvents())
  if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    mouse_x *= (float)display_w / w; // Convert mouse coordinates to pixels
    mouse_y *= (float)display_h / h;
    io.MousePos = ImVec2((float)mouse_x,
                         (float)mouse_y); // Mouse position, in pixels (set to
                                          // -1,-1 if no mouse / on another
                                          // screen, etc.)
  } else {
    io.MousePos = ImVec2(-1, -1);
  }
  for (int i = 0; i < 3; i++) {
    io.MouseDown[i] = mouse_pressed[i] ||
                      glfwGetMouseButton(window, i) !=
                          0; // If a mouse press event came, always
                             // pass it as "mouse held this frame", so
                             // we don't miss click-release events
                             // that are shorter than 1 frame.
    mouse_pressed[i] = false;
  }

  io.MouseWheel = mouse_wheel;
  mouse_wheel = 0.0f;

  // Hide/show hardware mouse cursor
  glfwSetInputMode(window, GLFW_CURSOR, io.MouseDrawCursor
                                            ? GLFW_CURSOR_HIDDEN
                                            : GLFW_CURSOR_NORMAL);

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
  screen_zoom(width, height, zoom, drag_x, drag_y);

  draw_state(*app_state);
  if (DRAW_DECISON) {
    draw_decision(*app_decision_max, *app_state, MAX);
    draw_decision(*app_decision_min, *app_state, MIN);
  }
  if (app_selected_suggestion >= 0 &&
      app_selected_suggestion < app_suggestions->tables_count) {
    draw_suggestion(app_suggestions->tables[app_selected_suggestion]);
  }

  draw_options_window();

  ImGui::Begin("Main");
  // bool opened = true;
  // ImGui::Begin("Main", &opened, ImVec2(0,0), 0.3f,
  // ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  draw_app_status();

  {
    ImGui::PushID(101);
    static char filename[256] = "local.cfg";
    ImGui::InputText("Params file", filename, 256);
    if (ImGui::Button("Save params")) {
      app_save_params(filename);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load params")) {
      app_load_params(filename);
    }
    ImGui::PopID();
  }

  ImGui::Checkbox("PARAM_GROUP_AUTOSELECT", &PARAM_GROUP_AUTOSELECT);
  ImGui::Checkbox("PARAM_GROUP_CONQUER", &PARAM_GROUP_CONQUER);
  ImGui::SliderFloat("PARAM_GROUP_THRESHOLD", &PARAM_GROUP_THRESHOLD, 0.0,
                     10.0);
  ImGui::SliderFloat("PARAM_GROUP_CONQUER_TIME", &PARAM_GROUP_CONQUER_TIME, 0.0,
                     1.0);
  const char *groups[] = {"MAX_ATTACK", "MIN_ATTACK", "MAX_CONQUER",
                          "MIN_CONQUER"};
  if (PARAM_GROUP_AUTOSELECT) {
    ImGui::Text(groups[*PARAM_GROUP]);
  } else {
    static int _PARAM_GROUP = 0;
    ImGui::Combo("PARAM_GROUP", &_PARAM_GROUP, groups,
                 PARAM_GROUP_CONQUER ? 4 : 2);
    set_param_group(_PARAM_GROUP);
  }
  const char *optimizes[] = {"NO_OPTIMIZE", "OPTIMIZE_ALL", "OPTIMIZE_BEST"};
  ImGui::Combo("FINE_OPTIMIZE", (int *)&FINE_OPTIMIZE, optimizes, 3);
  ImGui::End();

  ImGui::Begin("Calibration");
  ImGui::Checkbox("CONSTANT_RATE", &CONSTANT_RATE);
  ImGui::Checkbox("KICK_IF_NO_PASS", &KICK_IF_NO_PASS);
  if (CONSTANT_RATE)
    ImGui::SliderInt("DECISION_RATE", &DECISION_RATE, 1, 1000);
  else
    ImGui::SliderInt("RAMIFICATION_NUMBER", &RAMIFICATION_NUMBER,
                     MAX_SUGGESTIONS + 2, 20000);
  ImGui::SliderInt("FULL_CHANGE_PERCENTAGE", &FULL_CHANGE_PERCENTAGE, 0, 100);
  ImGui::SliderInt("MAX_DEPTH", &MAX_DEPTH, 0, 3);

#define SLIDER(V, S, A, B) ImGui::DragFloat(#V, &V, S, A, B)
  SLIDER(KICK_POS_VARIATION, 0.01, 0.0, 1.0);
  SLIDER(MIN_GAP_TO_KICK, 1.00, 0, 180);
  SLIDER(DESIRED_PASS_DIST, 0.1, 0, 10);
  SLIDER(WEIGHT_BALL_POS, 1.0, 0, 5000);
  SLIDER(WEIGHT_MOVE_DIST_TOTAL, 1.0, 0, 10);
  SLIDER(WEIGHT_MOVE_DIST_MAX, 1.0, 0, 10);
  SLIDER(WEIGHT_MOVE_CHANGE, 10.0, 0, 5000);
  SLIDER(WEIGHT_PASS_CHANGE, 10.0, 0, 5000);
  SLIDER(WEIGHT_KICK_CHANGE, 10.0, 0, 5000);
  SLIDER(TOTAL_MAX_GAP_RATIO, 0.01, 0, 1);
  SLIDER(WEIGHT_CLOSE_TO_BALL, 10.0, 0, 5000);
  SLIDER(WEIGHT_ENEMY_CLOSE_TO_BALL, 10.0, 0, 5000);
  SLIDER(WEIGHT_HAS_BALL, 10.0, 0, 5000);
  SLIDER(WEIGHT_ATTACK, 10.0, 0, 5000);
  SLIDER(WEIGHT_SEE_ENEMY_GOAL, 1.0, 0, 5000);
  SLIDER(WEIGHT_BLOCK_GOAL, 1.0, 0, 5000);
  SLIDER(WEIGHT_BLOCK_ATTACKER, 10.0, 0, 5000);
  SLIDER(WEIGHT_GOOD_RECEIVERS, 1.0, 0, 5000);
  SLIDER(WEIGHT_RECEIVERS_NUM, 1.0, 0, 5000);
  SLIDER(WEIGHT_ENEMY_RECEIVERS_NUM, 1.0, 0, 5000);
  SLIDER(DIST_GOAL_PENAL, 10.0, 0, 5000);
  SLIDER(DIST_GOAL_TO_PENAL, 0.1, 0, 6);
  SLIDER(MOVE_RADIUS_0, 0.05, 0, 10);
  SLIDER(MOVE_RADIUS_1, 0.10, 0, 10);
  SLIDER(MOVE_RADIUS_2, 0.10, 0, 10);
#undef SLIDER
  ImGui::End();

  ImGui::Begin("Suggestions");

  {
    ImGui::PushID(102);
    static char filename[256] = "suggestions.cfg";
    ImGui::InputText("Suggestions file", filename, 256);
    if (ImGui::Button("Save suggestions")) {
      save_suggestions(*app_suggestions, filename);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load suggestions")) {
      load_suggestions(*app_suggestions, filename);
    }
    ImGui::PopID();
  }

  int *e = &app_selected_suggestion;
  ImGui::RadioButton("(None)", e, -1);
  FOR_N(i, app_suggestions->tables_count) {
    auto &table = app_suggestions->tables[i];
    int usage = table.usage_count;
    ImGui::PushID(10000 + i);
    ImGui::RadioButton(table.name, e, i);
    ImGui::SameLine();
    if (i == app_suggestions->last_used) {
      ImGui::TextColored(ImColor(255, 0, 0, 255), "[%i]", usage);
    } else {
      ImGui::Text("[%i]", usage);
    }
    ImGui::SameLine();
    if (ImGui::Button("delete")) {
      del_suggestion(*app_suggestions, i);
    }
    ImGui::PopID();
  }
  static char sname[256] = "";
  ImGui::InputText("", sname, 256);
  ImGui::SameLine();
  if (ImGui::Button("add")) {
    int i = add_suggestion(*app_suggestions) - 1;
    strcpy(app_suggestions->tables[i].name, sname);
    sname[0] = '\0';
    *e = i;
  }
  double x, y;
  x = screen_xpos;
  y = screen_ypos;
  ImGui::Text("screen pos: %f, %f", x, y);
  ImGui::End();

  ImGui::Render();
  glfwSwapBuffers(window);
}

bool gui_should_close(void) { return glfwWindowShouldClose(window); }

void gui_shutdown(void) {
  unload_font_texture();
  glfwDestroyWindow(window);
  glfwTerminate();
}
