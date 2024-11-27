/*
This is my implementation of String type in c.
It has various utility functions to handle various operations.

## About ##
Author: Harikrishna Mohan
Contact: harikrishnamohan@proton.me
Started on: 11-11-2024
*/

#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct String {
    char* str;
    uint64_t length;
} String;

// The caller is responsible for freeing the memory.
// Creates an instance of String type
String str_from(const char* str);

// Returns the char* from String.
char* str_str(const String s);

// Returns length of String.
uint64_t str_len(const String s);

// Frees the memory allocated for a String object and resets its fields
void str_free(String* s);

// Creates a substring (slice) from the specified start and end indices
// Returns a new String containing the slice; caller must free it
String str_slice(const String str, uint64_t start, uint64_t end);

// Returns number of occurances of key in str if key is in str; 0 otherwise.
uint64_t str_key_frequency(const String str, const String key);

// Replaces all occurrences of `key` with `value` in `str`
// Modifies `str` in place, resizing it as necessary; caller must free result
// Returns 0 on success, -1 on failure and 2 if key is not in given str.
int8_t str_replace_all(String* str, const String key, const String value);

// Returns -1 on failure or the ending position of current replacement on success.
// ## Parameters: 
    // str: String to perform replace operation.
    // look_from: a starting position to search key.
    // key: String to be replaced.
    // value: String to replace.
int64_t str_replace_next(String* str, uint64_t look_from, const String key, const String value);

// Returns a composed string, ie; works like sprintf(),
// but returns the output as a string type.
// Returns an empty string on error
// possible errors are format errors and memory allocation failure.
String str_compose(const char *format, ...);

// frees the memory allocated for String.
void str_free(String* string);

#endif
