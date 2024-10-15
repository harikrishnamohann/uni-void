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

This program is a part of "hobby's_".
Author: Harikrishna Mohan
Date: 16-07-2024

Reference materials: https://m.youtube.com/watch?v=ZisNZcQn6fo&pp=ygULYXJlbmEgYWxsb2M%3D
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../include/types.h"

// Prototype of Arena
typedef struct Arena {
  u64 capacity; // holds total size of chunk of memory.
  u64 allocated_size; // total used size in the chunk.
  u8 *arena_buf; // stores the actual chunk.
  struct Arena *next_node; // to face arena overflow.
} Arena;

// initializes the arena chunk with a capacity of 
// ARENA_[8,16,32,..,2048] or any custom integer greater than 0.
Arena *arena_init(u64 capacity) {
    assert(capacity != 0);
    u8 *new = malloc(sizeof(u8) * capacity);
    assert(new != NULL);
    Arena *arena = malloc(sizeof(Arena));
    arena->capacity = capacity;
    arena->allocated_size = 0;
    arena->arena_buf = new;
    arena->next_node = NULL;
  return arena;
}

// Returns required size of memory from the arena to use.
// Returns NULL if the requested size is more than its capacity.
void *arena_alloc(Arena *arena, u64 size) {
  assert(size < arena->capacity);
  Arena *current = arena;
  // if the requsted size does not fit inside the current instance,
  // create a new arena instance and link it to next_node.
  // it's a single linked list.
  while(current->allocated_size + size > current->capacity) {
    if(current->next_node == NULL) {
      current->next_node = arena_init(current->capacity);
    }
    current = current->next_node;
  }
  // current->arena_buf => points to the start of arena_buf
  // buf_start_address + current_allocated_size gives 
  // an address to next unused space.
  void *mem_buf = current->arena_buf + current->allocated_size;
  current->allocated_size += size;
  return mem_buf;  
}

// To get an overview of the arena.
void arena_visualize(const Arena *arena) {
  const Arena *current = arena;
  while(current != NULL) {
    printf("capacity: %llu, size: %llu, buf: %p, nxt: %p\n",
           current->capacity,
           current->allocated_size,
           current->arena_buf,
           current->next_node);
    current = current->next_node;
  }
}

// resets the allocated sizes to 0.
// doesn't actually frees any memory.
void arena_reset(Arena *arena) {
  Arena *current = arena;
  while(current != NULL) {
    current->allocated_size = 0;
    current = current->next_node;
  }
}

// Deallocates the entire arena.
void arena_free(Arena *arena) {
  Arena *current = arena;
  Arena *next;
  while(current != NULL) {
    free(current->arena_buf);
    current->allocated_size = 0;
    current->capacity = 0;
    next = current->next_node;
    free(current);
    current = next;
  }
}
