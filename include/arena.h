#ifndef __ARENA_H__
#define __ARENA_H__

#include <stdint.h>

#define ARENA_16 16
#define ARENA_32 32
#define ARENA_64 64
#define ARENA_128 128
#define ARENA_256 256
#define ARENA_512 512
#define ARENA_1024 1024

// Prototype of Arena
typedef struct Arena Arena;

// initializes the arena chunk with a capacity of 
// ARENA_[8,16,32,..,2048] or any custom integer greater than 0.
Arena *arena_init(uint64_t capacity);

// Returns required size of memory from the arena to use.
// Returns NULL if the requested size is more than its capacity.
void *arena_alloc(Arena *arena, uint64_t size);

// To get an overview of the arena.
void arena_visualize(const Arena *arena);

// resets the allocated sizes to 0.
// doesn't actually frees any memory.
void arena_reset(Arena *arena);

// Deallocates the entire arena.
void arena_free(Arena *arena);

#endif
