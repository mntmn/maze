#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"
#include "ui.h"
#include "draw.h"

#define DIM_X 256
#define DIM_Y 256
#define DIM_Z 256

#define PROP_WIRE 1
#define PROP_SOLID 2
#define PROP_DOOR 4

#define GST_INTRO 0
#define GST_PLAY 1
#define GST_WON 2
#define GST_LOST 3
#define GST_MOVE 4

#define enable_solids 1

int game_state = GST_INTRO;

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
  int x;
  int y;
  int z;
  int look; // view rotation around y axis; index into look_xz list
  int health;
  int move_dir;
  int move_counter;
} actor_t;

int rot_walk_x[] = {0,1,0,-1};
int rot_walk_z[] = {1,0,-1,0};
int rot_strafe_x[] = {1,0,-1,0};
int rot_strafe_z[] = {0,-1,0,1};

int NUM_ENEMIES = 1;

actor_t player;
actor_t enemies[32];

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

uint32_t pr_next = 1;

int16_t pseudo_rand(void)
{
    pr_next = pr_next * 1103515243 + 12345;
    return (int16_t)(pr_next / 65536) % 32768;
}

double T = 0.0;
int fx_wound = 0;

void render_sector_box(sector_t* sec, int x1, int x2, int x3, int x4, int y1, int y2, int y3, int y4) {
  int cx = SCREEN_W/2;
  int cy = SCREEN_H/2;
  px_t color2 = color_darken(sec->color, 2);
  px_t color3 = color_darken(sec->color, 3);
  
  if (sec->props&PROP_WIRE) {
    int lx = sin(T)*2;
    int ly = cos(T)*5;

    // local fuzz
    x1+=lx;
    x2+=lx;
    x3+=lx;
    x4+=lx;

    y1+=ly;
    y2+=ly;
    y3+=ly;
    y4+=ly;
  }
  
  if (enable_solids && sec->props&PROP_SOLID) {
    // the back is never visible
    //draw_rect_fill(x3, y3, x4, y4, color_darken(color,2));
  } else {
    draw_line(x1, y1, x2, y1, sec->color);
    draw_line(x1, y2, x2, y2, sec->color);
    draw_line(x1, y1, x1, y2, sec->color);
    draw_line(x2, y1, x2, y2, sec->color);
  }

  if (enable_solids && sec->props&PROP_SOLID) {
    triangle_t tri;

    if (y1>cy && y3>cy) {
      // top
      tri = (triangle_t) {
        {(x1)<<16, y1},
        {(x3)<<16, y3},
        {(x2)<<16, y1},
      };
      draw_tri_flat(&tri, sec->color);

      tri = (triangle_t) {
        {x4<<16, y3},
        {x3<<16, y3},
        {x2<<16, y1},
      };
      draw_tri_flat(&tri, sec->color);
    }
    
    if (y1<cy && y3<cy) {
      // bottom
      tri = (triangle_t) {
        {x1<<16, y2},
        {x3<<16, y4},
        {x2<<16, y2},
        };
        draw_tri_flat(&tri, sec->color);

        tri = (triangle_t) {
        {x4<<16, y4},
        {x3<<16, y4},
        {x2<<16, y2},
        };
        draw_tri_flat(&tri, sec->color);
    }
    
    if (x3>cx && x4>cx) {
      // left
      tri = (triangle_t) {
        {x1<<16, y1},
        {x3<<16, y3},
        {x3<<16, y4},
      };
      draw_tri_flat(&tri, sec->color);

      tri = (triangle_t) {
        {x1<<16, y1},
        {x1<<16, y2},
        {x3<<16, y4},
      };
      draw_tri_flat(&tri, sec->color);
    }
    
    if (x1<cx && x2<cx) {
      // right
      tri = (triangle_t) {
        {x2<<16, y1},
        {x4<<16, y3},
        {x4<<16, y4},
      };
      draw_tri_flat(&tri, color3);

      tri = (triangle_t) {
        {x2<<16, y1},
        {x2<<16, y2},
        {x4<<16, y4},
      };
      draw_tri_flat(&tri, color3);
    }
  } else {
    draw_line(x1, y1, x3, y3, sec->color);
    draw_line(x2, y1, x4, y3, sec->color);
    draw_line(x1, y2, x3, y4, sec->color);
    draw_line(x2, y2, x4, y4, sec->color);
  }

  if (enable_solids && sec->props&PROP_SOLID) {
    // front
    if (y1>(SCREEN_H*750)/1000) {
      draw_rect_fill(x1, y1+1, x2, y2, sec->color);
    } else {
      draw_rect_fill(x1, y1+1, x2, y2, color2);
    }
  } else {
    draw_line(x3, y3, x4, y3, sec->color);
    draw_line(x3, y4, x4, y4, sec->color);
    draw_line(x3, y3, x3, y4, sec->color);
    draw_line(x4, y3, x4, y4, sec->color);
  }
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

  int xstep = 1;
  int zstep = -1;
  int ystep = -1;

  for (int rz=vd; rz!=0; rz+=zstep) {
    for (int ry=vh; ry!=-vh; ry+=ystep) {
      for (int rx=-vw; rx<=vw; rx+=xstep) {

        int x = rx;
        if (rx>=0) {
          // reverse render direction in the middle
          x = vw - rx;
        }
        int y = ry;
        if (ry<0) {
          // reverse render direction in the middle
          //y = -vh + ry;
        }
        
        int wx, wy, wz;
        wy = ay+y;
        
        if (actor->look == 0) {
          wx = ax+x;
          wz = az+rz;
        } else if (actor->look == 1) {
          wx = ax+rz;
          wz = az-x;
        } else if (actor->look == 2) {
          wx = ax-x;
          wz = az-rz;
        } else if (actor->look == 3) {
          wx = ax-rz;
          wz = az+x;
        }

        if (in_bounds(wx,wy,wz)) {
          sector_t sec = world_get(wx,wy,wz);

          px_t color = 0x333333;

          int z = rz;
          int lz = (rz+1)/2;
          if (lz<1) lz=1;
          if (player.health<5) lz+=2;
          if (player.health<3) lz+=4;
          if (player.health<2) lz+=8;

          int zz = z*10 - rot_walk_z[player.move_dir]*player.move_counter;
          int xx = x*10 + rot_walk_x[player.move_dir]*player.move_counter;
          int yy = y*10;

          /* project a box */
          int32_t x1 = cx + (xx*scale)/zz;
          int32_t x2 = cx + ((xx+10)*scale)/zz;
          int32_t x3 = cx + (xx*scale)/(zz+10);
          int32_t x4 = cx + ((xx+10)*scale)/(zz+10);

          int32_t y1 = cy + (yy*scale)/zz;
          int32_t y2 = cy + ((yy+10)*scale)/zz;
          int32_t y3 = cy + (yy*scale)/(zz+10);
          int32_t y4 = cy + ((yy+10)*scale)/(zz+10);

          if (sec.props>0) {
            render_sector_box(&sec,x1,x2,x3,x4,y1,y2,y3,y4);
          }

          // render enemies
          for (int i=0; i<NUM_ENEMIES; i++) {
            if (enemies[i].x==wx && enemies[i].y==wy && enemies[i].z==wz) {
              int32_t x5 = (x1+x2)/2;
              int32_t x6 = (x3+x4)/2;

              triangle_t tri = (triangle_t) {
                {x5<<16, y1},
                {x1<<16, y2},
                {x2<<16, y2},
              };
              draw_tri_flat(&tri, 0xff0000);
            }
          }
        }
      }
    }
  }

  if (fx_wound) {
    draw_rect_fill(0, 0, SCREEN_W-1, SCREEN_H-1, 0xff0000);
    draw_string_u32(200+pseudo_rand()/1000, 100+pseudo_rand()/1000,
                    (uint32_t[]){'A','R','G','H',0}, 0, 8);
    fx_wound--;
  }
}

