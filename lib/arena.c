/* 
This implementation is not stable and it may have critical issues.
It is not thread safe. Use it with caution!

This is a simple implementation of the well-known arena allocation
strategy. When initialized, a fixed size of memory is returned 
into an arena instance using malloc. Later, blocks of memory can
be requested from this chunk as needed as long as the required
size is less than the arena's capacity. This scinario is termed as
"arena overflow". When the allocated memory needs to be freed,
just free the entire arena.

Author: Harikrishna Mohan
Date: 16-07-2024

## HOW TO USE ##
Arena *arena_init(uint64_t capacity)
  -- initializes the arena chunk with a capacity of
      ARENA_[8,16,32,..,2048] or any custom integer greater than 0.

void *arena_alloc(Arena *arena, uint64_t size)
  -- Returns required size of memory from the arena to use.
      Returns NULL if the requested size is more than its capacity.

void arena_visualize(const Arena *arena)
  -- To get an overview of the arena.

void arena_reset(Arena *arena)
  -- resets the allocated sizes to 0, doesn't actually frees any memory.

void arena_free(Arena *arena)
  -- Deallocates the entire arena.

Reference materials: https://m.youtube.com/watch?v=ZisNZcQn6fo&pp=ygULYXJlbmEgYWxsb2M%3D
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "err.c"
_Thread_local char arena_err[ERR_BUF_SIZE];

typedef struct Arena {
  uint64_t capacity; // holds total capacity of the chunk.
  uint64_t buf_size; // total used size in the chunk.
  uint8_t *arena_buf; // stores the actual chunk.
  struct Arena *next_arena; // to face arena overflow.
} Arena;

// initializes the arena chunk with a capacity of 
// ARENA_[8,16,32,..,2048] or any custom integer greater than 0.
Arena *arena_init(uint64_t capacity) {
    if (capacity <= 0)
      return_halt(arena_err, NULL, "%s(): Capacity of arena must be greater than 0.", __FUNCTION__);

    uint8_t *new_buffer = malloc(sizeof(uint8_t) * capacity);
    if (new_buffer == NULL)
      return_halt(arena_err, NULL, "%s(): Failed to allocate memory for new buffer.", __FUNCTION__);

    Arena *arena = malloc(sizeof(Arena));
    if (arena == NULL)
      return_halt(arena_err, NULL, "%s(): Failed to allocate memory for arena", __FUNCTION__);

    arena->capacity = capacity;
    arena->buf_size = 0;
    arena->arena_buf = new_buffer;
    arena->next_arena = NULL;
    return_ok(arena_err, arena);
}

// Returns required size of memory from the arena to use.
// Returns NULL if the requested size is more than its capacity.
void *arena_alloc(Arena *arena, uint64_t size) {
  if (size > arena->capacity)
    return_bad(arena_err, NULL, "%s(): The requested size must be less than or equal to arena capacity", __FUNCTION__);

  Arena *current = arena;
  // if the requsted size does not fit inside the current instance,
  // create a new arena instance and traverse into it.
  while(current->buf_size + size > current->capacity) {
    if(current->next_arena == NULL) {
      current->next_arena = arena_init(current->capacity);
    }
    current = current->next_arena;
  }
  // current->arena_buf points to the start of arena_buf
  // buf_start_address + current_allocated_size gives 
  // an address to next unused space.
  void *mem_buf = current->arena_buf + current->buf_size;
  current->buf_size += size;
  return_ok(arena_err, mem_buf);
}

// To get an overview of the arena.
void arena_visualize(const Arena *arena) {
  const Arena *current = arena;
  while(current != NULL) {
    printf("capacity: %lu, used size: %lu, buf: %p, nxt: %p\n",
           current->capacity,
           current->buf_size,
           current->arena_buf,
           current->next_arena);
    current = current->next_arena;
  }
}

// resets the allocated sizes to 0.
// doesn't actually frees any memory.
void arena_reset(Arena *arena) {
  Arena *current = arena;
  while(current != NULL) {
    current->buf_size = 0;
    current = current->next_arena;
  }
}

// Deallocates the entire arena.
void arena_free(Arena *arena) {
  Arena *current = arena;
  Arena *next;
  while(current != NULL) {
    free(current->arena_buf);
    current->buf_size = 0;
    current->capacity = 0;
    next = current->next_arena;
    free(current);
    current = next;
  }
}
