#pragma once

#include "types.h"

enum Suit : u8 {
  HEARTS   = 0,
  CLUBS    = 1,
  DIAMONDS = 2,
  SPADES   = 3,
};
enum Colour {
  RED,
  BLACK,
};

#define COLOUR(suit) (Colour)(suit & 1)

enum Rank {
  ACE   = 1,
  // 2 - 10,
  JACK  = 11,
  QUEEN = 12,
  KING  = 13
};

enum Flip {
  FACEUP,
  FACEDOWN,
};

struct card {
  u8 rank : 4;
  u8 suit : 2;
  u8 flip : 1;
};

enum Pile {
  STACK0,
  STACK1,
  STACK2,
  STACK3,
  STACK4,
  STACK5,
  STACK6,
  FOUNDATION0,
  FOUNDATION1,
  FOUNDATION2,
  FOUNDATION3,
  DRAWPILE,
  STOCK,
  HAND,

  NUM_PILES,
};

struct solit_state {
  union {
    struct {
      stack<card> stacks[7];
      stack<card> foundations[4];
      stack<card> draw_pile;
      stack<card> stock;
      stack<card> hand;
    };
    stack<card> piles[NUM_PILES];
  };

  int pile_last_taken_from;
};

void solit_init(solit_state *, arena * perm);
void solit_reset_game(solit_state *);
bool solit_game_is_won(solit_state *);

// These return true if the board state has changed as a result of calling them
bool solit_flip_bottom_card(solit_state *, u8 pile_no);
bool solit_pickup_from_pile(solit_state *, u8 pile_no, u8 card_no);
bool solit_place_on_pile(solit_state *, u8 pile_no);
bool solit_return_hand_to_pile(solit_state *);

void solit_draw_from_stock(solit_state *);
