#pragma once

#include "sdlgl_texture.h"

#include "solitaire.h"
#include "types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_config.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#define CARD_WIDTH CARD_TEXTURE_WIDTH
#define CARD_HEIGHT CARD_TEXTURE_HEIGHT

#define MARGIN 10 // Around edges of table
#define SPACING 5 // Between card piles

#define STOCK_DX 0
#define STOCK_DY 0

#define DRAWPILE_DX 10

#define FOUNDATION_DX 0
#define FOUNDATION_DY 0
#define FOUNDATION_OFFSET_X ((CARD_WIDTH) + (SPACING))

#define STACK_DX 0
#define STACK_DY 15
#define STACK_OFFSET_X ((CARD_WIDTH) + (SPACING))

#define HAND_DY (STACK_DY)

#define MIN_WIDTH                                                              \
  (6 * (CARD_WIDTH) + 7 * (SPACING) + (DRAWPILE_CAP) * (DRAWPILE_DX))

/*
 * 0--3  Quad as two tris
 * |a/|     a = 0,1,3
 * |/b|     b = 1,2,3
 * 1--2
 */
struct card_vert {
  GLfloat x, y;
  GLint   face_id;
};

struct card_pile {
  float x, y;
  float dx, dy;
};

struct win_ani_state {
  bool playing;
  int card;
  int t;

  float vx;
};

struct sdlgl_state {
  bool initialised;
  bool updated;

  int  width, height;

  SDL_Window  *window;
  const Uint8 *keyboard;

  GLuint card_shader;
  GLuint vao, vbo;
  int num_verts;

  float grabbed_card_offset_x;
  float grabbed_card_offset_y;

  win_ani_state win_ani;

  union {
    struct {
      card_pile stacks[7];
      card_pile foundations[4];
      card_pile draw_pile;
      card_pile stock;
      card_pile hand;
    };
    card_pile piles[14];
  };
};

void sdlgl_init(sdlgl_state *, int width, int height, arena *perm, arena *temp);

void sdlgl_tick(sdlgl_state *, solit_state *);
void sdlgl_draw(sdlgl_state *, solit_state *);
