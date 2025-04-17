/*
 * This is a string library that I implemented for having a better time
 * messing with C. I tried to implement most of the basic String operations
 * I can think of. I also implemented some features for the love of writing
 * string parsers. To be clear, all the operations related to the offset
 * parameter of String type is built up on my limited knolowed about building
 * simple parsers.
 *
 * In this implementation, strings are not meant to be null-terminated.
 * String's length is available in the type itself for concluding the end.
 * There are two kinds of strings:
 * 1. Scalable: Scalable strings can resize itself like Vector in C++;
 *    Example: String s = str_declare(SCALABLE);
 * 2. Non-scalable: These are strings with fixed size.
 *    Example: String buffer = str_declare(128);
 *
 * Most functions rely on `debug_raise_err()` (defined in "err.c") for error
 * reporting and return status codes defined in err.c.
 *
 **************************************************************************
 * [IMPORTANT NOTE!] Memory management:
 * Memory allocated using any of the function that returns a String type
 * must be released by calling str_free(). 
 **************************************************************************
 *
 *
 * ## Function Reference ##
 *
 * ### Creation and Initialization ###
 *
 * String str_declare(uint64_t capacity)
 * Initializes an empty string with the specified capacity.
 * `capacity = SCALABLE` enables automatic scaling.
 *
 * String str_init(const char* s)
 * Initializes a String from a null-terminated C string.
 *
 * ### Insertion and Deletion ###
 *
 * int str_insert(String* str, int64_t index, char ch)
 * Inserts a character at the specified index (supports negative indexing).
 * Resizes the string exponentially if it is scalable. Returns status code.
 *
 * char str_remove(String* str, int64_t index)
 * Removes and returns the character at the specified index (supports negative indexing).
 * Returns RECONSIDER on error.
 *
 * ### String Manipulation ###
 *
 * String str_dup(const String* s)
 * Returns a deep copy of the given string. The caller is responsible for freeing the memory.
 *
 * String str_join(String a, String b)
 * Returns a new string by concatenating `a` and `b`.
 * The result is scalable if either `a` or `b` is scalable.
 * The caller is responsible for freeing the memory.
 *
 * int str_concat(String* dest, const String src)
 * Appends the contents of `src` to the end of `dest`.
 * Requires `dest` to be scalable. Returns status code.
 *
 * int str_copy(String* dest, const String src)
 * Replaces the contents of `dest` with the contents of `src`.
 * Resizes `dest` if needed. Requires `dest` to be scalable. Returns status code.
 *
 * int str_replace_first(String* str, int start, const char* key, uint32_t key_len, const char* target, uint32_t target_len)
 * Replaces the first occurrence of `key` within `str` (starting from `start`)
 * with `target`. Resizes `str` if necessary. Requires `str` to be scalable.
 * Returns the index after the replacement or RECONSIDER on error.
 *
 * void str_replace_all(String* str, const char* key, uint32_t key_len, const char* replace_with, uint32_t val_len)
 * Replaces all occurrences of `key` within `str` with `replace_with`.
 * Requires `str` to be scalable.
 *
 * ### Substring Operations ###
 *
 * const String str_slice(const String* str, uint64_t start, uint64_t end)
 * Returns a non-owning reference (view) into a substring of a non-scalable string.
 * No new memory is allocated.
 *
 * String str_owned_slice(String* s, uint64_t start, uint64_t end)
 * Returns an owning reference to a substring of a non-scalable String.
 * The extracted slice is moved to the beginning of the original string,
 * and the original string's internal pointer is offset.
 * The caller can free either the slice or the original string once.
 *
 * ### String Comparison ###
 *
 * int str_cmp(const String a, const String b)
 * Performs a lexicographical comparison between `a` and `b`.
 * Returns 0 if equal, >0 if `a` > `b`, and <0 if `a` < `b`.
 *
 * ### Searching ###
 *
 * int64_t str_contains(const String src, const char* key, uint64_t key_len)
 * Returns the index of the first occurrence of `key` within `src`,
 * or RECONSIDER if `key` is not found.
 *
 * ### String Formatting ###
 *
 * String str_compose(const char* fmt, ...)
 * Returns a non-scalable formatted String, similar to `sprintf`.
 * The caller is responsible for freeing the memory.
 *
 * ### Type Conversion ###
 *
 * int64_t str_to_int64(const String s)
 * Converts the string `s` to an `int64_t`.
 * Returns RECONSIDER if the input string is not a valid integer.
 *
 * double str_to_double(const String s)
 * Converts the string `s` to a `double`.
 * Returns RECONSIDER on failure (e.g., invalid input).
 *
 * char*  str_to_cstring(const String* s)
 * Returns null terminated c string. you have to use free() on returned pointer.
 *
 * ### Utility Functions ###
 *
 * void str_offset(String* s, int64_t offset)
 * Moves the internal pointer to a relative position based on the specified offset.
 *
 * void str_rewind(String* s)
 * Rewinds the internal pointer of an offset string back to the beginning (offset 0).
 *
 * void str_free(String* str)
 * Frees the memory allocated for the string and resets its metadata.
 *
 * ### Macro Shorthands ###
 *
 * int64_t str_contains_using_str(const String src_str, const String key_str)
 * A shorthand for `str_contains()` that accepts String types for both source and key.
 *
 * int str_replace_first_using_str(String* str_ptr, int start, String key_str, String target_str)
 * A shorthand for `str_replace_first()` that accepts String types for key and target.
 *
 * void str_replace_all_using_str(String* str_ptr, String key_str, String target_str)
 * A shorthand for `str_replace_all()` that accepts String types for key and replacement.
 *
 * Author: Harikrishna Mohan
 * Date: April-11-2025
 */

