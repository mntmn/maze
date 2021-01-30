#include <stdio.h>
#include <stdint.h>

#include "platform.h"
#include "ui.h"

int main(int argc, char** argv) {
  uint8_t running = 1;
  input_t input;
  ui_init("MAZE");
  
  while (running) {
    ui_loop_pre(&input);

    if (input.keycode == 27) {
      running = 0;
    }
    
    ui_loop_post();
  }

  ui_exit();
}
