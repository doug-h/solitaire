#include "solitaire.h"

#include <initializer_list>

#include <cstdio>
#include <cstring>
#include <ctime>

void shuffle(u8 *cards, iZ count) {
  for (int i = count - 1; i > 0; --i) {

    int j = i * ((double)rand() / (double)RAND_MAX);
    ASSERT(j < count && j >= 0);

    u8 c     = cards[i];
    cards[i] = cards[j];
    cards[j] = c;
  }
}

#define FOUNDATION_CAP 13
#define DRAWPILE_CAP 24
#define STACK_CAP 19
#define STOCK_CAP 24
#define HAND_CAP 13

void solit_init(solit_state *s, arena *perm) {
  srand(time(0));

  for (int i = 0; i < 7; ++i) {
    arena_alloc(perm, &(s->stacks[i]), STACK_CAP);
  }
  for (int i = 0; i < 4; ++i) {
    arena_alloc(perm, &(s->foundations[i]), FOUNDATION_CAP);
  }
  arena_alloc(perm, &(s->hand), HAND_CAP);
  arena_alloc(perm, &(s->stock), STOCK_CAP);
  arena_alloc(perm, &(s->draw_pile), DRAWPILE_CAP);
}

void solit_reset_game(solit_state *s) {
  for (int i = 0; i < NUM_PILES; ++i) {
    s->piles[i].clear();
  }

  card deck_store[52];

  stack<card> deck;
  deck.cap  = 52;
  deck.head = deck.tail = deck_store;

  for (Suit s : {HEARTS, CLUBS, DIAMONDS, SPADES}) {
    for (u8 r = ACE; r <= KING; ++r) {
      deck.push({.rank = r, .suit = s});
    }
  }
  shuffle((u8 *)deck.head, 52);

  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j <= i; ++j) {
      card c = deck.pop();
      c.flip = (j == i) ? FACEUP : FACEDOWN;
      s->stacks[i].push(c);
    }
  }

  while (!deck.isempty()) {
    card c = deck.pop();
    c.flip = FACEDOWN;
    s->stock.push(c);
  }
}

bool solit_flip_bottom_card(solit_state*s, u8 pile_no){
  ASSERT(pile_no <= STACK6);
  stack<card>& cards = s->piles[pile_no];
  if(cards.isempty() || cards.tail[-1].flip != FACEDOWN){
    return false;
  }
  cards.tail[-1].flip = FACEUP;
  return true;
}

void solit_draw_from_stock(solit_state *s) {
  // Restack
  if (s->stock.isempty()) {
    while (!s->draw_pile.isempty()) {
      card c = s->draw_pile.pop();
      c.flip = FACEDOWN;
      s->stock.push(c);
    }
    return;
  }

  // Draw 3
  int draw_count = 3;
  while (draw_count-- > 0 && !s->stock.isempty()) {
    card c = s->stock.pop();
    c.flip = FACEUP;
    s->draw_pile.push(c);
  }
}

bool pickup_possible(solit_state *s, u8 pile_no, u8 count) {
  stack<card> &source = s->piles[pile_no];

  if (source.size() < count) {
    return false;
  }

  card c = source.tail[-count];
  if (c.flip == FACEDOWN) {
    return false;
  }

  if (pile_no == DRAWPILE && count != 1) {
    return false;
  }

  return true;
}

bool place_possible(solit_state *s, u8 pile_no) {
  if (pile_no > FOUNDATION3) {
    return false;
  }

  stack<card> &dest = s->piles[pile_no];

  if (dest.free() < s->hand.size()) {
    return false;
  }

  card ch = s->hand.tail[-1];
  if (pile_no <= STACK6) {
    if (dest.size() == 0 && ch.rank == KING) {
      return true;
    }
    card cd = dest.tail[-1];
    if (cd.flip == FACEDOWN) {
      return false;
    }
    if (dest.size() != 0 && cd.rank == ch.rank + 1 &&
        COLOUR(ch.suit) != COLOUR(cd.suit)) {
      return true;
    }
    return false;
  }
  // if(pile_no <= FOUNDATION3)
  {
    if (s->hand.size() != 1) {
      return false;
    }
    if (dest.size() == 0 && ch.rank == ACE) {
      return true;
    }
    card cd = dest.tail[-1];
    if (dest.size() != 0 && ch.rank == cd.rank + 1 && ch.suit == cd.suit) {
      return true;
    }
    return false;
  }
}

bool solit_pickup_from_pile(solit_state *s, u8 pile_no, u8 count) {
  if (count == 0 || !s->hand.isempty()) {
    return false;
  }
  if (!pickup_possible(s, pile_no, count)) {
    return false;
  }
  while (count-- > 0) {
    s->hand.push(s->piles[pile_no].pop());
  }

  s->pile_last_taken_from = pile_no;
  return true;
}

bool solit_place_on_pile(solit_state *s, u8 pile_no) {

  if (s->hand.isempty()) {
    return false;
  }
  if (!place_possible(s, pile_no)) {
    return false;
  }

  while (!s->hand.isempty()) {
    s->piles[pile_no].push(s->hand.pop());
  }
  return true;
}

bool solit_return_hand_to_pile(solit_state *s) {
  ASSERT(s->pile_last_taken_from <= DRAWPILE);
  stack<card> &dest = s->piles[s->pile_last_taken_from];
  ASSERT(dest.free() >= s->hand.size());

  if(s->hand.isempty()){
    return false;
  }

  while (!s->hand.isempty()) {
    dest.push(s->hand.pop());
  }
  return true;
}

bool solit_game_is_won(solit_state *s) {
  for (int i = 0; i < 4; ++i) {
    if (s->foundations[i].size() != 13) {
      return false;
    }
  }
  return true;
}