#pragma once

#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "err.c"
#define DEBUG_ACTION WARN

#define SCALABLE 0
#define STR_BEGIN 0
#define STR_END -1

// scale factor for scalable strings
#define SCALE_FACTOR 2.0

typedef struct {
  char* str;
  uint64_t capacity;
  uint64_t length;
  bool scalable;
  int64_t offset;
} String;

uint64_t str_len(const char* str) {
  uint64_t len = 0;
  while (str[len] != '\0') len++;
  return len;
}

// Initializes an empty string with the specified capacity.
// If capacity == SCALABLE, it enables automatic scaling.
// Caller must free it using str_free().
String str_declare(uint64_t capacity) {
  if (capacity < 0) {
    debug_raise_err(INVALID_SIZE_ERR, "invalid string capacity");
  }
  String s;
  s.length = 0;
  s.offset = 0;
  if (capacity == SCALABLE) {
    capacity = 1;
    s.scalable = true;
  } else {
    s.scalable = false;
  }
  s.capacity = capacity;
  s.str = (char*)malloc(sizeof(char) * capacity);
  if (s.str == NULL) {
    debug_raise_err(MALLOC_FAILURE, NULL);
  }
  return s;
}

// Initializes a String from a null-terminated C string.
String str_init(const char* s) {
  String str = str_declare(str_len(s));
  str.length = str.capacity;
  for (int i = 0; i < str.length; i++) str.str[i] = s[i];
  return str;
}

void _str_scale(String* s, float scale_factor) {
  s->str = realloc(s->str, (uint64_t)(s->capacity * scale_factor));
  if (s->str == NULL) {
    debug_raise_err(MALLOC_FAILURE, "Failed to scale string!");
  }
  s->capacity *= scale_factor;
}

