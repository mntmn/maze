#include "ui.h"

typedef struct {
	int32_t	a[2];
	int32_t	b[2];
	int32_t	c[2];
} triangle_t;

void draw_line(int x0, int y0, int x1, int y1, px_t color);
void draw_fill(int x, int y, px_t color);
void draw_rect_fill(int x0, int y0, int x1, int y1, px_t color);
void draw_tri_flat(triangle_t *d, px_t color);
