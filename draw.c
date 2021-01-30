#include <stdint.h>
#include "ui.h"
#include <math.h>

/* idea: https://www.thecrazyprogrammer.com/2017/02/flood-fill-algorithm-in-c.html */

void draw_fill(int x, int y, px_t color)
{
  px_t xcolor = ui_px_get(x,y);
	if (xcolor != 0xffffffff && xcolor != color) {
		ui_px_put(x,y,color);
		draw_fill(x+1,y,color);
		draw_fill(x,y+1,color);
		draw_fill(x-1,y,color);
		draw_fill(x,y-1,color);
	}
}

/* source: https://gist.github.com/bert/1085538 */

void draw_line(int x0, int y0, int x1, int y1, px_t color)
{
  int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2; /* error value e_xy */
 
  for (;;) {
    ui_px_put(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

void draw_rect_fill(int x0, int y0, int x1, int y1, px_t color) {
  for (int y=y0; y<=y1; y++) {
    for (int x=x0; x<=x1; x++) {
      ui_px_put(x, y, color);
    }
  }
}
