#include <string.h>
#include "../include/arena.h"
#include <stdint.h>

char *str_slice(Arena *arena, const char *str, uint64_t start, uint64_t end) {
  size_t len = strlen(str);
  if (end >= len && start >= len) return NULL;
  size_t s = 0;
  char *slice = (char*)arena_alloc(arena, end - start + 1);
  for (int i = start; i < end; i++) {
    slice[s++] = str[i];
  }
  return slice;
}
