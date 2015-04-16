#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>

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

static std::mutex state_mutex, decision_mutex, display_mutex;
static Decision decision_min, decision_max;
static State state, command_state;
static Optimization optimization;
static IdTable id_table;

const State *app_state = &state;
const struct Decision *app_decision_max = &decision_max;
const struct Decision *app_decision_min = &decision_min;
const struct DecisionTable *app_decision_table = &optimization.table;

static struct {
  int uptime = 0;
  int decision_count = 0;
  int pps = 0;
  int mps = 0;
  float val = 0.0;
  bool has_val = false;
  float decision_val = 0.0;
} display;

static bool play_minimax = false, play_decision_once = false, eval_state = false, eval_state_once = false,
            use_experimental = false;

static int selected_robot = 0;
const int *app_selected_robot = &selected_robot;

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

void app_run(std::function<void(void)> loop_func, bool play_as_max) {
  state = uniform_rand_state();

  // Timer tmr;
  bool should_recv(true);

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // for counting received messages
  std::atomic<int> req_count(0);
  std::atomic<int> mnmx_count(0);

  std::thread count_thread([&]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int n_ticks = 0;
    while (should_recv) {
      int count = req_count.exchange(0);
      int mcount = mnmx_count.exchange(0);
      {
        std::lock_guard<std::mutex> _(display_mutex);
        display.uptime = ++n_ticks;
        display.pps = count;
        display.mps = mcount;
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
    static const char *addr = play_as_max? "tcp://*:5555" : "tcp://*:5556";
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
          ::roboime::Update update;
          update.ParseFromString(buffer_str);

          {
            // critical section to update the global state
            std::lock_guard<std::mutex> _(state_mutex);
            update_from_proto(state, update, id_table);
            display.has_val = false;
          }

          // update done, time to reply that request, remember?
          // just like above we'll atomically copy the latest decision
          // staright from the app, we don't want it to change while
          // iterating over it
          Decision local_decision;
          {
            std::lock_guard<std::mutex> _(decision_mutex);
            if (play_as_max) local_decision = decision_max;
            else local_decision = decision_min;
          }

          // another important part, we'll assemble the protobuf command packet
          ::roboime::Command command;
          to_proto_command(local_decision, play_as_max? MAX : MIN, command, id_table);

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
        float val;

        if (MAX_DEPTH == 0) {
          // optimization decision
          if (play_as_max) {
            auto valued_decision = decide(optimization, local_state, MAX);
            local_decision_max = valued_decision.decision;
            val = valued_decision.value;
          } else {
            auto valued_decision = decide(optimization, local_state, MIN);
            local_decision_min = valued_decision.decision;
            val = valued_decision.value;
          }
        } else {
          // TODO: minimax decision
        }

        mnmx_count++;
        display.decision_count++;
        display.decision_val = val;
      }

      if (eval_state || eval_state_once) {
        eval_state_once = false;
        if (MAX_DEPTH == 0) {
          display.val = evaluate_with_decision(play_as_max? MAX : MIN, local_state, local_decision_max, optimization.table);
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
}

void app_toggle_experimental() { use_experimental = !use_experimental; }

void app_select_save_slot(int slot) { save_slot = slot % save_slots; }

void app_load_state() {
  std::lock_guard<std::mutex> _(state_mutex);
  state = save_states[save_slot];
}

void app_save_state() { save_states[save_slot] = state; }

void app_toggle_selected_player() {
  selected_robot = (selected_robot / N_ROBOTS + 1) % 2 * N_ROBOTS + (selected_robot + 1) % N_ROBOTS;
}

void app_select_next_robot() {
  selected_robot = (selected_robot / N_ROBOTS) * N_ROBOTS + (selected_robot + 1) % N_ROBOTS;
}

#define MOVE(D, V)                                                                                                     \
  void app_move_##D() {                                                                                                \
    if (selected_robot >= 0)                                                                                           \
      state.robots[selected_robot] += V;                                                                               \
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
  ImGui::Text("%i decisions/s", display.mps);
  ImGui::Text("decision: %f", display.decision_val);
  if (display.has_val)
    ImGui::Text("value: %f", display.val);
}
