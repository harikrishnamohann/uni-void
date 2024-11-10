#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#include "arena.h"

// to toggle error messages.
#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#endif

typedef struct String String;

// Reads given character sequence file int *read_content.
// Returns 0 on success and -1 on failures.
int8_t read_file_content(const char* filename, String* read_content);

// Writes given content to filename.
// returns 0 on success and -1 on failure.
int8_t write_to_file(const char* filename, const String content);

#endif
