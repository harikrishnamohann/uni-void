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

// Either the ownership or cloned (strdup()) version can be passed.
// The caller is responsible for freeing the memory.
String str_init(char* str);

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
uint64_t str_contains(const String str, const String key);

// Replaces all occurrences of `key` with `value` in `str`
// Modifies `str` in place, resizing it as necessary; caller must free result
// Returns 0 on success, -1 on failure and 2 if key is not in given str.
int8_t str_replace_all(String* str, const String key, const String value);

/*
    Replaces first occurre of `key` with `value` in `str`
    Modifies `str` in place, resizing it as necessary; caller must free result
    It uses a static variable to keep track of the position.
    So, calling this function with a different key will reset 
    its memory. Point is, it might not work as expected when calls
    are concurrent.
    Returns 0 on success, -1 on failure and 2 if key is not in given str.
*/
int8_t str_replace_next(String* str, const String key, const String value);

// frees the memory allocated for String.
void str_free(String* string);

#endif
