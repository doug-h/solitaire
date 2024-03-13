#include "sdlgl_platform.h"
#include "sdlgl_shader.h"

#include "solitaire.h"

#include <cstdio>
#include <cmath>

void resize_board(sdlgl_state *s, int width, int height) {
  if (s->width == width && s->height == height) {
    return;
  }
  bool split_top_row = (width < MIN_WIDTH);

  s->width  = width;
  s->height = height;

  s->stock.x  = MARGIN;
  s->stock.y  = MARGIN;
  s->stock.dx = STOCK_DX;
  s->stock.dy = STOCK_DY;

  s->draw_pile.x  = s->stock.x + (STOCK_CAP * STOCK_DX) + CARD_WIDTH + SPACING;
  s->draw_pile.y  = MARGIN;
  s->draw_pile.dx = DRAWPILE_DX;
  s->draw_pile.dy = 0.0f;

  for (int i = 0; i < 4; ++i) {
    s->foundations[i].x = width - MARGIN - CARD_WIDTH - i * FOUNDATION_OFFSET_X;
    s->foundations[i].y = MARGIN + split_top_row * (CARD_HEIGHT + SPACING);
    s->foundations[i].dx = FOUNDATION_DX;
    s->foundations[i].dy = FOUNDATION_DY;
  }
  for (int i = 0; i < 7; ++i) {
    s->stacks[i].x  = MARGIN + i * STACK_OFFSET_X;
    s->stacks[i].y  = s->foundations[0].y + CARD_HEIGHT + SPACING;
    s->stacks[i].dx = STACK_DX;
    s->stacks[i].dy = STACK_DY;
  }

  s->hand.x  = 0.0f;
  s->hand.y  = 0.0f;
  s->hand.dx = 0.0f;
  s->hand.dy = HAND_DY;

  s->updated = true;
}

void sdlgl_init(sdlgl_state *s, int width, int height, arena *perm,
                arena *temp) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL could not initialize! SDL_Error:%s\n", SDL_GetError());
    ASSERT(0);
  }
  {
    SDL_version vers;
    SDL_GetVersion(&vers);
    printf("SDL Version: %d.%d.%d\n", vers.major, vers.minor, vers.patch);
  }

  int request_prof = SDL_GL_CONTEXT_PROFILE_ES;
  int request_majv = 3;
  int request_minv = 0;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, request_prof);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, request_majv);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, request_minv);

  Uint32      flags;
  SDL_Window *window;

  flags  = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  window = SDL_CreateWindow("Solitaire", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, width, height, flags);

  SDL_GL_CreateContext(window);

  int prof, majv, minv;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &prof);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &majv);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minv);

  // TODO - Allow non-exact version matches
  if (prof != request_prof || majv != request_majv || minv != request_minv) {
    puts("Platform doesn't support requested OpenGLES version");
    ASSERT(0);
  }

  // TODO - Query adaptive vsync
  SDL_GL_SetSwapInterval(0);

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(card_vert),
                        (void *)offsetof(card_vert, x));
  glEnableVertexAttribArray(0);
  glVertexAttribIPointer(1, 1, GL_INT, sizeof(card_vert),
                         (void *)offsetof(card_vert, face_id));
  glEnableVertexAttribArray(1);
  // glBindVertexArray(0);

  const char *vert_code =
#include "../shaders/card.vert"
      ;
  const char *frag_code =
