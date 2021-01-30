#include <stdio.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "ui.h"

#define SCREEN_W 1920/2
#define SCREEN_H 1080/2

SDL_Window* sdl_window;
SDL_Renderer* sdl_renderer;
SDL_Texture* sdl_texture;
px_t* pixels;

void ui_px_put(uint16_t x, uint16_t y, px_t color)
{
  if (x<0 || y<0 || x>=SCREEN_W || y>=SCREEN_H) return;
  *(pixels + y*SCREEN_W + x) = color;
}

px_t ui_px_get(uint16_t x, uint16_t y) {
  if (x<0 || y<0 || x>=SCREEN_W || y>=SCREEN_H) return 0xffffffff;
  return *(pixels + y*SCREEN_W + x);
}

void ui_loop_pre(input_t* input) {
  SDL_Event event;

  if (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      input->keycode = 27; // escape
      break;
    case SDL_KEYDOWN:
      {
        int c = event.key.keysym.sym;
        // FIXME this is obvs. very primitive (only 1 key allowed at a time)
        input->keycode = c;
        break;
      }
    case SDL_KEYUP:
      {
        int c = event.key.keysym.sym;
        input->keycode = 0;
        break;
      }
    case SDL_MOUSEMOTION:
      {
        input->mouse_x = event.motion.x;
        input->mouse_y = event.motion.y;
        break;
      }
    case SDL_MOUSEBUTTONDOWN:
      {
        input->mouse_buttons |= (1<<(event.button.button-1));
        break;
      }
    case SDL_MOUSEBUTTONUP:
      {
        input->mouse_buttons &= ~(1<<(event.button.button-1));
        break;
      }
    }
  }
  
  SDL_UpdateTexture(sdl_texture, NULL, pixels, SCREEN_W * sizeof(px_t));
  SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
}

void ui_loop_post() {
  SDL_RenderPresent(sdl_renderer);
}

int ui_init(char* title) {
  // Initialize the SDL video/audio components.
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("Can't initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  /* Uncommenting the following two lines turns on pixel smoothing. */
  /* SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
     SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); */

  /* The SDL window defines the window title, position, and size,
     along with how to render it. */
  sdl_window = SDL_CreateWindow(title,
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                SCREEN_W, SCREEN_H,
                                SDL_WINDOW_OPENGL);

  if (!sdl_window) {
    printf("[ui/sdl] Can't open window: %s\n", SDL_GetError());
    return 1;
  }

  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  
  if (!sdl_renderer) {
    printf("[ui/sdl] Can't start renderer: %s\n", SDL_GetError());
    return 1;
  }

  /* Create a texture as big as the screen.
     The texture contains the actual pixel data. */
  sdl_texture = SDL_CreateTexture(sdl_renderer,
			       SDL_PIXELFORMAT_ARGB8888,
			       SDL_TEXTUREACCESS_STREAMING,
			       SCREEN_W, SCREEN_H);

  SDL_RenderSetLogicalSize(sdl_renderer, SCREEN_W, SCREEN_H);
  
  pixels = (px_t*)malloc(SCREEN_W*SCREEN_H*sizeof(px_t));

  if (!pixels) {
    printf("[ui/sdl] Can't allocate pixels!\n");
    return 1;
  }

  return 0;
}

void ui_exit() {
  SDL_Quit();
  free(pixels);
}
