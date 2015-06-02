#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>
#include <mutex>

#include <imgui.h>
#include <zmq.hpp>
#include "discrete.pb.h"
#include "update.pb.h"

#include "app.h"
#include "state.h"
#include "decision.h"
#include "player.h"
#include "consts.h"
#include "optimization.h"
#include "minimax.h"
#include "draw.h"
#include "utils.h"
#include "id_table.h"
#include "suggestions.h"
#include "decision_source.h"

static std::mutex state_mutex, decision_mutex, display_mutex;
static Decision decision_min, decision_max;
static State state, command_state;
static Optimization optimization;
static IdTable id_table;
static Suggestions suggestions;

const State *app_state = &state;
const struct Decision *app_decision_max = &decision_max;
const struct Decision *app_decision_min = &decision_min;
const struct DecisionTable *app_decision_table = &optimization.table;
struct Suggestions *app_suggestions = &suggestions;

DecisionSource decision_source = NO_SOURCE;
DecisionSource *app_decision_source = &decision_source;

static struct {
  int uptime = 0;
  int decision_count = 0;
  int pps = 0;
  int mps = 0;
  float rpd = 0.0;
  float val = 0.0;
  float vals[W_SIZE] = {};
  bool has_val = false;
  float decision_val = 0.0;
} display;

static bool play_minimax = false, play_decision_once = false, eval_state = true,
            eval_state_once = false, use_experimental = false;

static bool ball_selected = false;
static int selected_robot = 0;
const int *app_selected_robot = &selected_robot;
int app_selected_suggestion = -1;

// TODO: maybe persist these
static constexpr int save_slots = 10;
static State save_states[save_slots];
static int save_slot = 0;

static constexpr float move_step = 0.05;

struct Timer {
  Timer() { reset(); }

  double elapsed() {
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
  }

  void reset() { start = std::chrono::system_clock::now(); }

  std::chrono::time_point<std::chrono::system_clock> start, end;
};

void _update_param_group_2(void) {
  auto ball = state.ball;
  if (ball.x < -PARAM_GROUP_THRESHOLD) {
    set_param_group(MIN_ATTACK);
  }
  if (ball.x > +PARAM_GROUP_THRESHOLD) {
    set_param_group(MAX_ATTACK);
  }
}

void _update_param_group_4(void) {
  auto ball = state.ball;
  float time_min, time_max;
  robot_with_ball(state, &time_min, &time_max);
  if (ball.x < 0) {
    if (time_max < PARAM_GROUP_CONQUER_TIME)
      set_param_group(MAX_CONQUER);
    else if (ball.x < -PARAM_GROUP_THRESHOLD)
      set_param_group(MIN_ATTACK);
  } else {
    if (time_min < PARAM_GROUP_CONQUER_TIME)
      set_param_group(MIN_CONQUER);
    else if (ball.x > +PARAM_GROUP_THRESHOLD)
      set_param_group(MAX_ATTACK);
  }
}

void update_param_group(void) {
  if (PARAM_GROUP_AUTOSELECT) {
    if (PARAM_GROUP_CONQUER)
      _update_param_group_4();
    else
      _update_param_group_2();
  }
}

