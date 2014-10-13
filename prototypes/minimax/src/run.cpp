#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>

#include <zmq.hpp>

#include "minimax.h"
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

  std::thread count_thread([&]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int n_ticks = 0;
    while (should_recv) {
      int count = req_count.exchange(0);
      {
        std::lock_guard<std::mutex> _(app.display_mutex);
        app.display.uptime = ++n_ticks;
        app.display.pps = count;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

  std::thread zmq_thread([&]() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    static const char *addr = "tcp://*:5555";
    socket.bind(addr);
    int timeout = 100; // ms
    socket.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof timeout);
    socket.setsockopt(ZMQ_SNDTIMEO, &timeout, sizeof timeout);

    zmq::message_t buffer(1024);
    std::string data;

    std::cout << "listening on " << addr << std::endl;

    while (should_recv) {
      try {
        if (socket.recv(&buffer)) {
          std::string buffer_str((char *)buffer.data(), buffer.size());
          req_count++;
          roboime::Update u;
          u.ParseFromString(buffer_str);

          Team min_t(MIN), max_t(MAX);
          for (int i = 0; i < u.min_team_size(); i++) {
            const ::roboime::Robot &r = u.min_team(i);
            min_t.addRobot(Robot(r.i(), Vector(r.x(), r.y()), Vector(r.vx(), r.vy())));
          }
          for (int i = 0; i < u.max_team_size(); i++) {
            const ::roboime::Robot &r = u.max_team(i);
            max_t.addRobot(Robot(r.i(), Vector(r.x(), r.y()), Vector(r.vx(), r.vy())));
          }
          Ball ball(Vector(u.ball().x(), u.ball().y()), Vector(u.ball().vx(), u.ball().vy()));
          Board local_board(min_t, max_t, ball);

          {
            std::lock_guard<std::mutex> _(app.board_mutex);
            app.board = local_board;
          }

          zmq::message_t command_message(data.length());
          memcpy((void *)command_message.data(), data.c_str(), data.length());
          socket.send(command_message);
        }
      } catch (zmq::error_t e) {
        std::cerr << "error" << std::endl;
      }
    }
  });

  run(app);

  should_recv = false;
  zmq_thread.join();
  count_thread.join();
}