// Inserts a character at the specified index (supports negative indexing).
// Resizes the string exponentially if the string is scalable.
 int str_insert(String* s, int64_t pos, char ch) {
  // validate pos
  if (pos < 0) { // for negative index access
    pos = s->length + pos + 1;
  }
  if (pos > s->length || pos < 0) { // check for invalid pos
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "invalid access positon");
    return RECONSIDER;
  }
  // deal with string capacity
  if (s->length >= s->capacity) { // string is full
    if (s->scalable) { // reallocate if the user want an arraylist
      _str_scale(s, SCALE_FACTOR);
    } else { // not possible to add more characters
      debug_raise_err(INDEX_OUT_OF_BOUNDS, "str is not scalable");
      return RECONSIDER;
    }
  }
  // assign ch to required pos
  for (int64_t i = s->length - 1; i >= pos; i--) {
    s->str[i + 1] = s->str[i];
  }  

  s->str[pos] = ch;
  s->length++;
  return PROCEED;
}

// Removes and returns the character at the specified index.
// Supports negative indexing.
char str_remove(String* s, int64_t pos) {
  if (s->length <= 0) {
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "cannot remove from an empty string.");
    return RECONSIDER;
  }
  if (pos < 0) { // normalize index
    pos = s->length + pos;
  }
  if (pos > s->length || pos < 0) { // check for invalid pos
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "invalid access positon");
    return RECONSIDER;
  }

  char ch = s->str[pos];
  s->length--;
  for (int64_t i = pos; i < s->length; i++) {
    s->str[i] = s->str[i + 1];
  }
  return ch;
}

// Moves the pointer to a relative position based on specified offset.
void str_offset(String* s, int64_t offset) {
  if (offset > (signed)s->length || offset * -1 > (signed)s->offset) {
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "invalid offset");
    return;
  }
  s->length -= offset;
  s->offset += offset;
  s->str += offset;
}

// rewinds offseted string back to 0
void str_rewind(String* s) {
  s->length += s->offset;
  s->str -= s->offset;
  s->offset = 0;
}

// Returns a deep copy of the given string. Caller must free it.
String str_dup(const String* s) {
  String dup = str_declare(s->capacity);
  if (dup.capacity == 0) {
    debug_raise_err(NULL_REFERENCE, "failed to create duplicate");
    return dup;
  }
  dup.length = s->length;
  dup.scalable = s->scalable;
  for (int i = 0; i < s->length; i++) dup.str[i] = s->str[i];
  return dup;
}

// Returns a view (non-owning reference) into a non-scalable substring.
// No memory is allocated. Useful for efficient slicing.
// you can either pass the slice or the original string into free once.
String str_slice(const String* s, uint64_t start, uint64_t end) {
  if (end > s->length || start >= end) {
    printf("length: %lu, start: %lu, end: %lu\n", s->length, start, end);
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "incorrect slice length");
    return (String) {NULL, 0, 0};
  }
  return (String) {
    .length = end - start,
    .str = s->str + start,
    .capacity = s->capacity,
    .scalable = false,
    .offset = s->str - s->str + start,
  }; 
}

// Returns an owned reference into a non-scalable String.
// The Returned slice will be removed from original string.
// No memory is allocated.
// you can either pass the slice or the original string into str_free() once.
String str_owned_slice(String* s, uint64_t start, uint64_t end) {
  if (end > s->length || start >= end) {
    printf("length: %lu, start: %lu, end: %lu\n", s->length, start, end);
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "incorrect slice length");
    return (String) {NULL, 0, 0};
  }

  String slice = {
    .str = s->str,
    .length = end - start,
    .capacity = end - start,
    .offset = s->offset,
    .scalable = false
  };

  for (int i = 0; i < slice.length; i++) {
    char tmp = slice.str[start + i];
    for (int j = start + i; j > i; j--) {
      slice.str[j] = slice.str[j - 1];
    }
    slice.str[i] = tmp;
  }

  str_offset(s, slice.length);
  return slice;
}

// Returns a new string by concatenating `a` and `b`.
// The resultant is scalable if either a or b is scalable.
// Caller must free the memory allocated for return value.
String str_join(String a, String b) {
  String result = str_declare(a.capacity + b.capacity); 
  result.length = a.length + b.length;
  result.scalable = a.scalable || b.scalable;
  int j = 0;
  for (int i = 0; i < a.length; i++, j++) result.str[j] = a.str[i];
  for (int i = 0; i < b.length; i++, j++) result.str[j] = b.str[i];
  return result;
}

