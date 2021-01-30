#include <stdint.h>
#include "platform.h"

typedef struct input {
  uint8_t mouse_buttons;
  int16_t mouse_x;
  int16_t mouse_y;
  int16_t keycode;
} input_t;

void ui_px_put(uint16_t x, uint16_t y, px_t color);
void ui_loop_pre(input_t* input);
void ui_loop_post();
int ui_init(char* title);
void ui_exit();