void seed_world() {
  sector_t sec = {
    0,
    0,
    0,0,0
  };
  
  for (uint32_t y=0; y<DIM_Y; y++) {
    for (uint32_t x=0; x<DIM_X; x++) {
      for (uint32_t z=0; z<DIM_Z; z++) {
        world_put(x, y, z, sec);
      }
    }
  }
  
  for (uint32_t y=0; y<125; y++) {
    for (uint32_t x=0; x<DIM_X; x++) {
      for (uint32_t z=0; z<DIM_Z; z++) {
        int16_t r = pseudo_rand();

        if (r>32000) {
          sec = (sector_t) {
            PROP_WIRE,
            0xffffff,
            0,0,0
          };
          world_put(x, y, z, sec);
        }
      }
    }
  }

  sec = (sector_t) {
    PROP_SOLID,
    0xffffff,
    0,0,0
  };

  // ground
  for (int x=0; x<DIM_X; x++) {
    for (int z=0; z<DIM_Z; z++) {
      sec.props = PROP_SOLID;
      sec.color = 0x008800;
      // stripe
      if (z==129) sec.color=0xffffff;
      world_put(x, 128, z, sec);
    }
  }

  // underground 1
  for (int x=127-5; x<127+5; x++) {
    for (int z=127-5; z<127+5; z++) {
      sec.props = PROP_SOLID;
      sec.color = 0x880000;
      world_put(x, 180, z, sec);
    }
  }

  // a hole in the ground
  sec.props = 0;
  sec.color = 0;
  world_put(126, 128, 131, sec);
  world_put(127, 128, 131, sec);
  world_put(126, 128, 130, sec);
  world_put(127, 128, 130, sec);

  // wall around the hole
  for (int x=127-5; x<127+5; x++) {
    for (int z=127-5; z<127+5; z++) {
      if (x==127-5 || x==127+4 || z==127-5 || z==127+4) {
        sec.props = PROP_SOLID;
        sec.color = 0x0000ff;
        world_put(x, 127, z, sec);
        world_put(x, 126, z, sec);
        sec.props = PROP_WIRE;
        world_put(x, 125, z, sec);
      }
    }
  }

  // a door
  sec.props = PROP_SOLID|PROP_DOOR;
  sec.color = 0xff00ff;
  world_put(127, 127, 127-5, sec);
}

