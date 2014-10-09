#include <csignal>
#include <iostream>
#include "minimax.h"

static bool should_wait = true;

void signal_handler(int signal) {
  should_wait = false;
  std::cout << std::endl << "Goodbye!" << std::endl;
}

int main(int argc, char** argv) {
  std::signal(SIGINT, signal_handler);

  Minimax::run_minimax([&] (Board& board_, std::mutex& board_mutex_) {
    while (should_wait);
  });

  exit(EXIT_SUCCESS);
}
