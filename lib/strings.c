#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/utils.h"

typedef struct String {
  char* str;
  uint64_t length;
} String;

String str_from(const char* str) {
  return (str != NULL) ? (String){strdup(str), strlen(str)} : (String){NULL, 0};
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
  String slice = str_from(NULL);

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
uint64_t str_key_frequency(const String str, const String key) {
  size_t count = 0, k, j;
  for (size_t i = 0; i < str_len(str); i++) {
    for (j = 0, k = i; j < str_len(key) && k < str_len(str) && str.str[k] == key.str[j]; k++, j++);
    if (j == str_len(key)) count++;
  }
  return count;
}

int8_t str_replace_all(String* str, const String key, const String value) {
  String buf = str_from(NULL);
  size_t j, k, key_count = str_key_frequency(*str, key);

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

int64_t str_replace_next(String* str, uint64_t look_from, const String key, const String value) {
  int64_t start_pos = -1;
  size_t k, j;
  for (size_t i = look_from; i < str->length; i++) {
    for (j = 0, k = i; j < key.length && k < str->length && str->str[k] == key.str[j]; k++, j++);
    if (j == key.length) {
      start_pos = i;
      break;
    }
  }

  String buf = {NULL, 0};
  if (start_pos != -1) {
    buf.str = (char*)malloc(str->length - key.length + value.length + 1);
    if (buf.str == NULL) {
      DEBUG_PRINT("err! str_replace_next(): failed to allocate memory for buf.str.\n");
      return -1;
    }

    for (size_t i = 0; i < start_pos; i++) {
      buf.str[buf.length++] = str->str[i];
    }
    for (size_t i = 0; i < value.length; i++) {
      buf.str[buf.length++] = value.str[i];
    }
    for (size_t i = buf.length - value.length + key.length; i < str->length; i++) {
      buf.str[buf.length++] = str->str[i];
    }

    buf.str[buf.length] = '\0';
    str_free(str);
    *str = buf;
    return start_pos + value.length;
  }
  return -1;
}
