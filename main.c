#include <stdio.h>
#include <stdint.h>

#include "platform.h"
#include "ui.h"
#include "draw.h"

#define DIM_X 256
#define DIM_Y 256
#define DIM_Z 256

#define PROP_SOLID 1

typedef struct sector {
  uint16_t props;
  px_t color;
  int8_t lx;
  int8_t ly;
  int8_t lz;
  // linked list of objects with local coords
} sector_t;

static sector_t world[DIM_X * DIM_Y * DIM_Z];

typedef struct actor {
  char* name;
  uint16_t x;
  uint16_t y;
  uint16_t z;
} actor_t;

void world_put(uint8_t x, uint8_t y, uint8_t z, sector_t sec) {
  world[DIM_Y*DIM_Z*z + DIM_Y*y + x] = sec;
}

sector_t world_get(uint8_t x, uint8_t y, uint8_t z) {
  return world[DIM_Y*DIM_Z*z + DIM_Y*y + x];
}

uint8_t in_bounds(int16_t x, int16_t y, int16_t z) {
  if (x<0 || y<0 || z<0) return 0;
  if (x>=DIM_X || y>=DIM_Y || z>=DIM_Z) return 0;
  return 1;
}

#define SCREEN_W 1920/2
#define SCREEN_H 1080/2

void render_actors_view(actor_t* actor) {
  int ax=actor->x;
  int ay=actor->y;
  int az=actor->z;

  int vw = 20;
  int vh = 10;
  int vd = 50;

  int cx = SCREEN_W/2;
  int cy = SCREEN_H/2;
  
  int scale = SCREEN_W/4;
  
  for (int rz=1; rz<=vd; rz++) {
    for (int y=-vh; y<=vh; y++) {
      for (int x=-vw; x<=vw; x++) {
        if (in_bounds(ax+x,ay+y,az+rz)) {
          sector_t sec = world_get(ax+x,ay+y,az+rz);

          px_t color = 0x333333;

          if (sec.props&PROP_SOLID) {
            color = sec.color;

            int z = rz;
        
            /* draw a box here */
            /* project */
            int x1 = cx + (x*scale)/z;
            int x2 = cx + ((x+1)*scale)/z;
            int x3 = cx + (x*scale)/(z+1);
            int x4 = cx + ((x+1)*scale)/(z+1);
          
            int y1 = cy + (y*scale)/z;
            int y2 = cy + ((y+1)*scale)/z;
            int y3 = cy + (y*scale)/(z+1);
            int y4 = cy + ((y+1)*scale)/(z+1);

            // local fuzz
            x1+=sec.lx;
            x2+=sec.lx;
            x3+=sec.lx;
            x4+=sec.lx;
            
            y1+=sec.ly;
            y2+=sec.ly;
            y3+=sec.ly;
            y4+=sec.ly;            
          
            draw_line(x1, y1, x2, y1, color);
            draw_line(x1, y2, x2, y2, color);
            draw_line(x1, y1, x1, y2, color);
            draw_line(x2, y1, x2, y2, color);

            //draw_fill(x1+1, y1+1, color);
          
            draw_line(x1, y1, x3, y3, color);
            draw_line(x2, y1, x4, y3, color);
            draw_line(x1, y2, x3, y4, color);
            draw_line(x2, y2, x4, y4, color);
          
            draw_line(x3, y3, x4, y3, color);
            draw_line(x3, y4, x4, y4, color);
            draw_line(x3, y3, x3, y4, color);
            draw_line(x4, y3, x4, y4, color);
          }
        }
      }
    }
  }
}

uint32_t pr_next = 1;

int16_t pseudo_rand(void)
{
    pr_next = pr_next * 1103515243 + 12345;
    return (int16_t)(pr_next / 65536) % 32768;
}

void seed_world() {
  for (uint32_t i=0; i<DIM_Z*DIM_Y*DIM_X; i++) {
    int16_t r = pseudo_rand();
    
    if (r>32000) {
      sector_t sec = {
        PROP_SOLID,
        0x0000ff,
        0,0,0
      };
      world[i]=sec;
    }
    else if (r>30000) {
      sector_t sec = {
        PROP_SOLID,
        0xff0000,
        pseudo_rand()/2000,
        pseudo_rand()/2000,
        pseudo_rand()/2000,
      };
      world[i]=sec;
    }
  }
}

int main(int argc, char** argv) {
  uint8_t running = 1;
  input_t input;
  actor_t player;

  player.name = "i";
  player.x = 127;
  player.y = 127;
  player.z = 122;

  seed_world();
  
  ui_init("MAZE");
  
  while (running) {
    ui_loop_pre(&input);

    draw_rect_fill(0, 0, SCREEN_W-1, SCREEN_H-1, 0);
    render_actors_view(&player);

    if (input.keycode == 27) {
      running = 0;
    }
    else if (input.keycode == 82) {
      player.z++;
    }
    else if (input.keycode == 81) {
      player.z--;
    }
    else if (input.keycode == 79) {
      player.x++;
    }
    else if (input.keycode == 80) {
      player.x--;
    }

    //printf("%d %d %d\n",player.x,player.y,player.z);
    
    ui_loop_post();
  }

  ui_exit();
}
