/*
 * Error handling utilities.
 *
 * Provides a minimal mechanism for propagating execution status and
 * formatted error messages using a thread-local buffer.
 *
 * Status codes:
 *   OK   – success
 *   BAD  – recoverable error (caller should handle)
 *   HALT – unrecoverable error (program should exit)
 *
 * Macros are provided for setting status codes, writing messages,
 * printing error state, and conditionally exiting on HALT.

 * It is recomended to define an error buffer for each library or file
 * as your project scale.

 * Author: Harikrishna Mohan
 * Date: April-22-2025
 */

#pragma once

#include <stdlib.h>

#define ERR_BUF_SIZE 256
_Thread_local signed char err_buf[ERR_BUF_SIZE];

#ifndef OK
// indicates execution success
#define OK 0
#endif
#ifndef BAD
// indicates recoverable errors. The caller should deal with it.
#define BAD -1
#endif
#ifndef HALT
// indicates the program should terminate
#define HALT -2
#endif

// writes OK to buf and return val
#define return_ok(buf, val) do {\
                              *buf = OK;\
                              return val;\
                            } while(0)

// writes BAD to *buf and update err_msg to buf + 1 and return val
#define return_bad(buf, val, fmt, ...) do {\
                                         *buf = BAD; \
                                         snprintf((char*)buf + 1, ERR_BUF_SIZE - 1, fmt, __VA_ARGS__);\
                                         return val;\
                                       } while (0)

// writes HALT to *buf and update err_msg to buf + 1 and return val
#define return_halt(buf, val, fmt, ...) do {\
                                         *buf = HALT; \
                                         snprintf((char*)buf + 1, ERR_BUF_SIZE - 1, fmt, __VA_ARGS__);\
                                         return val;\
                                       } while (0)

//  Prints out the err buf in style.
#define err_status(buf) do { \
                         if (*buf) { \
                           printf("%s::{ \e[31m%s\e[0m } => \"%s\"\n", #buf, (*buf == BAD) ? "BAD" : "HALT", buf + 1); \
                         } else {\
                           printf("%s::{ \e[32mOK\e[0m }\n", #buf); \
                         } \
                       } while(0)

// executes function call and exit if HALT status is found in the associated err buffer of the call
#define err_expect(buf, call_associated_with_buf) call_associated_with_buf; \
                         do { \
                         if (*buf == HALT) { \
                           printf("%s::{ \e[31mHALT\e[0m } => \"%s\"\nexit.\n", #call_associated_with_buf, buf + 1); \
                           exit(EXIT_FAILURE); \
                         } else if (*buf == BAD) { \
                           printf("%s::{ \e[31mBAD\e[0m } => \"%s\"\n", #call_associated_with_buf, buf + 1); \
                         } \
                       } while(0)
