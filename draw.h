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

void draw_character(uint16_t dx, uint16_t dy, uint32_t c, px_t color, uint16_t scl);

void draw_load_font();
void draw_free_font();
void draw_string_u32(uint16_t dx, uint16_t dy, uint32_t* str, px_t color, uint16_t scl);
void draw_string_u32_offset(uint16_t dx, uint16_t dy, uint32_t* str, px_t color, uint16_t scl, uint16_t offset);
