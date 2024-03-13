#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cassert>

#define ASSERT(c) assert((c))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t   uZ;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef ptrdiff_t iZ;

typedef float  f32;
typedef double f64;
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);

struct arena {
  u8 *head;
  u8 *tail;
};

arena new_arena(iZ size) {
  ASSERT(size > 0);
  u8 *head = (u8 *)calloc((uZ)size, 1);
  ASSERT(head);
  return {.head = head, .tail = head + size};
}

void free_arena(arena* a) {
  ASSERT(a->head);
  free(a->head);
  a->head = a->tail = 0;
}

u8 *arena_push_bytes(arena *a, iZ num_bytes, uZ align = 1) {
  u8 *aligned_tail = (u8 *)((uintptr_t)a->tail & ~(align - 1));

  iZ free = aligned_tail - a->head;
  if (free < num_bytes) {
    ASSERT(0); // OOM
  }
  a->tail = aligned_tail - num_bytes;
  return a->tail;
}

template <class T>
struct stack {
  T *head;
  T *tail;
  iZ cap;

  void push(T v) { *tail++ = v; }
  T    pop() { return *--tail; }
  void clear() { tail = head; }

  iZ   size() const { return tail - head; }
  iZ   free() const { return cap - size(); }
  bool isempty() const { return size() == 0; }
  bool isfull() const { return free() == 0; }
};
template <class T>
void arena_alloc(arena *a, stack<T> *s, iZ _cap) {
  s->cap  = _cap;
  s->head = s->tail = (T *)arena_push_bytes(a, _cap * (iZ)sizeof(T), alignof(T));
}