int process_gravity(actor_t* player) {
  int wx=player->x;
  int wy=player->y+1;
  int wz=player->z;
  if (in_bounds(wx, wy, wz)) {
    if (!(world_get(wx,wy,wz).props&PROP_SOLID)) {
      player->y++;
    }
    return 0;
  } else {
    return 1;
  }
}

#define ERES_NONE 0
#define ERES_WOUND 1

int process_enemies(int step) {
  int res = ERES_NONE;

  for (int i=0; i<NUM_ENEMIES; i++) {
    int dx=0, dz=0;
    process_gravity(&enemies[i]);

    if (step) {
      // move towards player
      if (player.x>enemies[i].x+1) {
        dx = 1;
      }
      if (player.x<enemies[i].x-1) {
        dx = -1;
      }
      if (player.z>enemies[i].z+1) {
        dz = 1;
      }
      if (player.z<enemies[i].z-1) {
        dz = -1;
      }

      if (dx||dz) {
        if (pseudo_rand()>0) {
          enemies[i].x+=dx;
        } else {
          enemies[i].z+=dz;
        }
      } else if (enemies[i].y == player.y) {
        /* enemy attacks */
        player.health--;
        res = ERES_WOUND;
      }
    }
  }

  return res;
}

void process_play() {
  int enemy_step = 0;
  int timer = ((int)T)%3;
  //printf("timer: %d\n",timer);
  //if (timer==0 && timer!=last_timer) enemy_step = 1;

  //last_timer = timer;

  int eres = process_enemies(enemy_step);

  if (eres == ERES_WOUND) {
    fx_wound = 5;
  }

  if (process_gravity(&player) == 1) {
    /*
      you left the map = won!
    */
    game_state = GST_WON;
  }

  if (player.health<1) {
    game_state = GST_LOST;
  }
}