#include "../shaders/card.frag"
      ;

  GLuint card_program = sdlgl_create_shader(vert_code, frag_code);
  glUseProgram(card_program);

  GLuint texture_deck =
      sdlgl_create_texture_deck(CARD_TEXTURE_WIDTH, CARD_TEXTURE_HEIGHT, *temp);

  GLint loc = glGetUniformLocation(card_program, "face_textures");
  glUniform1i(loc, 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_deck);

  glClearColor(table_green.r / 255.0f, table_green.g / 255.0f,
               table_green.b / 255.0f, 1.0f);
  SDL_GL_SwapWindow(window);

  ASSERT(s->initialised == 0);
  s->initialised = true;
  s->updated     = true;

  s->width  = 0;
  s->height = 0;

  s->window   = window;
  s->keyboard = SDL_GetKeyboardState(0);

  s->card_shader = card_program;
  s->vao         = vao;
  s->vbo         = vbo;
  s->num_verts   = 0;

  s->grabbed_card_offset_x = 0.0f;
  s->grabbed_card_offset_y = 0.0f;

  s->win_ani = {.playing = false};

  resize_board(s, width, height);
}

#define TO_VIEW_X(x) (((x) / (float)s->width) * 2.0f - 1.0f)
#define TO_VIEW_Y(y) (((y) / (float)s->height) * -2.0f + 1.0f)

void make_card_verts(sdlgl_state *s, int face_id, float x, float y,
                     stack<card_vert> *out) {
  float w, h;
  w = CARD_WIDTH;
  h = CARD_HEIGHT;

  out->push({.x = TO_VIEW_X(x + 0), .y = TO_VIEW_Y(y + 0), .face_id = face_id});
  out->push({.x = TO_VIEW_X(x + 0), .y = TO_VIEW_Y(y + h), .face_id = face_id});
  out->push({.x = TO_VIEW_X(x + w), .y = TO_VIEW_Y(y + 0), .face_id = face_id});
  out->push({.x = TO_VIEW_X(x + 0), .y = TO_VIEW_Y(y + h), .face_id = face_id});
  out->push({.x = TO_VIEW_X(x + w), .y = TO_VIEW_Y(y + h), .face_id = face_id});
  out->push({.x = TO_VIEW_X(x + w), .y = TO_VIEW_Y(y + 0), .face_id = face_id});
}

void make_pile_verts(sdlgl_state *s, solit_state *game, u8 pile_no,
                     stack<card_vert> *out) {
  stack<card> &cards = game->piles[pile_no];
  card_pile    pile  = s->piles[pile_no];

  if (cards.size() == 0 && pile_no != HAND) {
    make_card_verts(s, CARD_ID_EMPTY, pile.x, pile.y, out);
    return;
  }

  int n = (pile.dx || pile.dy || s->win_ani.playing) ? cards.size() : 1;
  while (n-- > 0) {
    card c = (pile_no == HAND) ? cards.head[n] : cards.tail[-n - 1];
    make_card_verts(s, CARD_ID(c), pile.x, pile.y, out);

    pile.x += pile.dx;
    pile.y += pile.dy;
  }
}
void upload_cards(sdlgl_state *s, solit_state *game) {
  card_vert vert_store[(14 + 52) * 6];

  stack<card_vert> vert_buffer = {
      .head = vert_store, .tail = vert_store, .cap = 52 * 6};

  for (int i = 0; i < NUM_PILES; ++i) {
    make_pile_verts(s, game, i, &vert_buffer);
  }

  int num_verts = vert_buffer.size();
  int num_bytes = num_verts * (iZ)sizeof(card_vert);
  glBufferData(GL_ARRAY_BUFFER, num_bytes, vert_buffer.head, GL_STATIC_DRAW);

  s->num_verts = num_verts;
}

void start_win_ani(sdlgl_state *s, solit_state *game) {
  ASSERT(solit_game_is_won(game));

  s->win_ani.playing = true;
  s->win_ani.card    = 0;
  s->win_ani.t       = 0;
  s->win_ani.vx      = 0.0f;

  upload_cards(s, game);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, s->num_verts);
  SDL_GL_SwapWindow(s->window);
}

void stop_win_ani(sdlgl_state *s) {
  s->win_ani.playing = false;
  s->win_ani.card    = 0;
  s->win_ani.t       = 0;
  s->win_ani.vx      = 0.0f;

  s->updated = true;
}