// Appends `src` to `dest`. Requires `dest` to be scalable.
int str_concat(String *dest, const String src) {
  if (!dest->scalable) {
    debug_raise_err(RESIZE_ERR, "dest is not scalable"); 
    return HALT;
  }
  dest->capacity += src.capacity;
  dest->str  = realloc(dest->str, sizeof(char) * dest->capacity);
  if(dest->str == NULL) {
    debug_raise_err(MALLOC_FAILURE, NULL);
  }
  for (int i = dest->length, j = 0; j < src.length; i++, j++) dest->str[i] = src.str[j];
  dest->length += src.length;
  return 0;
}

// Replaces contents of `dest` with `src`. scales if needed.
// Requires `dest` to be scalable.
int str_copy(String *dest, const String src) {
  if (!dest->scalable) {
    debug_raise_err(RESIZE_ERR, "dest is not scalable"); 
    return HALT;
  }
  if (dest->capacity < src.length) {
    dest->capacity += src.length - dest->capacity;
    dest->str = realloc(dest->str, dest->capacity + 1);
    if (dest->str == NULL) {
      debug_raise_err(MALLOC_FAILURE, "realloc() failed.");
      return -1;
    }
  }
  for (int i = 0; i < src.length; i++) {
    dest->str[i] = src.str[i];
  }
  dest->length = src.length;
  return 0;
}

// Lexicographical comparison: 
// returns 0 if equal, >0 if a > b, <0 if a < b.
int str_cmp(const String a, const String b) {
  for (int i = 0; i < a.length && i < b.length; i++) {
    if (a.str[i] != b.str[i]) return a.str[i] - b.str[i];
  }
  return a.length - b.length;
}

// Returns a non-scalable formatted String (like sprintf).
// Caller must free.
String str_compose(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int req_length = vsnprintf(NULL, 0, fmt, args); // calculate length of the string
    va_end(args);

    if (req_length < 0) {
        debug_raise_err(MALLOC_FAILURE, "Error in vsnprintf format");
    }

    String composed = str_declare(req_length + 1);

    va_start(args, fmt);
    vsnprintf(composed.str, composed.capacity, fmt, args);
    va_end(args);

    composed.length = req_length;
    return composed;
}

// Returns the index of the first occurrence of `key` in `src`, or RECONSIDER.
int64_t str_contains(const String src, const char* key, uint64_t key_len) {
  int pos;
  for (int i = 0; i < src.length; i++) {
    for (pos = 0; pos < key_len && src.str[i + pos] == key[pos]; pos++);
    if (pos == key_len) {
      return i;
    }
  }
  return RECONSIDER;
}

// Converts the string to int64_t. Returns RECONSIDER if invalid input.
static void print_invalid_number_err_msg(const String s, int i) {
    for (int j = 0; j < s.length; j++) {
      if (j == i) {
        printf("\033[0;31m%c\033[0m", s.str[i]);
      } else {
        printf("%c", s.str[j]);
      }
    }
    printf("\n");
    for (int j = 0; j < i; j++) printf(" ");
    printf("^ invalid character found.\n");
}

// Converts the string to a double. Returns RECONSIDER on failure.
int64_t str_to_int64(const String s) {
  int8_t sign = 1;
  int i = 0;
  int64_t result = 0;
  if (s.length == 0) return 0;

  if (s.str[0] == '-') {
    sign = -1;
    i++;
  } else if (s.str[0] == '+') {
    i++;
  }
  while (i < s.length && s.str[i] >= '0' && s.str[i] <= '9') {
    result = result * 10 + (s.str[i] - '0');
    i++;
  }
  if (i != s.length){
    debug_raise_err(ARITHMETIC_ERR, "error converting to double: invalid character found.");
    print_invalid_number_err_msg(s, i);
    return RECONSIDER;
  }
  return result * sign;
}

