#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/utils.h"

typedef struct String {
  char* str;
  uint64_t length;
} String;

String str_init(char* str) {
  return  (str != NULL) ? (String){str, strlen(str)} : (String){NULL, 0};
}

char* str_str(const String s) {
  return s.str;
}

uint64_t str_len(const String s) {
  return s.length;
}

void str_free(String* s) {
  memset(s->str, 0, s->length);
  free(s->str);
  s->length = 0;
}

char* str_slice(const String str, uint64_t start, uint64_t end) {
  String slice = str_init(NULL);

  if (start == end || end >= str.length || start >= str.length || start < 0 || end < 0) return NULL;

  slice.str = (char*)malloc(end - start + 1);
  if (slice.str == NULL) {
    DEBUG_PRINT("err! str_slice(): failed to allocate memory for slice.str.\n");
    return NULL;
  }

  for (int i = start; i < end; i++) {
    slice.str[slice.length++] = str.str[i];
  }
  slice.str[slice.length] = '\0';
  return slice.str;
}

// Returns number of occurances of key in str if key is in str; 0 otherwise.
uint64_t str_contains(const String str, const String key) {
  size_t count = 0, k, j;
  for (size_t i = 0; i < str_len(str); i++) {
    for (j = 0, k = i; j < str_len(key) && k < str_len(str) && str.str[k] == key.str[j]; k++, j++);
    if (j == str_len(key)) count++;
  }
  return count;
}

int8_t str_replace_all(String* str, const String key, const String value) {
  String buf = str_init(NULL);
  size_t j, k, key_count = str_contains(*str, key);

  if (key_count != 0) {
    buf.str = (char*)malloc(str->length - (key.length - value.length) * key_count);
    if (buf.str == NULL) {
      DEBUG_PRINT("err! str_replace_all(): failed to allocate memory for buf.str.\n");
      return -1;
    }

    for (size_t i = 0; i < str->length; i++) {
      for (j = 0, k = i; j < key.length && k < str->length && str->str[k] == key.str[j]; k++, j++);
      if (j == key.length) {
        for (j = 0; j < value.length; j++) {
          buf.str[buf.length++] = value.str[j];
        }
        i += key.length - 1;
      } else {
       buf.str[buf.length++] = str->str[i]; 
      }
    }
    buf.str[buf.length] = '\0';
  } else {
    DEBUG_PRINT("str_replace_all(): key not found.\n");
    return 2;
  }

  str_free(str);
  *str = buf;

  return 0;
}

/*
Returns the starting index of next occrance of key in str.
It uses a static variable to keep track of the position.
So, calling this function with a different key will reset 
its memory. Point is, it will not work as expected in a
multi-threaded context.
Returns starting index of key in str; -1 if key is not in str.
*/
static int64_t str_has_next(const String str, const String current_key) {
  size_t k, j;
  static String key = {NULL, 0};
  static size_t next_pos = 0;
  if (str_len(key) == 0) {
    key = current_key;
  } else if (strcmp(str_str(key), str_str(current_key)) != 0) {
    key = current_key;
    next_pos = 0;
  } 
  for (size_t i = next_pos; i < str_len(str); i++) {
    for (j = 0, k = i; j < str_len(key) && k < str_len(str) && str.str[k] == key.str[j]; k++, j++);
    if (j == str_len(current_key)) {
      next_pos = k;
      return i;
    }
  }
  return -1;
}

int8_t str_replace_next(String* str, const String key, const String value) {
  int64_t replace_pos = str_has_next(*str, key);
  String buf = str_init(NULL);
  if (replace_pos >= 0) {
    buf.str = (char*)malloc(str_len(*str) - str_len(key) + str_len(value));
    if (buf.str == NULL) {
      DEBUG_PRINT("err! str_replace_next(): failed to allocate memory for buf.str.\n");
      return -1;
    }

    for (size_t i = 0; i < replace_pos; i++) {
      buf.str[buf.length++] = str->str[i];
    }
    for (size_t i = 0; i < str_len(value); i++) {
      buf.str[buf.length++] = value.str[i];
    }
    for (size_t i = buf.length - str_len(value) + str_len(key); i < str_len(*str); i++) {
      buf.str[buf.length++] = str->str[i];
    }

    buf.str[buf.length] = '\0';
    str_free(str);
    *str = buf;
    return 0;
  }
  return 2;
}
