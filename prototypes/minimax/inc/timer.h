#ifndef MINIMAX_TIMER_H
#define MINIMAX_TIMER_H
#include <chrono>

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

#endif
