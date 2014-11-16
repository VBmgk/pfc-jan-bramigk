#include <csignal>
#include <iostream>
#include "app.h"

static bool should_wait = true;

void signal_handler(int signal) {
  should_wait = false;
  std::cout << "\rGoodbye!" << std::endl;
}

int main(int argc, char **argv) {
  std::signal(SIGINT, signal_handler);

  App::run([&](App &app_) {
    while (should_wait)
      ;
  });

  exit(EXIT_SUCCESS);
}