int mouse_pile_test(card_pile p, int pile_size, float x, float y) {
#if 0
  if(p.dx == 0 and (x < p.x or x > p.x+CARD_WIDTH)){
    return 0;
  }
  if(p.dy == 0 and (y < p.y or y > p.y+CARD_HEIGHT)){
    return 0;
  }
#endif
  for (int i = pile_size - 1; i >= 0; --i) {
    float px0 = p.x + i * p.dx;
    float py0 = p.y + i * p.dy;
    float px1 = px0 + CARD_WIDTH;
    float py1 = py0 + CARD_HEIGHT;
    if (px0 <= x and x <= px1 and py0 <= y and y <= py1) {
      return pile_size - i;
    }
  }

  return 0;
}

void on_mouse_down(sdlgl_state *s, solit_state *game, int button, float x,
                   float y) {
  if (s->win_ani.playing) {
    stop_win_ani(s);
    solit_reset_game(game);
    return;
  }
  if (game->hand.isempty() && button == SDL_BUTTON_RIGHT) {
    for (int pile_no = 0; pile_no <= DRAWPILE; ++pile_no) {
      if (pile_no == FOUNDATION0) {
        pile_no = FOUNDATION3;
        continue;
      }

      card_pile pile = s->piles[pile_no];

      int num_cards     = game->piles[pile_no].size();
      int cards_grabbed = mouse_pile_test(pile, num_cards, x, y);
      if (cards_grabbed == 1) {
        card c = game->piles[pile_no].tail[-1];

        for (int found_no = 0; found_no < 4; ++found_no) {
          stack<card> found = game->foundations[found_no];
          if (found.isempty()) {
            continue;
          }
          card top = found.tail[-1];
          if (c.suit == top.suit && c.rank == top.rank + 1) {
            if (solit_pickup_from_pile(game, pile_no, 1)) {
              if (solit_place_on_pile(game, FOUNDATION0 + found_no)) {
                s->updated = true;
              } else {
                solit_return_hand_to_pile(game);
              }
            }
            return;
          }
        }
      }
    }

    return;
  }

  if (game->hand.isempty() && mouse_pile_test(s->stock, 1, x, y)) {
    solit_draw_from_stock(game);
    s->updated = true;
    return;
  }
  for (int pile_no = 0; pile_no <= DRAWPILE; ++pile_no) {
    card_pile pile = s->piles[pile_no];

    int num_cards     = game->piles[pile_no].size();
    int cards_grabbed = mouse_pile_test(pile, num_cards, x, y);
    if (cards_grabbed) {
      if (pile_no <= STACK6 && game->hand.isempty() &&
          solit_flip_bottom_card(game, pile_no)) {
        s->updated = true;
      } else if (solit_pickup_from_pile(game, pile_no, cards_grabbed)) {
        int i = num_cards - cards_grabbed;

        s->grabbed_card_offset_x = pile.x + i * pile.dx - x;
        s->grabbed_card_offset_y = pile.y + i * pile.dy - y;

        s->hand.x = x + s->grabbed_card_offset_x;
        s->hand.y = y + s->grabbed_card_offset_y;

        s->updated = true;
      }
      return;
    }
  }

  return;
}
void on_mouse_up(sdlgl_state *s, solit_state *game, float x, float y) {
  if (s->win_ani.playing) {
    return;
  }
  for (int pile_no = 0; pile_no <= FOUNDATION3; ++pile_no) {
    card_pile pile = s->piles[pile_no];

    int num_cards = MAX(1, game->piles[pile_no].size());

    if (mouse_pile_test(pile, num_cards, x, y)) {
      if (solit_place_on_pile(game, pile_no)) {
        s->updated = true;
      }
      break;
    }
  }
  if (solit_return_hand_to_pile(game)) {
    s->updated = true;
  }
}