void app_run(std::function<void(void)> loop_func, bool play_as_max) {
  state = uniform_rand_state();
  update_param_group();

  // Timer tmr;
  bool should_recv(true);

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // for counting received messages
  std::atomic<int> req_count(0);
  std::atomic<int> dec_count(0);
  std::atomic<int> tram_count(0);

  std::thread count_thread([&]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int n_ticks = 0;
    while (should_recv) {
      int count = req_count.exchange(0);
      int dcount = dec_count.exchange(0);
      int rcount = tram_count.exchange(0);
      {
        std::lock_guard<std::mutex> _(display_mutex);
        display.uptime = ++n_ticks;
        display.pps = count;
        display.mps = dcount;
        display.rpd = ((float)rcount) / dcount;
      }
      if (CONSTANT_RATE) {
        if (dcount > 0)
          RAMIFICATION_NUMBER = rcount / dcount;
      } else {
        DECISION_RATE = dcount;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  // this is the communication thread
  std::thread zmq_thread([&]() {

    // set up the ZeroMQ context and create a Reply socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    // we will listen on any interface at port 5555
    static const char *addr = play_as_max ? "tcp://*:5555" : "tcp://*:5556";
    socket.bind(addr);

    // this is importante to avoid blocking the whole process
    // when a request is not received
    int timeout = 100; // ms
    socket.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof timeout);
    socket.setsockopt(ZMQ_SNDTIMEO, &timeout, sizeof timeout);

    // we'll need a buffer to read and write data to
    zmq::message_t buffer(1024);
    std::string data;

    // if we got so far, let's at least make that clear
    std::cout << "listening on " << addr << std::endl;

    // ok, loop until told to stop (by the main thread)
    while (should_recv) {
      // yeah, let's go for some robustness, aka hide the dirt
      try {
        // in case of timeout the return won't be true
        if (socket.recv(&buffer)) {
          // quick save it on a buffer, you never know right
          std::string buffer_str((char *)buffer.data(), buffer.size());
          // we're stat maniac, count up the number of requests
          req_count++;

          // ok, time to parse that data
          UpdateMessage update;
          update.ParseFromString(buffer_str);

          {
            // critical section to update the global state
            std::lock_guard<std::mutex> _(state_mutex);
            update_from_proto(state, update, id_table);
            update_param_group();
            // display.has_val = false;
          }

          // update done, time to reply that request, remember?
          // just like above we'll atomically copy the latest decision
          // staright from the app, we don't want it to change while
          // iterating over it
          Decision local_decision;
          {
            std::lock_guard<std::mutex> _(decision_mutex);
            if (play_as_max)
              local_decision = decision_max;
            else
              local_decision = decision_min;
          }

          // another important part, we'll assemble the protobuf command
          // packet
          CommandMessage command;
          to_proto_command(local_decision, play_as_max ? MAX : MIN, command,
                           id_table);

          // now let's serialize and shove it on our message buffer
          command.SerializeToString(&data);
          zmq::message_t command_message(data.length());
          memcpy((void *)command_message.data(), data.c_str(), data.length());

          // finally, reply:
          socket.send(command_message);
        }
      } catch (zmq::error_t e) {
        // what? an error?? what's that???
        std::cerr << "error" << std::endl;
      }
    }
  });

  std::thread decision_thread([&]() {
    // single minimax instance, may make use of cache in the future
    // Minimax minimax;
    State local_state;
    Decision local_decision_max, local_decision_min;

    int n_ticks = 0;
    while (should_recv) {
      {
        std::lock_guard<std::mutex> _(state_mutex);
        command_state = state;
        local_state = command_state;
      }

      if (play_minimax || play_decision_once) {
        play_decision_once = false;
        int ram_count = 0;
        float val;

        if (MAX_DEPTH == 0) {
          // optimization decision
          if (play_as_max) {
            auto valued_decision = decide(optimization, local_state, MAX,
                                          &suggestions, &ram_count);
            local_decision_max = valued_decision.decision;
            val = valued_decision.value;
          } else {
            auto valued_decision = decide(optimization, local_state, MIN,
                                          &suggestions, &ram_count);
            local_decision_min = valued_decision.decision;
            val = valued_decision.value;
          }
        } else {
          // TODO: minimax decision
          val = 0.0;
        }

        dec_count++;
        tram_count += ram_count;
        display.decision_count++;
        display.decision_val = val;
      }

      if (true || eval_state || eval_state_once) {
        eval_state_once = false;
        if (MAX_DEPTH == 0) {
          FOR_N(i, W_SIZE) display.vals[i] = 0.0;
          display.val = evaluate_with_decision(
              play_as_max ? MAX : MIN, local_state,
              play_as_max ? local_decision_max : local_decision_min,
              optimization.table, display.vals);
        } else {
          // TODO: use minimax decision table
        }
        display.has_val = true;
      }

      {
        std::lock_guard<std::mutex> _(decision_mutex);
        decision_max = local_decision_max;
        decision_min = local_decision_min;
      }

      n_ticks++;
      // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  loop_func();

  should_recv = false;
  count_thread.join();
  decision_thread.join();
  zmq_thread.join();
}

void app_random() {
  std::lock_guard<std::mutex> _(state_mutex);
  state = uniform_rand_state();
  update_param_group();
  display.has_val = false;
}

void app_decide_once() { play_decision_once = true; }

void app_decide_toggle() { play_minimax = !play_minimax; }

void app_eval_once() { eval_state_once = true; }

void app_eval_toggle() { eval_state = !eval_state; }

void app_apply() {
  std::lock_guard<std::mutex> _(state_mutex);
  apply_to_state(decision_max, MAX, &state);
  apply_to_state(decision_min, MIN, &state);
  update_param_group();
}

void app_toggle_experimental() { use_experimental = !use_experimental; }

void app_select_save_slot(int slot) { save_slot = slot % save_slots; }

void app_load_state() {
  std::lock_guard<std::mutex> _(state_mutex);
  state = save_states[save_slot];
  update_param_group();
}

void app_save_state() { save_states[save_slot] = state; }

void app_toggle_selected_player() {
  selected_robot = (selected_robot / N_ROBOTS + 1) % 2 * N_ROBOTS +
                   (selected_robot + 1) % N_ROBOTS;
}

void app_select_next_robot() {
  selected_robot =
      (selected_robot / N_ROBOTS) * N_ROBOTS + (selected_robot + 1) % N_ROBOTS;
}

void app_select_ball() {
  // select ball
  ball_selected = ball_selected ? false : true;
}

#define MOVE(D, V)                                                             \
  void app_move_##D() {                                                        \
    if (selected_robot >= 0) {                                                 \
      if (ball_selected == true)                                               \
        state.ball += V;                                                       \
      else                                                                     \
        state.robots[selected_robot] += V;                                     \
    }                                                                          \
    update_param_group();                                                      \
  }
MOVE(up, Vector(0, move_step))
MOVE(down, Vector(0, -move_step))
MOVE(right, Vector(move_step, 0))
MOVE(left, Vector(-move_step, 0))
#undef MOVE

void draw_app_status(void) {
  std::lock_guard<std::mutex> _(display_mutex);
  ImGui::Text("uptime: %is", display.uptime);
  ImGui::Text("decision: #%i", display.decision_count);
  ImGui::Text("%i packets/s", display.pps);
  if (CONSTANT_RATE)
    ImGui::Text("%.2f ramifications/decision", display.rpd);
  else
    ImGui::Text("%i decisions/s", display.mps);
  ImGui::Text("decided val: %f", display.decision_val);
  if (display.has_val)
    ImGui::Text("current val: %f", display.val);
  switch (decision_source) {
  case SUGGESTION:
    ImGui::Text("decision from suggestion");
    break;
  case TABLE:
    ImGui::Text("decision from table");
    break;
  case FULL_RANDOM:
    ImGui::Text("decision from full random");
    break;
  case SINGLE_RANDOM:
    ImGui::Text("decision from single random");
    break;
  default:
  case NO_SOURCE:
    ImGui::Text("no decision source");
    break;
  }
#define SHOW_VAR(NAME) ImGui::Text(#NAME ": %f", display.vals[_##NAME]);
  SHOW_VAR(WEIGHT_BALL_POS);
  SHOW_VAR(WEIGHT_MOVE_DIST_MAX);
  SHOW_VAR(WEIGHT_MOVE_DIST_TOTAL);
  SHOW_VAR(WEIGHT_MOVE_CHANGE);
  SHOW_VAR(WEIGHT_PASS_CHANGE);
  SHOW_VAR(WEIGHT_KICK_CHANGE);
  SHOW_VAR(WEIGHT_CLOSE_TO_BALL);
  SHOW_VAR(WEIGHT_ENEMY_CLOSE_TO_BALL);
  SHOW_VAR(WEIGHT_HAS_BALL);
  SHOW_VAR(WEIGHT_ATTACK);
  SHOW_VAR(WEIGHT_SEE_ENEMY_GOAL);
  SHOW_VAR(WEIGHT_BLOCK_GOAL);
  SHOW_VAR(WEIGHT_BLOCK_ATTACKER);
  SHOW_VAR(WEIGHT_GOOD_RECEIVERS);
  SHOW_VAR(WEIGHT_RECEIVERS_NUM);
  SHOW_VAR(WEIGHT_ENEMY_RECEIVERS_NUM);
  SHOW_VAR(WEIGHT_PENALS);
#undef SHOW_VAR
}

#define PARAMS_FILE_HEADER "[AI params version 1]"
void app_save_params(const char *filename) {
  auto file = fopen(filename, "w");
  if (!file) {
    perror("Could not save params");
    return;
  }
  fprintf(file, PARAMS_FILE_HEADER "\n");
#define SEP " = "
#define SAVE_PARAM(MODE, NAME) fprintf(file, #NAME SEP MODE "\n", NAME)
  SAVE_PARAM("%i", CONSTANT_RATE);
  SAVE_PARAM("%i", KICK_IF_NO_PASS);
  if (CONSTANT_RATE)
    SAVE_PARAM("%i", DECISION_RATE);
  else
    SAVE_PARAM("%i", RAMIFICATION_NUMBER);
  SAVE_PARAM("%i", FULL_CHANGE_PERCENTAGE);
  SAVE_PARAM("%i", MAX_DEPTH);
  SAVE_PARAM("%f", KICK_POS_VARIATION);
  SAVE_PARAM("%f", MIN_GAP_TO_KICK);
  SAVE_PARAM("%f", DESIRED_PASS_DIST);
  SAVE_PARAM("%f", WEIGHT_BALL_POS);
  SAVE_PARAM("%f", WEIGHT_MOVE_DIST_MAX);
  SAVE_PARAM("%f", WEIGHT_MOVE_DIST_TOTAL);
  SAVE_PARAM("%f", WEIGHT_MOVE_CHANGE);
  SAVE_PARAM("%f", WEIGHT_PASS_CHANGE);
  SAVE_PARAM("%f", WEIGHT_KICK_CHANGE);
  SAVE_PARAM("%f", TOTAL_MAX_GAP_RATIO);
  SAVE_PARAM("%f", WEIGHT_CLOSE_TO_BALL);
  SAVE_PARAM("%f", WEIGHT_ENEMY_CLOSE_TO_BALL);
  SAVE_PARAM("%f", WEIGHT_HAS_BALL);
  SAVE_PARAM("%f", WEIGHT_ATTACK);
  SAVE_PARAM("%f", WEIGHT_SEE_ENEMY_GOAL);
  SAVE_PARAM("%f", WEIGHT_BLOCK_GOAL);
  SAVE_PARAM("%f", WEIGHT_BLOCK_ATTACKER);
  SAVE_PARAM("%f", WEIGHT_GOOD_RECEIVERS);
  SAVE_PARAM("%f", WEIGHT_RECEIVERS_NUM);
  SAVE_PARAM("%f", WEIGHT_ENEMY_RECEIVERS_NUM);
  SAVE_PARAM("%f", DIST_GOAL_PENAL);
  SAVE_PARAM("%f", DIST_GOAL_TO_PENAL);
  SAVE_PARAM("%f", MOVE_RADIUS_0);
  SAVE_PARAM("%f", MOVE_RADIUS_1);
  SAVE_PARAM("%f", MOVE_RADIUS_2);
#undef SAVE_PARAM
#undef SEP
  fclose(file);
}

void app_load_params(const char *filename) {
  auto file = fopen(filename, "r");
  if (!file) {
    perror("Could not load params");
    return;
  }
  int CONSTANT_RATE;
  int KICK_IF_NO_PASS;

  char line[256];
  fgets(line, 256, file);
  if (strcmp(line, PARAMS_FILE_HEADER "\n")) {
    fprintf(stderr, "Incompatible header detected, maybe newer or invalid.\n");
    goto out;
  }

  // TODO: warn duplicates, warn missing, warn extraneous

  while (!feof(file)) {
    fgets(line, 256, file);
#define SEP " = "
#define LOAD_PARAM(MODE, NAME)                                                 \
  if (!strncmp(line, #NAME, strlen(#NAME)))                                    \
  sscanf(line, "%*s" SEP MODE, &NAME)
    LOAD_PARAM("%i", CONSTANT_RATE);
    LOAD_PARAM("%i", KICK_IF_NO_PASS);
    LOAD_PARAM("%i", DECISION_RATE);
    LOAD_PARAM("%i", RAMIFICATION_NUMBER);
    LOAD_PARAM("%i", FULL_CHANGE_PERCENTAGE);
    LOAD_PARAM("%i", MAX_DEPTH);
    LOAD_PARAM("%f", KICK_POS_VARIATION);
    LOAD_PARAM("%f", MIN_GAP_TO_KICK);
    LOAD_PARAM("%f", DESIRED_PASS_DIST);
    LOAD_PARAM("%f", WEIGHT_BALL_POS);
    LOAD_PARAM("%f", WEIGHT_MOVE_DIST_MAX);
    LOAD_PARAM("%f", WEIGHT_MOVE_DIST_TOTAL);
    LOAD_PARAM("%f", WEIGHT_MOVE_CHANGE);
    LOAD_PARAM("%f", WEIGHT_PASS_CHANGE);
    LOAD_PARAM("%f", WEIGHT_KICK_CHANGE);
    LOAD_PARAM("%f", TOTAL_MAX_GAP_RATIO);
    LOAD_PARAM("%f", WEIGHT_CLOSE_TO_BALL);
    LOAD_PARAM("%f", WEIGHT_ENEMY_CLOSE_TO_BALL);
    LOAD_PARAM("%f", WEIGHT_HAS_BALL);
    LOAD_PARAM("%f", WEIGHT_ATTACK);
    LOAD_PARAM("%f", WEIGHT_SEE_ENEMY_GOAL);
    LOAD_PARAM("%f", WEIGHT_BLOCK_GOAL);
    LOAD_PARAM("%f", WEIGHT_BLOCK_ATTACKER);
    LOAD_PARAM("%f", WEIGHT_GOOD_RECEIVERS);
    LOAD_PARAM("%f", WEIGHT_RECEIVERS_NUM);
    LOAD_PARAM("%f", WEIGHT_ENEMY_RECEIVERS_NUM);
    LOAD_PARAM("%f", DIST_GOAL_PENAL);
    LOAD_PARAM("%f", DIST_GOAL_TO_PENAL);
    LOAD_PARAM("%f", MOVE_RADIUS_0);
    LOAD_PARAM("%f", MOVE_RADIUS_1);
    LOAD_PARAM("%f", MOVE_RADIUS_2);
#undef LOAD_PARAM
#undef SEP
  }
  ::CONSTANT_RATE = CONSTANT_RATE;
  ::KICK_IF_NO_PASS = KICK_IF_NO_PASS;

out:
  fclose(file);
}
