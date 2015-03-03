#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>

#include <zmq.hpp>

#include "minimax.h"
#include "app.h"
#include "discrete.pb.h"
#include "update.pb.h"
#include "timer.h"

void App::run(std::function<void(App &)> run) {
  App app;

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
        std::lock_guard<std::mutex> _(app.display_mutex);
        app.display.uptime = ++n_ticks;
        app.display.pps = count;
        app.display.mps = mcount;
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
    static const char *addr = "tcp://*:5555";
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
          ::roboime::Update u;
          u.ParseFromString(buffer_str);

          // now that we've got the parsed data,
          // we'll start by populating two teams to build our board
          Team min_t, max_t;
          for (int i = 0; i < u.min_team_size(); i++) {
            const ::roboime::Robot &r = u.min_team(i);
            // yeah the line breaks there, autoformatter does that
            // and it's fine, don't cry over it
            min_t.addRobot(
                Robot(r.i(), Vector(r.x(), r.y()), Vector(r.vx(), r.vy())));
          }
          for (int i = 0; i < u.max_team_size(); i++) {
            const ::roboime::Robot &r = u.max_team(i);
            max_t.addRobot(
                Robot(r.i(), Vector(r.x(), r.y()), Vector(r.vx(), r.vy())));
          }
          // don't forget the ball
          Ball ball(Vector(u.ball().x(), u.ball().y()),
                    Vector(u.ball().vx(), u.ball().vy()));

          // now we assemble the board
          Board local_board(min_t, max_t, ball);
          {
            // this is the critical section, where we atomically
            // switch the app board with our freshly built one
            std::lock_guard<std::mutex> _(app.board_mutex);
            app.display.has_val = false;
            app.board = local_board;
          }

          // update done, time to reply that request, remember?
          // just like above we'll atomically copy the latest command
          // staright from the app, we don't want it to change while
          // iterating over it
          TeamAction local_command;
          {
            std::lock_guard<std::mutex> _(app.command_mutex);
            local_command = app.command;
          }

          // another important part, we'll assemble the protobuf command packet
          ::roboime::Command command;
          for (auto robot_action : local_command) {
            ::roboime::Action *action = command.add_action();
            // don't worry, each action knows how to generate itself:
            robot_action.discreteAction(action);
          }

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

  std::thread minimax_thread([&]() {
    // single minimax instance, may make use of cache in the future
    Minimax minimax;
    TeamAction local_command, local_enemy_command;
    Board local_board;

    int n_ticks = 0;
    while (should_recv) {
      {
        std::lock_guard<std::mutex> _(app.board_mutex);
        app.command_board = app.board;
        local_board = app.command_board;
      }

      if (app.play_minimax || app.play_minimax_once) {
        app.play_minimax_once = false;
        float val;
        if (app.use_experimental) {
          std::tie(val, local_command, local_enemy_command) =
              minimax.decision_experimental(local_board);
        } else {
          std::tie(val, local_command, local_enemy_command) =
              minimax.decision_value(local_board);
        }
        mnmx_count++;
        app.display.minimax_count++;
        app.display.minimax_val = val;
      }

      if (app.eval_board_once) {
        app.eval_board_once = false;
        app.display.val = local_board.evaluate();
        app.display.has_val = true;
      }

      {
        std::lock_guard<std::mutex> _(app.command_mutex);
        app.command = local_command;
        app.enemy_command = local_enemy_command;
      }

      n_ticks++;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  run(app);

  should_recv = false;
  count_thread.join();
  minimax_thread.join();
  zmq_thread.join();
}