uint32_t STR_GAME_OVER[] = {'G','A','M','E',' ','O','V','E','R',0};
uint32_t STR_GAME_WON[] = {'*','Y','O','U',' ','W','O','N','*',0};
uint32_t STR_GAME_INTRO1[] = {'E','S','C','A','P','E',0};
uint32_t STR_GAME_INTRO2[] = {'T','H','E',0};
uint32_t STR_GAME_INTRO3[] = {'M','A','Z','E',0};

void init_game() {
  player.name = "i";
  player.x = 127;
  player.y = 127;
  player.z = 110;
  player.health = 10;
  player.look = 0;

  enemies[0].x = 127;
  enemies[0].y = 40;
  enemies[0].z = 129;
  enemies[0].health = 10;
  enemies[0].look = 0;

  seed_world();
}

int walk_coords(int* x, int* y, int* z, int look, int dir) {
  int dx = *x;
  int dy = *y;
  int dz = *z;
  if (dir == 0) {
    dx+=rot_walk_x[look];
    dz+=rot_walk_z[look];
  } else if (dir == 2) {
    dx-=rot_walk_x[look];
    dz-=rot_walk_z[look];
  } else if (dir == 1) {
    dx-=rot_strafe_x[look];
    dz-=rot_strafe_z[look];
  } else if (dir == 3) {
    dx+=rot_strafe_x[look];
    dz+=rot_strafe_z[look];
  }
  *x=dx;
  *y=dy;
  *z=dz;
}

int walkable(int x, int y, int z, int look, int dir) {
  int dx = x;
  int dy = y;
  int dz = z;
  walk_coords(&dx, &dy, &dz, look, dir);
  
  if (!in_bounds(dx, dy, dz)) return 0;
  sector_t sec = world_get(dx, dy, dz);
  if (sec.props & PROP_SOLID) return 0;
  return 1;
}

int interact(int x, int y, int z, int look, int dir) {
  int dx = x;
  int dy = y;
  int dz = z;
  walk_coords(&dx, &dy, &dz, look, dir);

  if (!in_bounds(dx, dy, dz)) return 0;
  sector_t sec = world_get(dx, dy, dz);

  if (sec.props & PROP_DOOR) {
    if (sec.props & PROP_SOLID) {
      sec.props = PROP_DOOR|PROP_WIRE;
    } else {
      sec.props = PROP_DOOR|PROP_SOLID;
    }
    world_put(dx, dy, dz, sec);
    return 1;
  }
  
  return 0;
}

#define MOVE_STEPS 10

