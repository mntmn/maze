#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include "ui.h"
#include "draw.h"
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
  if (x0<0) x0=0;
  if (x1<0) x1=0;
  if (y0<0) y0=0;
  if (y1<0) y1=0;
  if (x0>=SCREEN_W) x0=SCREEN_W-1;
  if (x1>=SCREEN_W) x1=SCREEN_W-1;
  if (y0>=SCREEN_H) y0=SCREEN_H-1;
  if (y1>=SCREEN_H) y1=SCREEN_H-1;
  
  for (int y=y0; y<=y1; y++) {
    for (int x=x0; x<=x1; x++) {
      ui_px_put_preclipped(x, y, color);
    }
  }
}

void draw_tri_flat(triangle_t *d, px_t color) {
	int32_t *dataa, *datab, *datac;
	int32_t xs1, xs2, xs3;
	int32_t *tempdata;

  uint16_t w = SCREEN_W;
  uint16_t h = SCREEN_H;

	dataa = d->a;
	datab = d->b;
	datac = d->c;

	// Very simple sorting of the three y coordinates
	if (dataa[1] > datab[1]) {
		tempdata = dataa;
		dataa = datab;
		datab = tempdata;
	}
	if (datab[1] > datac[1]) {
		tempdata = datab;
		datab = datac;
		datac = tempdata;
	}
	if (dataa[1] > datab[1]) {
		tempdata = dataa;
		dataa = datab;
		datab = tempdata;
	}

	// Calculate some deltas 
	int32_t xd1 = datab[0] - dataa[0];
	int32_t xd2 = datac[0] - dataa[0];
	int32_t xd3 = datac[0] - datab[0];
	int32_t yd1 = datab[1] - dataa[1];
	int32_t yd2 = datac[1] - dataa[1];
	int32_t yd3 = datac[1] - datab[1];

	// Calculate steps per line while taking care of division by 0
	if (yd1 != 0) {
		xs1 = xd1 / yd1;
	}
	else {
		xs1 = xd1;
	}
	if (yd2 != 0) {
		xs2 = xd2 / yd2;
	}
	else {
		xs2 = xd2;
	}
	if (yd3 != 0) {
		xs3 = xd3 / yd3;
	}
	else {
		xs3 = xd3;
	}
	
	/*
	 xs: xstep=delta x
	 xw: current x-value used in loop
	*/
	/*
	 Start values for the first part (up to y of point 2)
	 xw1 and xw2 are x-values for the current line. The triangle is drawn from
	 top to bottom line after line...
	 txw, tyw and gw are values for texture and brightness
	 always for start- and ending-point of the current line
	 A line is also called "Span".
	*/

	int32_t xw1 = dataa[0]; //pax
	int32_t xw2 = dataa[0];
  
	if (yd1) {
		for (int sz = dataa[1]; sz <= datab[1]; sz++) {
			// draw if y is inside the screen (clipping)   
			if (sz >= h)
				break;
			if (sz >= 0 && sz < h) {
				int32_t xed = (xw1 < xw2) ? xw1 : xw2;
				int32_t xed2 = (xw1 < xw2) ? xw2 : xw1;
				xed = (xed  >> 16);
				xed2 = (xed2  >> 16);
				if ((xed < 0 && xed2 < 0) || (xed >= w && xed2 >= w) || (xed == xed2))
					goto skip_span;
				if (xed < 0) xed = 0;
				if (xed2 >= w) xed2 = w - 1;

				int clear_w = (xed == xed2) ? 1 : xed2 - xed;
        ui_span_preclipped(xed, sz, clear_w, color);
				
        skip_span:;
			}
			xw1 += xs1;
			xw2 += xs2;
		}
	}
	
	/*
	 New start values for the second part of the triangle
	*/
	xw1 = datab[0] + xs3;
  
	if (yd3) { //If Span-Height 1 or higher
		for (int sz=datab[1] + 1; sz < datac[1]; sz++)
		{
			if (sz >=h )
				break;

			if (sz >= 0 && sz < (h - 1)) {
				int32_t xed = (xw1 < xw2) ? xw1 : xw2;
				int32_t xed2 = (xw1 < xw2) ? xw2 : xw1;
				xed = (xed  >> 16);
				xed2 = (xed2 >> 16);
				if ((xed < 0 && xed2 < 0) || (xed >= w && xed2 >= w) || (xed == xed2))
					goto skip_span2;
				if (xed < 0) xed = 0;
				if (xed2 >= w) xed2 = w - 1;

				int clear_w = (xed == xed2) ? 1 : xed2 - xed;
        ui_span_preclipped(xed, sz, clear_w, color);
				skip_span2:;
			}
			xw1 += xs3;
			xw2 += xs2;
		}
	}
}

#define FONT_HDR 130
#define FONT_PITCH 256
#define FONT_BMP_PITCH 4128/8
#define FONT_BMP_HEIGHT 4112

uint8_t* data_font;

void draw_load_font() {
  FILE *f = fopen("unifont0.bmp", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  data_font = (uint8_t*)malloc(fsize + 1);
  fread(data_font, 1, fsize, f);
  fclose(f);
}

void draw_free_font() {
  free(data_font);
}

void draw_character(uint16_t dx, uint16_t dy, uint32_t c, px_t color, uint16_t scl) {
  int row = c/FONT_PITCH;
  int col = c%FONT_PITCH;

  for (int y=0; y<16; y++) {
    uint8_t px1 = data_font[FONT_HDR + FONT_BMP_HEIGHT*FONT_BMP_PITCH - ((row+1)*16+y+1)*FONT_BMP_PITCH + ((col+1)*2)];
    uint8_t px2 = data_font[FONT_HDR + FONT_BMP_HEIGHT*FONT_BMP_PITCH - ((row+1)*16+y+1)*FONT_BMP_PITCH + ((col+1)*2+1)];
    for (int x=0; x<8; x++) {
      if (~px1 & (1<<(7-x))) {
        draw_rect_fill(dx+x*scl, dy+y*scl, dx+(x+1)*scl-1, dy+(y+1)*scl-1, color);
      }
    }
    for (int x=0; x<8; x++) {
      if (~px2 & (1<<(7-x))) {
        draw_rect_fill(dx+(x+8)*scl, dy+y*scl, dx+(x+1+8)*scl-1, dy+(y+1)*scl-1, color);
      }
    }
  }
}

void draw_string_u32(uint16_t dx, uint16_t dy, uint32_t* str, px_t color, uint16_t scl) {
  int i=0;
  while (str[i]!=0) {
    draw_character(dx+16*scl*i, dy, str[i], color, scl);
    i++;
  }
}

void draw_string_u32_offset(uint16_t dx, uint16_t dy, uint32_t* str, px_t color, uint16_t scl, uint16_t offset) {
  int i=0;
  while (str[i]!=0) {
    draw_character(dx+16*scl*i, dy, str[i]+offset, color, scl);
    i++;
  }
}
