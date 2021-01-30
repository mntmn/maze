#include <stdio.h>
#include <stdint.h>

#include "platform.h"
#include "ui.h"
#include "draw.h"

#define DIM_X 256
#define DIM_Y 256
#define DIM_Z 256

#define PROP_SOLID 2
#define PROP_WIRE 1

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
  world[DIM_Y*DIM_X*z + DIM_X*y + x] = sec;
}

sector_t world_get(uint8_t x, uint8_t y, uint8_t z) {
  return world[DIM_Y*DIM_Z*z + DIM_Y*y + x];
}

uint8_t in_bounds(int16_t x, int16_t y, int16_t z) {
  if (x<0 || y<0 || z<0) return 0;
  if (x>=DIM_X || y>=DIM_Y || z>=DIM_Z) return 0;
  return 1;
}

uint32_t color_darken(uint32_t color, int lz) {
  uint32_t r = (color>>16)/lz;
  uint32_t g = ((color&0xff00)>>8)/lz;
  uint32_t b = (color&0xff)/lz;
  color = (r<<16)|(g<<8)|b;
  return color;
}

void render_actors_view(actor_t* actor) {
  int ax=actor->x;
  int ay=actor->y;
  int az=actor->z;

  int vw = 10;
  int vh = 10;
  int vd = 30;

  int cx = SCREEN_W/2;
  int cy = SCREEN_H/2;
  
  int scale = SCREEN_W/4;
  
  for (int rz=vd; rz>0; rz--) {
    for (int y=-vh; y<=vh; y++) {
      for (int x=-vw; x<=vw; x++) {
        if (in_bounds(ax+x,ay+y,az+rz)) {
          sector_t sec = world_get(ax+x,ay+y,az+rz);

          px_t color = 0x333333;

          if (sec.props>0) {
            int z = rz;
            int lz = (rz+1)/2;
            if (lz<1) lz=1;
            
            color = color_darken(sec.color, lz);
            
            /* draw a box here */
            /* project */
            int32_t x1 = cx + (x*scale)/z;
            int32_t x2 = cx + ((x+1)*scale)/z;
            int32_t x3 = cx + (x*scale)/(z+1);
            int32_t x4 = cx + ((x+1)*scale)/(z+1);
          
            int32_t y1 = cy + (y*scale)/z;
            int32_t y2 = cy + ((y+1)*scale)/z;
            int32_t y3 = cy + (y*scale)/(z+1);
            int32_t y4 = cy + ((y+1)*scale)/(z+1);

            // local fuzz
            x1+=sec.lx;
            x2+=sec.lx;
            x3+=sec.lx;
            x4+=sec.lx;
            
            y1+=sec.ly;
            y2+=sec.ly;
            y3+=sec.ly;
            y4+=sec.ly;            
          
            if (sec.props&PROP_SOLID) {
              //draw_rect_fill(x3, y3, x4, y4, color_darken(color,2));
            } else {
              draw_line(x1, y1, x2, y1, color);
              draw_line(x1, y2, x2, y2, color);
              draw_line(x1, y1, x1, y2, color);
              draw_line(x2, y1, x2, y2, color);
            }

            if (sec.props&PROP_SOLID) {
              //printf("x1: %d x2: %d x3: %d\n",x1,x2,x3);
              triangle_t tri = {
                {(x1)<<16, y1},
                {(x3)<<16, y3},
                {(x2)<<16, y1},
              };
              draw_tri_flat(&tri, color);
              
              tri = (triangle_t) {
                {x4<<16, y3},
                {x3<<16, y3},
                {x2<<16, y1},
              };
              draw_tri_flat(&tri, color);
              
            } else {
              draw_line(x1, y1, x3, y3, color);
              draw_line(x2, y1, x4, y3, color);
              draw_line(x1, y2, x3, y4, color);
              draw_line(x2, y2, x4, y4, color);
            }
            
            if (sec.props&PROP_SOLID) {
              //draw_rect_fill(x1, y1, x2, y2, color);
            } else {
              draw_line(x3, y3, x4, y3, color);
              draw_line(x3, y4, x4, y4, color);
              draw_line(x3, y3, x3, y4, color);
              draw_line(x4, y3, x4, y4, color);
            }

            //draw_fill(x1+1, y1+1, color);
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
    
    if (r>32700) {
      sector_t sec = {
        PROP_SOLID,
        0xff0000,
        0,0,0
      };
      world[i]=sec;
    }
    else if (r>32000) {
      sector_t sec = {
        PROP_WIRE,
        0xffffff,
        pseudo_rand()/2000,
        pseudo_rand()/2000,
        pseudo_rand()/2000,
      };
      world[i]=sec;
    }
  }

  sector_t sec = {
    PROP_SOLID,
    0xffffff,
    0,0,0
  };
  for (int z=10; z<250; z++) {
    world_put(125, 129, z, sec);
    world_put(126, 129, z, sec);
    world_put(127, 129, z, sec);
    world_put(128, 129, z, sec);
  }
}

int main(int argc, char** argv) {
  uint8_t running = 1;
  input_t input;
  actor_t player;
  int last_keycode=0;

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

    if (last_keycode!=input.keycode) {
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
      else if (input.keycode == 78) {
        player.y++;
      }
      else if (input.keycode == 75) {
        player.y--;
      }
      else if (input.keycode == 32) {
        // space
        world_put(player.x, player.y, player.z+2, (sector_t){
            PROP_WIRE,
            0x0000ff,
            pseudo_rand()/2000,
            pseudo_rand()/2000,
            pseudo_rand()/2000
          });
      }
    }
    
    last_keycode = input.keycode;

    printf("%d %d %d key: %d\n",player.x,player.y,player.z,input.keycode);
    
    ui_loop_post();
  }

  ui_exit();
}
