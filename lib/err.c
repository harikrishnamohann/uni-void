#pragma once

#include <stdlib.h>
#include <stdio.h>

#define WARN 0
#define TERMINATE 1
#define LOG 2

#define PROCEED 0
#define RECONSIDER -1
#define HALT -2

typedef enum {
  INDEX_OUT_OF_BOUNDS = 1,
  MALLOC_FAILURE, // terminating point
  NULL_REFERENCE,
  INVALID_SIZE_ERR,
  ARITHMETIC_ERR,
  RESIZE_ERR,
  FILE_NOT_FOUND,
  IO_ERR,
} err_t;

#define debug_raise_err(errcode, msg) raise_err(errcode, DEBUG_ACTION, __FILE__, __FUNCTION__, __LINE__, msg)

void raise_err(err_t code, int action, const char* file_name, const char* fn, int line, const char* msg) {
  if (code == PROCEED) return;
  printf("\033[0;31m");
  if (action == WARN) {
    printf("WARNING!");
  } else {
    printf("ERRORO!");
  }
  printf("\033[0m %s :: %s() :: line %d\n", file_name, fn, line);
  switch (code) {
    case IO_ERR :
      printf("Input/Output failure\n");
      break;
    case FILE_NOT_FOUND :
      printf("File does not exist.\n");
      break;
    case INDEX_OUT_OF_BOUNDS :
      printf("You've tried to access an out of bound index.\n");
      break;
    case MALLOC_FAILURE :
      printf("memory allocation failure!\n");
      exit(EXIT_FAILURE);
    case NULL_REFERENCE :
      printf("Invalid pointer reference.\n");
      break;
    case INVALID_SIZE_ERR :
      printf("you've tried to create an entity with 0 or negative size.\n");
      break;
    case ARITHMETIC_ERR :
      printf("An arithmetic error occured.\n");
      break;
    case RESIZE_ERR :
      printf("you tried to resize a non-resizeable reference.\n");
      break;
  }
  if (msg != NULL) {
    printf("msg: %s\n", msg);
  }
  if (action == TERMINATE) {
    printf("\033[0;31mPROGRAM TERMINAED \033[0m\n");
    exit(EXIT_FAILURE);
  }
}
