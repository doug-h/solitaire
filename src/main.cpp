#include "sdlgl_platform.h"
#include "solitaire.h"

#include <emscripten.h>
#include <emscripten/html5.h>

/* TODO:
 *  - Fix win animation on resize
 *  -
 */

sdlgl_state platform_state;
solit_state game_state;

void main_loop() {
  sdlgl_tick(&platform_state, &game_state);
  sdlgl_draw(&platform_state, &game_state);
};

int main(int argv, char **args) {
  arena temp = new_arena(1 << 15);
  arena perm = new_arena(1 << 8);

  sdlgl_init(&platform_state, 900, 600, &perm, &temp);
  solit_init(&game_state, &perm);

  free_arena(&temp);

  solit_reset_game(&game_state);

  emscripten_set_main_loop(main_loop, 0, true);

  return 0;
};

#include "solitaire.cpp"
#include "sdlgl_platform.cpp"
