#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

// to toggle error messages.
#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#endif

char *str_slice(const char *str, uint64_t start, uint64_t end);

#endif
