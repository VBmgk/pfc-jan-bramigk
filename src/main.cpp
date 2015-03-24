#include <stdlib.h>

#include "app.h"
#include "gui.h"

int main(int argc, char **argv) {

  // TODO: parameter to disable gui maybe?

  app_run([&]() {

    gui_init();

    while (!gui_should_close()) {
      gui_update();
      gui_sync();
      gui_render();
    }

    gui_destroy();
    printf("\rGood");
  });

  printf("bye!\n");
  return EXIT_SUCCESS;
}