// converts string to double
double str_to_double(const String s) {
  double result = 0.0;
  int8_t sign = 1;
  uint32_t i = 0;
  if (s.length == 0) return 0.0;

  if (s.str[0] == '-') {
    sign = -1;
    i++;
  } else if (s.str[0] == '+') {
    i++;
  }

  while (i < s.length && s.str[i] >= '0' && s.str[i] <= '9') {
    result = result * 10.0 + (s.str[i] - '0');
    i++;
  }
  if (i < s.length && s.str[i] == '.') {
    i++;
    double fraction = 0.1;
    while (i < s.length && s.str[i] >= '0' && s.str[i] <= '9') {
      result += (s.str[i] - '0') * fraction;
      fraction /= 10.0;
      i++;
    }
  }

  if (i != s.length){
    debug_raise_err(ARITHMETIC_ERR, "error converting to double: invalid character found.");
    print_invalid_number_err_msg(s, i);
    return RECONSIDER;
  }
  return result * sign;
}

// Replaces the first occurrence of `key` (after `start`) with `target`.
// Scales if necessary.
// Requires str to be scalable.
int str_replace_first(String* s, int start, const char* search_key, uint32_t key_len, const char* target, uint32_t target_len) {
  if (!s->scalable) {
    debug_raise_err(RESIZE_ERR, "dest is not Scalable type"); 
    return HALT;
  }
  if (start < 0 || start >= s->length) {
    debug_raise_err(INDEX_OUT_OF_BOUNDS, "invalid start index");    
    return RECONSIDER;
  }

  uint32_t span_start; // to store begin index of key in s
  int8_t contains = 0;
  for (int i = start; i < s->length; i++) {
    for (span_start = 0; span_start < key_len && s->str[i + span_start] == search_key[span_start]; span_start++);
    if (span_start == key_len) {
      span_start = i;
      contains = 1;
      break;
    }
  }

  if (!contains) { // return if key is not present in s
    return RECONSIDER;
  }

  if (str_cmp((String){(char*)search_key, key_len, key_len}, (String){(char*)target, target_len, target_len}) == 0) {
    goto ret; // no need of replacement if key and value are same. just return.
  }

  uint32_t span_end = span_start + key_len - 1;
  int32_t diff = key_len - target_len;
  if (diff > 0) { // target string is shorter than search_key
    s->length -= diff;
    for (int j = span_start + target_len; j < s->length; j++) {
      s->str[j] = s->str[j + diff]; // shift characters to the left
    }
  } else if (diff < 0) { // target string is longer than search_key
    diff *= -1;
    if (s->length + diff > s->capacity) {
      _str_scale(s, 1.5);
      s->length += diff;
      for (int j = s->length - 1; j >= span_end; j--) {
        s->str[j + diff] = s->str[j];
      } 
    }
  }

  for (int i = span_start, j = 0; j < target_len; i++, j++) {
    s->str[i] = target[j];
  }

  ret:
    return span_start + target_len - 1;
}

// Replaces all occurrences of `key` with `replace_with`.
void str_replace_all(String* s, const char* search_key, uint32_t key_length, const char* replace_with, uint32_t val_length) {
  int pos = 0;
  while ((pos = str_replace_first(s, pos, search_key, key_length, replace_with, val_length)) != -1);
}


// Frees the memory allocated for the string and resets metadata.
void str_free(String* s) {
  s->capacity = 0;
  s->length = 0;
  free(s->str - s->offset);
  s->offset = 0;
  s->str = NULL;
}

// shorthand of str_replace_first() using String types.
#define str_replace_first_using_str(str_ptr, start, key_str, target_str) \
  str_replace_first(str_ptr, start, key_str.str, key_str.length, target_str.str, target_str.length)

// shorthand of str_replace_all() using String types.
#define str_replace_all_using_str(str_ptr, key_str, target_str) \
  str_replace_all(str_ptr, key_str.str, key_str.length, target_str.str, target_str.length)

// shorthand of str_contains() using String types.
#define str_contains_using_str(src_str, key_str) \
  (str_contains(src_str, key_str.str, key_str.length))