int main(int argc, char** argv) {
  uint8_t running = 1;
  input_t input;
  int last_keycode=0;
  int last_timer=0;

  draw_load_font();

  init_game();

  ui_init("MAZE");

  while (running) {
    T += 0.1;

    ui_loop_pre(&input);

    draw_rect_fill(0, 0, SCREEN_W-1, SCREEN_H-1, 0);

    render_actors_view(&player);

    if (game_state == GST_MOVE) {
      player.move_counter++;
      if (player.move_counter == MOVE_STEPS) {
        player.move_counter = 0;
        if (player.move_dir == 0) {
          player.x+=rot_walk_x[player.look];
          player.z+=rot_walk_z[player.look];
        } else if (player.move_dir == 2) {
          player.x-=rot_walk_x[player.look];
          player.z-=rot_walk_z[player.look];
        } else if (player.move_dir == 1) {
          player.x-=rot_strafe_x[player.look];
          player.z-=rot_strafe_z[player.look];
        } else if (player.move_dir == 3) {
          player.x+=rot_strafe_x[player.look];
          player.z+=rot_strafe_z[player.look];
        }
        game_state = GST_PLAY;
      }
      process_play();
    }
    else if (game_state == GST_PLAY) {
      if (last_keycode!=input.keycode) {
        if (input.keycode == 27) {
          game_state = GST_LOST;
        }
        else if (input.keycode == 82 || input.keycode == 119) {
          // north
          if (walkable(player.x,player.y,player.z,player.look,0)) {
            player.move_dir = 0;
            player.move_counter = 0;
            game_state = GST_MOVE;
          }
        }
        else if (input.keycode == 81 || input.keycode == 115) {
          // south
          if (walkable(player.x,player.y,player.z,player.look,2)) {
            player.move_dir = 2;
            player.move_counter = 0;
            game_state = GST_MOVE;
          }
        }
        else if (input.keycode == 79) {
          // east
          player.look=(player.look+1)%4;
        }
        else if (input.keycode == 80) {
          // west
          player.look=player.look==0 ? 3 : player.look-1;
        }
        else if (input.keycode == 97) {
          // A
          if (walkable(player.x,player.y,player.z,player.look,1)) {
            player.move_dir = 1;
            player.move_counter = 0;
            game_state = GST_MOVE;
          }
        }
        else if (input.keycode == 100) {
          // D
          if (walkable(player.x,player.y,player.z,player.look,3)) {
            player.move_dir = 3;
            player.move_counter = 0;
            game_state = GST_MOVE;
          }
        }
        else if (input.keycode == 78) {
          // up
          player.y++;
        }
        else if (input.keycode == 75) {
          // down
          player.y--;
        }
        else if (input.keycode == 32) {
          // space:
          // action
          interact(player.x, player.y, player.z, player.look, 0);
        }
      }

      process_play();
    } else if (game_state == GST_LOST) {
      uint16_t rand_offset = pseudo_rand()%0xff00;
      if (((int)T)%20 != 0) rand_offset = 0xfee0;
      draw_string_u32_offset(192, 238, STR_GAME_OVER, 0xff00ff, 4, rand_offset);

      if (last_keycode!=input.keycode) {
        if (input.keycode == 27) {
          init_game();
          game_state = GST_INTRO;
        } else if (input.keycode == 32) {
          init_game();
          game_state = GST_INTRO;
        }
      }
    } else if (game_state == GST_INTRO) {
      uint16_t rand_offset = pseudo_rand()%0xff00;
      if (((int)T)%20 != 0) rand_offset = 0xfee0;
      draw_string_u32_offset(240, 138, STR_GAME_INTRO1, 0x00ffff, 4, rand_offset);
      draw_string_u32_offset(240, 238, STR_GAME_INTRO2, 0x00ffff, 4, rand_offset);
      draw_string_u32_offset(240, 338, STR_GAME_INTRO3, 0x00ffff, 4, rand_offset);

      if (last_keycode!=input.keycode) {
        if (input.keycode == 27) {
          running = 0;
        } else if (input.keycode == 32) {
          init_game();
          game_state = GST_PLAY;
        }
      }

    } else if (game_state == GST_WON) {
      uint16_t rand_offset = pseudo_rand()%0xff00;
      if (((int)T)%20 != 0) rand_offset = 0xfee0;
      draw_string_u32_offset(192, 238, STR_GAME_WON, 0xff00ff, 4, rand_offset);

      if (last_keycode!=input.keycode) {
        if (input.keycode == 27) {
          init_game();
          game_state = GST_INTRO;
        } else if (input.keycode == 32) {
          init_game();
          game_state = GST_INTRO;
        }
      }
    }

    last_keycode = input.keycode;

    //printf("%d %d %d key: %d\n",player.x,player.y,player.z,input.keycode);

    ui_loop_post();
  }

  draw_free_font();
  ui_exit();
}
