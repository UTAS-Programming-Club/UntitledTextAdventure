#include <assert.h>
#include <stddef.h>

#define ARENA_IMPLEMENTATION
#include <arena.h>

#include "alloc.h"

static Arena arena = {0};

void *Allocate(size_t size) {
  assert(&arena);
  return arena_alloc(&arena, size);
}

void FreeAll(void) {
  arena_free(&arena);
}
