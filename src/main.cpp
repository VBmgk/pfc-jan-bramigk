#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "gui.h"

int main(int argc, char **argv) {

  // TODO: parameter to disable gui maybe?

  bool play_as_max = true;
  if (argc > 1 && strcmp(argv[1], "--min") == 0) {
    play_as_max = false;
  }

  printf("Playing as %s.\n", play_as_max ? "blue" : "yellow");

  app_run([&]() {

            gui_init();

            while (!gui_should_close()) {
              gui_new_frame();
              gui_sync();
              gui_render();
            }

            gui_shutdown();
            printf("\rGood");
          },
          play_as_max);

  printf("bye!\n");
  return EXIT_SUCCESS;
}
