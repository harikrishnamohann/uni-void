/*
todo: a better memory allocator that can handle reallocations.
*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../include/utils.h"
#include <stdio.h>

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

String str_slice(const String str, uint64_t start, uint64_t end) {
  String slice = str_init(NULL);

  if (start == end || end >= str.length || start >= str.length || start < 0 || end < 0) return slice;

  slice.str = (char*)malloc(end - start + 1);
  if (slice.str == NULL) {
    DEBUG_PRINT("err! str_slice(): failed to allocate memory for slice.str.\n");
    return str_init(NULL);
  }

  for (int i = start; i < end; i++) {
    slice.str[slice.length++] = str.str[i];
  }
  slice.str[slice.length] = '\0';
  return slice;
}

static size_t get_key_count(const String str, const String key) {
  size_t count = 0, k, j;
  for (size_t i = 0; i < str_len(str); i++) {
    for (j = 0, k = i; j < str_len(key) && k < str_len(str) && str.str[k] == key.str[j]; k++, j++);
    if (j == str_len(key)) count++;
  }
  return count;
}

int8_t str_replace_all(String* str, const char* key_to_rpl, const char* value_to_repl) {
  String key = str_init((char*)key_to_rpl);
  String value = str_init((char*)value_to_repl);
  String buf = str_init(NULL);
  size_t j, k, key_count = get_key_count(*str, key);

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
  *str = str_init(buf.str);

  return 0;
}

