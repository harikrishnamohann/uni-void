#ifndef UNI_VOID_H
#define UNI_VOID_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../include/arena.h"

// file for storing leaderboard info
#define LEADERBOARD_FILE "game_files/leaderboard.csv"

// location for serializing game state.
#define STATE_FILE "game_files/game_state.bin"

// size of stack
#define STK_SIZE 60 

// these macros evaluate mid point of stdscr based on given offset
#define CENTER_Y(offset) (((LINES - (offset)) / 2))
#define CENTER_X(offset) (((COLS - (offset)) / 2))

// used to adjust the order of matrix based on the selected game mode
#define MODE_OFFSET 2

// defines maximum moves for hard mode
#define HARD_MODE_MOVE_LIMIT 300

typedef enum {
  key_invalid,
  key_up = 1,
  key_down = -1, // inverse of key_up (for undo-redo mechanism)
  key_left = 2,
  key_right = -2, // inverse of key_left
  key_enter = 3,
  key_undo,
  key_redo,
  key_exit,
  key_resize,
  key_usage,
  key_force_quit,
} Key;

typedef enum {
  mode_load,
  mode_easy,
  mode_normal,
  mode_hard,
  mode_custom,
  mode_exit,
} Mode;

typedef enum {
  count_stop, // indicates to stop counting
  count_up, // indicate increment the move counter
  count_down, // indicate decrement move counter
} Counter;

struct status_line {
  size_t moves;
  char *msg;
  Key key;
};

struct game_state {
  uint16_t order; // order of square matrix
  uint16_t mode; // easy, normal, hard modes
  uint16_t curs_x; // x-coordinate of void-tile
  uint16_t curs_y; // y-coordinate of void-tile
  uint16_t moves; // moves counter
  Counter count_ctrl; // specify to use an up-counter or a down-counter
  int16_t utop; // stack pointers
  int16_t rtop;
  Key undo_stack[STK_SIZE];
  Key redo_stack[STK_SIZE];
  int** mat; // our puzzle matrix
};

// main.c
struct game_state game_state_init(Arena* arena, int order);
struct status_line status_line_init(char* msg);


// utils.c
// swaps x and y using xor operation.
void swap(int *x, int *y);

// push key into stack
void push_key(Key stk[], int16_t *top, Key key);

// pop key from stack
Key pop_key(Key stk[], int16_t *top);

// creates an array of whole numbers up to specified size and arranges them in random order.
void make_radomized_array(int* arr, size_t size);

// update move counter based on count_ctrl variable
void update_moves(struct game_state* gs);

// query the user and returns input of the query.
char* input_str(const char* query);

// display help dialog on a small window
void display_usage();

// keymap.c
Key decode_key(int ch);



// save_and_load.c
void save_game_state(struct game_state* gs);
struct game_state load_game_state(Arena* arena);



// leaderboard.c
void display_leaderboards(const struct game_state* gs, char* name);

#endif
