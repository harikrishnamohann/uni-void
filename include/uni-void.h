#ifndef UNI_VOID_H
#define UNI_VOID_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/arena.h"

#define STK_SIZE 60
#define CENTER_Y(offset) (((LINES - (offset)) / 2))
#define CENTER_X(offset) (((COLS - (offset)) / 2))

#define HARD_MODE_MOVE_LIMIT 300

typedef enum {
  key_invalid,
  key_up = 1,
  key_down = -1,
  key_left = 2,
  key_right = -2,
  key_return = 3,
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
  count_stop,
  count_up,
  count_down,
} Counter;

struct status_line {
  size_t moves;
  char *msg;
  Key key;
};

struct game_state {
  uint16_t order;
  uint16_t curs_x;
  uint16_t curs_y;
  uint16_t moves;
  Counter count_ctrl;
  int16_t utop;
  int16_t rtop;
  Key undo_stack[STK_SIZE];
  Key redo_stack[STK_SIZE];
  int** mat;
};

// main.c
void update_moves(struct game_state* gs);
bool update_matrix_view(const struct game_state* gs);
Counter mov_zero(struct game_state* gs, Key key);
bool is_solvable(int* list, int order);
void populate_mat(struct game_state* gs);
struct game_state game_state_init(Arena* arena, int order);
struct status_line status_line_init(char* msg);
void print_status_line(struct status_line data);
void show_menu(Key key, uint16_t *highlight);
int input_number(const char* query);
void display_usage();
uint16_t choose_mode(struct status_line status);

// utils.c
void swap(int *x, int *y);
void push_key(Key stk[], int16_t *top, Key key);
Key pop_key(Key stk[], int16_t *top);
void make_radomized_array(int* arr, size_t size);

// keymap.c
Key decode_key(int ch);

// save_and_load.c
void save_game_state(struct game_state* gs);
struct game_state load_game_state(Arena* arena);

#endif