void on_mouse_motion(sdlgl_state *s, solit_state *game, int x, int y) {
  if (s->win_ani.playing) {
    return;
  }
  s->hand.x = x + s->grabbed_card_offset_x;
  s->hand.y = y + s->grabbed_card_offset_y;
  if (game->hand.size()) {
    s->updated = true;
  }
}

void sdlgl_tick(sdlgl_state *s, solit_state *game) {
  if (!s->win_ani.playing && solit_game_is_won(game)) {
    start_win_ani(s, game);
  }

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT: {
      // TODO - What goes here?
    } break;

    case SDL_WINDOWEVENT: {
      switch (e.window.event) {
      case SDL_WINDOWEVENT_SIZE_CHANGED: {
        printf("Resizing to %i, %i\n", e.window.data1, e.window.data2);
        glViewport(0, 0, e.window.data1, e.window.data2);

        resize_board(s, e.window.data1, e.window.data2);
      } break;
      }
    } break;

      // NOTE - SDL+Emscripten gets mouse state wrong sometimes
    case SDL_MOUSEMOTION: {
      on_mouse_motion(s, game, e.motion.x, e.motion.y);
    } break;
    case SDL_MOUSEBUTTONDOWN: {
      on_mouse_down(s, game, e.button.button, e.button.x, e.button.y);
    } break;
    case SDL_MOUSEBUTTONUP: {
      on_mouse_up(s, game, e.button.x, e.button.y);
    } break;

    case SDL_KEYUP: {
      switch (e.key.keysym.sym) {
      case SDLK_r: {
        stop_win_ani(s);
        solit_reset_game(game);
        s->updated = true;
      } break;
      }
    } break;
    };
  }
}

void draw_win_ani(sdlgl_state *s) {
  ++s->win_ani.t;

  card_pile start = s->foundations[s->win_ani.card % 4];

  float dx = s->win_ani.vx * s->win_ani.t;
  if (TO_VIEW_X(start.x) + dx > 1.0f ||
      TO_VIEW_X(start.x + CARD_WIDTH) + dx < -1.0f || dx == 0) {
    s->win_ani.t = 0;
    if (s->win_ani.card < 51) {
      if (dx) {
        ++s->win_ani.card;
      }

      float min_speed = 0.01f;

      float f  = (double)rand() / (double)RAND_MAX;
      // We want left bias since it looks better
      float df = std::lerp(-1, 1, f);
      float v  = std::lerp(min_speed, 3 * min_speed, abs(df));

      s->win_ani.vx = std::signbit(df) ? -v : v;
    }
  }
  if (s->win_ani.card < 51) {
    GLint offset = glGetUniformLocation(s->card_shader, "offset");

    float t_to_1st_bounce   = 20.0f;
    float coeff_restitution = 0.6f;

    float t = (float)s->win_ani.t / t_to_1st_bounce / 2.0f;

    int bounces = floor(t + 0.5f);

    float ay = abs(cos(3.14159f * t));
    float df = s->height + CARD_HEIGHT - 2;
    float y0 = TO_VIEW_Y(df);
    float y1 = 0.0f;
    float dy =
        std::lerp(y0, y0 + (y1 - y0) * pow(coeff_restitution, bounces), ay);
    glUniform2f(offset, s->win_ani.vx * s->win_ani.t, dy);

    int pile = s->win_ani.card % 4;
    int card = 12 - s->win_ani.card / 4;
    int idx  = FOUNDATION0 + card + pile * 13;

    glDrawArrays(GL_TRIANGLES, idx * 6, 6);
  }
}
void sdlgl_draw(sdlgl_state *s, solit_state *game) {
  if (s->win_ani.playing) {
    ASSERT(solit_game_is_won(game));
    draw_win_ani(s);
    SDL_GL_SwapWindow(s->window);
  } else if (s->updated) {

    upload_cards(s, game);

    GLint offset = glGetUniformLocation(s->card_shader, "offset");
    glUniform2f(offset, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, s->num_verts);
    SDL_GL_SwapWindow(s->window);

    s->updated = false;
  }
}
